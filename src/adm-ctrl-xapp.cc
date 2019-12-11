/*
==================================================================================

        Copyright (c) 2018-2019 AT&T Intellectual Property.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
==================================================================================
*/
/* Author : Ashwin Sridharan
   Date    : Feb 2019
*/

#include "adm-ctrl-xapp.hpp"

int run_program = 1; 



// list of plugins 
plugin_list Plugins;
std::map<int, Policy *> plugin_rmr_map;

// Policy handler : All plugins (of abstract class Policy) are registered with the plugin list.
// The policy handler  is registered as a call back to the RMR message handler. When a policy related RMR message
// is received, this function is invoked. It finds the appropriate plugin from the plugin list, passes the policy message and then
// returns back the response (in the response string)

// NOTE : This version of policy handler was written with R1 policy protocol in mind.
// Specifically, it was assumed that each message type corresponds to either set/get for a specific xAPP.
// It still works with R2, but ideally should be modified since a single policy type could be applied to multiple plugins.

void  policy_handler(int message_type, const char * message, int message_len, std::string & response, bool set){
  auto it = plugin_rmr_map.find(message_type);
  bool res;
  if (it != plugin_rmr_map.end()){
    if (set){
      res = it->second->setPolicy(message, message_len, response);
      if (res){
	mdclog_write(MDCLOG_INFO, "A1 POLICY SET :: Successfully set A1 Policy\n");
      }
      else{
	mdclog_write(MDCLOG_ERR, "Error :: A1 POLICY SET   %s, %d . Unable to set policy. Reason = %s\n", __FILE__, __LINE__, response.c_str());
      }
	
    }
    else{
      res = it->second->getPolicy(message, message_len, response);
      if (res){
	mdclog_write(MDCLOG_INFO, "A1 POLICY GET : Successfully retreived  A1 Policy\n");
      }
      else{
	mdclog_write(MDCLOG_ERR, "Error :: A1 POLICY GET  %s, %d . Unable to get policy. Reason = %s\n", __FILE__, __LINE__, response.c_str());
      }
      
    }
  }
  else{
    response = "{\"status\":\"FAIL\", \"message\":\"Could not find plugin associated with RMR message type = " + std::to_string(message_type) + "\"}";
    mdclog_write(MDCLOG_ERR, "Error ! %s, %d : %s\n", __FILE__, __LINE__, response.c_str());
  }
};



// polling function that routinely queries all plugins for metrics and then posts them on
// VES url
void metrics_collector(std::string ves_url, plugin_list * plugins,  unsigned int interval){
  
  // Instantiate the ves collector 
  curl_interface curl_obj(ves_url);
  std::vector<std::string> response_vector;
  int res;
  while(run_program){
    for(unsigned int i = 0; i < plugins->size(); i++){
      response_vector.clear();
      res =  (*plugins)[i].get()->getMetrics(response_vector);
      if (res != 0){
	mdclog_write(MDCLOG_WARN, "VES :: Warning : %s, %d: could not get metrics from plugin %s. Reason = %s", __FILE__, __LINE__, (*plugins)[i].get()->getName().c_str(), (*plugins)[i].get()->get_error().c_str());
      }
      else{
	// send each response
	for(auto  &e: response_vector){
	  res = curl_obj.post_metrics(e);
	  if (!res){
	    mdclog_write(MDCLOG_WARN, "VES :: Warning : %s, %d , could not post metrics to %s. Reason = %s\n", __FILE__, __LINE__, ves_url.c_str(), curl_obj.getError().c_str());
	  }
	  else{
	    mdclog_write(MDCLOG_INFO, "VES :: Successfully posted metrics %s to VES collector\n", e.c_str());
	  }
	}
      }
    }
    sleep(interval);
  }
  std::cout <<"Stopped metrics collector/reporter .." << std::endl;
};
    

void EndProgram(int signum){
  std::cout <<"Signal received. Stopping program ....." << std::endl;
  run_program = 0;
  
}
// ideally should be expanded for rollback purposes etc.
void msg_error(rmr_mbuf_t *message){
  mdclog_write(MDCLOG_ERR, "Error: %s, %d  Could not send RMR message of length %d and type %d, Reason %s",  __FILE__, __LINE__, message->len,  message->mtype, strerror(errno) );
};


