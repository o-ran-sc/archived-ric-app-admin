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

bool report_mode_only = true;
static int RunProg = 1;  // keep loop running

// list of plugins 
typedef  std::vector<std::unique_ptr<Policy> > plugin_list;
plugin_list Plugins;
std::map<int, Policy *> plugin_rmr_map;


bool add_subscription(SubscriptionHandler & sub_handler, XaPP * xapp_ref, subscription_helper & he, subscription_response_helper he_resp, std::string & gNodeB){
  unsigned char node_buffer[32];
  std::copy(gNodeB.begin(), gNodeB.end(), node_buffer);
  node_buffer[gNodeB.length()] = '\0';
  bool res = sub_handler.RequestSubscription(he, he_resp,  RIC_SUB_REQ, std::bind(static_cast<bool (XaPP::*)(int, int, void *, unsigned char const*)>( &XaPP::Send), xapp_ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, node_buffer));
  return res;
};


bool delete_subscription(SubscriptionHandler & sub_handler, XaPP * xapp_ref, subscription_helper & he, subscription_response_helper  he_resp, std::string & gNodeB){
  unsigned char node_buffer[32];
  std::copy(gNodeB.begin(), gNodeB.end(), node_buffer);
  node_buffer[gNodeB.length()] = '\0';

  bool res = sub_handler.RequestSubscriptionDelete(he, he_resp, RIC_SUB_DEL_REQ, std::bind(static_cast<bool (XaPP::*)(int, int, void *, unsigned char const*)>( &XaPP::Send), xapp_ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, node_buffer));
  return res;
};


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
  }
};


void metrics_collector(std::string ves_url, plugin_list * plugins,  unsigned int interval){
  
  // Instantiate the ves collector 
  curl_interface curl_obj(ves_url);
  std::string metrics_response;
  int res;
  while(RunProg){
    for(unsigned int i = 0; i < plugins->size(); i++){
      res = (*plugins)[i].get()->getMetrics(metrics_response);
      if (res != 0){
	mdclog_write(MDCLOG_WARN, "VES :: Warning : %s, %d: could not get metrics from plugin %s. Reason = %s", __FILE__, __LINE__, (*plugins)[i].get()->getName().c_str(), metrics_response.c_str());
      }
      else{
	res = curl_obj.post_metrics(metrics_response);
	if (!res){
	  mdclog_write(MDCLOG_WARN, "VES :: Warning : %s, %d , could not post metrics to %s. Reason = %s\n", __FILE__, __LINE__, ves_url.c_str(), curl_obj.getError().c_str());
	}
	else{
	  mdclog_write(MDCLOG_INFO, "VES :: Successfully posted metrics %s to VES collector\n", metrics_response.c_str());
	}
      }
    }
    sleep(interval);
  }
  std::cout <<"Stopped metrics collector/reporter .." << std::endl;
};
    