int main(int argc, char *argv[]){

  // initially set log level to INFO
  init_logger("XaPP", MDCLOG_INFO);
  
  configuration my_config;

  // set config variables from environment
  // used when deploying via start-up script
  get_environment_config(my_config);

  // over-ride with any command line variables if
  // provided
  get_command_line_config(argc, argv, my_config);

  std::unique_ptr<XaPP> my_xapp;


  // Reset log level based on configuration
  init_logger(my_config.name, static_cast<mdclog_severity_t>(my_config.log_level));
  
  if (my_config.gNodeB_list.size() == 0){
    mdclog_write(MDCLOG_WARN, "WARNING  : gNodeB not set for subscription. Subscription MAY FAIL");
  }

  std::string operating_mode;

  // How are we operating ? 
  if (my_config.operating_mode == "CONTROL"){
    // Full closed loop : process E2AP indication,
    // E2SM, X2AP and generate response
    my_config.report_mode_only = false;
    operating_mode = "CLOSED LOOP CONTROL";
  }
  else if (my_config.operating_mode == "E2AP_PROC_ONLY"){
    // Debugging : processing only E2AP indication 
    my_config.processing_level = ProcessingLevelTypes::E2AP_PROC_ONLY;
    operating_mode = "E2AP PROCESSING ONLY";
  }
  else if (my_config.operating_mode == "E2SM_PROC_ONLY"){
    // Debugging : processing only till E2SM indication header
    my_config.processing_level = ProcessingLevelTypes::E2SM_PROC_ONLY;
    operating_mode = "E2SM PROCESSING ONLY";
  }
  else{
    // Passive : processing till X2AP but do not generate response
    my_config.report_mode_only = true;
    operating_mode = "REPORTING ONLY";
  }

  mdclog_write(MDCLOG_DEBUG, "Operating mode of Admission Control xAPP is %s\n", operating_mode.c_str());
  // Finished passing command line/environment arguments 
  //=============================================================

  // instantiate xapp-rmr-framework  object
  mdclog_write(MDCLOG_INFO, "XaPP listener threads specified = %d", my_config.num_threads);
  mdclog_write(MDCLOG_INFO, "XaPP name specified = %s", my_config.name);
  mdclog_write(MDCLOG_INFO, "XaPP port specified = %s", my_config.port);

  my_xapp = std::make_unique<XaPP>(my_config.name, my_config.port, 1024);
  
  
  // Instantiate admission logic handler (with only one instance for now)
  int num_instances = 1;
  Plugins.emplace_back(std::make_unique<admission>(my_config.a1_schema_file, my_config.sample_file, my_config.ves_schema_file,  num_instances, my_config.xapp_id, my_config.report_mode_only));
  
  // Add reference to plugin list . 
  // Plugin list is used by policy handler and metrics collector
   plugin_rmr_map.insert(std::pair<int, Policy *>(A1_POLICY_REQ, Plugins[0].get()));
   
   // instantiate ves thread (it polls all plugins and sends out their metrics) 
   std::thread metrics_thread(metrics_collector, my_config.ves_collector_url, &Plugins, my_config.measurement_interval);

   // Instantiate subscription handler
   subscription_handler sub_handler;

   // Instantiate message handlers for RMR
   // (one for each thread) and registrer
   // subscription and admission handlers
   
   std::vector<std::unique_ptr<message_processor> > message_procs;
   for(int i = 0; i < my_config.num_threads; i++){
     std::unique_ptr<message_processor> mp_handler = std::make_unique<message_processor> (my_config.processing_level, my_config.report_mode_only);
     mp_handler.get()->register_subscription_handler(& sub_handler);
     mp_handler.get()->register_protector(dynamic_cast<admission *>(Plugins[0].get())->get_protector_instance(0));
     mp_handler.get()->register_policy_handler (& policy_handler);
     message_procs.push_back(std::move(mp_handler));
   }
  
  
   // Start the RMR listening loops
   std::vector<int> thread_ids(my_config.num_threads);
   unsigned int i = 0;
   for(auto  &e: message_procs){
     thread_ids[i] = (*my_xapp).StartThread(*(e.get()), msg_error);
     i++;
   };

   mdclog_write(MDCLOG_INFO, "xAPP is UP and Listening on RMR. ...\n");
   mdclog_write(MDCLOG_INFO, "Number of message processors = %lu", message_procs.size());

   //Register signal handler to stop
   signal(SIGINT, EndProgram);
   signal(SIGTERM, EndProgram);

   // Instantiate startup/shutown subroutine objects
   init boot_shutdown((*my_xapp), sub_handler, my_config);


   // Trigger start functions
   boot_shutdown.startup();
   
   
   //Wait for stop
   while(run_program){
     sleep(10);
   }


   // we are in shutdown mode
   // send out subscription deletes
   boot_shutdown.shutdown();

   i = 0;
   for(auto  &e: message_procs){
     mdclog_write(MDCLOG_INFO, "Thread %d : Number of packets handled = %lu", thread_ids[i], e.get()->get_messages());
     std::cout << "Thread " << thread_ids[i] << "  Number of packets handled = " <<  e.get()->get_messages() << std::endl;
     
     i ++ ;
   }
   
   std::cout <<"Stopping all running threads ..." << std::endl;
   (*my_xapp).Stop();
   std::cout <<"Stopped RMR processing threads ...." << std::endl;
   metrics_thread.join();
   std::cout <<"Stopped Metric collection thread ...." << std::endl;
   Plugins.clear();
   plugin_rmr_map.clear();
   std::cout <<"Cleared Plugins .." << std::endl;
  
   std::cout <<"Finished ... " << std::endl;
 
   return 0;
};