void EndProgram(int signum){
  std::cout <<"Signal received. Stopping program ....." << std::endl;
  RunProg = 0;
}

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

  if (my_config.operating_mode == "CONTROL"){
    report_mode_only = false;
  }
  else{
    report_mode_only = true;
  }
  
  // Finished passing command line/environment arguments 
  //=============================================================

   // instantiate xapp object
   if(my_config.num_threads >= 1){
    mdclog_write(MDCLOG_INFO, "XaPP listener threads specified = %d", my_config.num_threads);
    // Create XaPP that starts with specified number of threads 
    my_xapp = std::make_unique<XaPP>(my_config.name, my_config.port, 1024, my_config.num_threads);
  }
  else{
    mdclog_write(MDCLOG_INFO,"XaPP listener threads specified = auto");
    //Let XaPP pick threads based on hardware 
    my_xapp = std::make_unique<XaPP>(my_config.name, my_config.port, 1024);
  }
  

  mdclog_write(MDCLOG_INFO, "XaPP name specified = %s", my_config.name);
  mdclog_write(MDCLOG_INFO, "XaPP port specified = %s", my_config.port);

  
   // Instantiate admission logic handler
   Plugins.emplace_back(std::make_unique<admission>(my_config.a1_schema_file, my_config.sample_file, my_config.ves_schema_file,  1));
  
   // Add reference to plugin list . We add twice (once for set policy  and once for get policy  ids)
   // Plugin list is used by policy handler and metrics collector
   plugin_rmr_map.insert(std::pair<int, Policy *>(DC_ADM_INT_CONTROL, Plugins[0].get()));
   plugin_rmr_map.insert(std::pair<int, Policy *>(DC_ADM_GET_POLICY,  Plugins[0].get()));
   
   // instantiate curl object for ves 
   std::thread metrics_thread(metrics_collector, my_config.ves_collector_url, &Plugins, my_config.measurement_interval);


   // Instantiate subscription handler
   SubscriptionHandler sub_handler;

   // Instantiate message handlers for RMR
   // (one for each thread) and registrer
   // subscription and admission handlers
   
   std::vector<std::unique_ptr<message_processor> > message_procs;
   for(int i = 0; i < my_config.num_threads; i++){
     std::unique_ptr<message_processor> mp_handler = std::make_unique<message_processor> ();
     mp_handler.get()->register_subscription_handler(& sub_handler);
     mp_handler.get()->register_protector(dynamic_cast<admission *>(Plugins[0].get())->get_protector_instance(0));
     mp_handler.get()->register_policy_handler (& policy_handler);
     message_procs.push_back(std::move(mp_handler));
   }
  
  
   // Start the listening loops
   std::vector<int> thread_ids(my_config.num_threads);
   unsigned int i = 0;
   for(auto  &e: message_procs){
     thread_ids[i] = (*my_xapp).StartThread(*(e.get()), msg_error);
     i++;
   };

   mdclog_write(MDCLOG_INFO, "xAPP is UP and Listening on RMR. ...\n");
   mdclog_write(MDCLOG_INFO, "Number of message processors = %lu", message_procs.size());

   //======================================================
   // sgnb Subscription spec

   int request_id = 2; // will be over-written by subscription handler
   int req_seq = 1;
   int function_id = 0;
   int action_id = 4;
   int action_type = report_mode_only ? 0:1;
   
   int message_type = 1;
   int procedure_code = 27;
   std::string egnb_id = "Testgnb";
   std::string plmn_id = "Testplmn";

   unsigned char event_buf[128];
   size_t event_buf_len = 128;
   bool res;


   e2sm_event_trigger_helper trigger_data;
   e2sm_event_trigger event_trigger;
  
   trigger_data.egNB_id = egnb_id;
   trigger_data.plmn_id = plmn_id;
   trigger_data.egNB_id_type = 2;
   trigger_data.interface_direction = 1;
   trigger_data.procedure_code = procedure_code;
   trigger_data.message_type = message_type;
   res = event_trigger.encode_event_trigger(&event_buf[0], &event_buf_len, trigger_data);
   if (!res){
     mdclog_write(MDCLOG_ERR, "Error : %s, %d: Could not encode subscription Request. Reason = %s\n", __FILE__, __LINE__, event_trigger.get_error().c_str());
     exit(0);
   }
  

   subscription_helper sgnb_add_subscr_req;
   subscription_response_helper subscr_response;
  
   sgnb_add_subscr_req.clear();
   sgnb_add_subscr_req.set_request(request_id, req_seq);
   sgnb_add_subscr_req.set_function_id(function_id);
   sgnb_add_subscr_req.add_action(action_id, action_type);
  
  
   sgnb_add_subscr_req.set_event_def(&event_buf[0], event_buf_len);
   mdclog_write(MDCLOG_INFO, "Encoded event trigger definition into PDU of size %lu bytes\n", event_buf_len);

   //======================================================
   // Purely for testing purposes ... write subscription ASN binary to file 
   // FILE *pfile;
   // pfile = fopen("event_trigger.pr", "wb");
   // fwrite(event_buf, 1, event_buf_len,  pfile);
   // fclose(pfile);
   //======================================================
  
   
   // keep sending subscription request till successfull for all gnodebs ?
   auto it = my_config.gNodeB_list.begin();
   while(my_config.gNodeB_list.size() > 0 && RunProg){
     int attempt = 0;
     res = false;
      
     while(!res){
       mdclog_write(MDCLOG_INFO, "Sending subscription request for %s ... Attempt number = %d\n", (*it).c_str(), attempt);
       res = add_subscription(sub_handler, my_xapp.get(),  sgnb_add_subscr_req, subscr_response, *it);
       if (!res){
	 sleep(5);
       };
       attempt ++;
       if (attempt > MAX_SUBSCRIPTION_ATTEMPTS){
	 break;
       }
     }
     
     if(res){
       mdclog_write(MDCLOG_INFO, "Successfully subscribed for gNodeB %s", (*it).c_str());
       // remove node from list,
       // move to next gnobde
       it = my_config.gNodeB_list.erase(it);
     }

     if (it == my_config.gNodeB_list.end()){
       it = my_config.gNodeB_list.begin();
     }
     
   }
   
   
   std::cout <<"SUBSCRIPTION REQUEST :: Successfully subscribed to events for all gNodeBs " << std::endl;

   //Register signal handler to stop 
   signal(SIGINT, EndProgram);
   signal(SIGTERM, EndProgram);
   

   // Purely for testing purposes ....
   // If in test mode, we wait an interval and then send delete subscription request for each gNodeB
   if(my_config.test_mode){
     std::cout <<"====================== " << std::endl;
     std::cout <<"WE ARE IN TEST MODE. " << std::endl;
     std::cout <<"====================== " << std::endl;
     std::cout <<"WILL SEND SUBSCRIPTION DELETE REQUEST AFTER " << my_config.measurement_interval << " SECONDS " << std::endl;
     sleep(my_config.measurement_interval); 
     res = false;
     // keep sending subscription delete request till successfull ? 
     int attempt = 0;
     while(!res){
       mdclog_write(MDCLOG_INFO, "Sending subscription delete request for id = %d ... Attempt number = %d\n", sgnb_add_subscr_req.get_request_id(), attempt);
       res = delete_subscription(sub_handler, my_xapp.get(),  sgnb_add_subscr_req, subscr_response, my_config.gNodeB_list[0]);
       if (!res){
	 sleep(5);
       };
       attempt ++;
     }

     std::cout <<"SUBSCRIPTION DELETE REQUEST :: Successfuly deleted subscription request " << request_id << std::endl;
     
   };
   
   //Wait for stop
   while(RunProg){
     sleep(10);
   }
  
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
