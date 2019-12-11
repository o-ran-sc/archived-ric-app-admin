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
   Date    : Dec 2019
*/


/* 
   holds all functionality related to message exchange upon startup
   - subscription requests
   - policy requests

   NOTE : This module only sends out requests. Responses are assumed to be
   handled on RMR listening threads that are expected to already running in 
   main

*/

#include "adm-ctrl-xapp.hpp"


// function to call to add subscriptions
// Note 1 : it is synchronous. will block till it succeeds or fails
// Note 2:  we bind and pass the xapp tx function to separate out RMR from subscription process

int add_subscription(subscription_handler *sub_handler_ref, XaPP * xapp_ref, subscription_helper & he, subscription_response_helper he_resp, std::string & gNodeB){
  unsigned char node_buffer[32];
  std::copy(gNodeB.begin(), gNodeB.end(), node_buffer);
  node_buffer[gNodeB.length()] = '\0';

  int res = sub_handler_ref->request_subscription(he, he_resp,  gNodeB, RIC_SUB_REQ, std::bind(static_cast<bool (XaPP::*)(int, size_t, void *, unsigned char const*,  link_types, tx_types)>( &XaPP::Send), xapp_ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, node_buffer, link_types::HIGH_RELIABILITY, tx_types::ROUTE));
  return res;
};


// function to call to delete subscription
// Note 1 : it is synchronous. will block till it succeeds or fails
// Note 2:  we bind and pass the xapp tx function to separate out RMR from subscription process

int delete_subscription(subscription_handler *sub_handler_ref, XaPP * xapp_ref, subscription_helper & he, subscription_response_helper  he_resp, std::string & gNodeB){
  unsigned char node_buffer[32];
  std::copy(gNodeB.begin(), gNodeB.end(), node_buffer);
  node_buffer[gNodeB.length()] = '\0';
  
  int res = sub_handler_ref->request_subscription_delete(he, he_resp, gNodeB, RIC_SUB_DEL_REQ, std::bind(static_cast<bool (XaPP::*)(int, size_t, void *, unsigned char const*, link_types, tx_types)>(&XaPP::Send), xapp_ref, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, node_buffer, link_types::HIGH_RELIABILITY, tx_types::ROUTE));
  return res;
};



init::init (XaPP & xapp, subscription_handler & sub_handler, configuration & my_config){
  xapp_ref = &xapp;
  sub_handler_ref = &sub_handler;
  config_ref = &my_config;
}


// Main handle to subscribe to requests
// AC xAPP basically subscribes to just one  subscription (SgNB Addition Request), but can be extended to subscribe to
// multiple too.
void init::startup_subscribe_requests(void ){
  
  //======================================================
  // sgnb Subscription spec
  
   int request_id = 2; // will be over-written by subscription handler
   int req_seq = 1;
   int function_id = 0;
   int action_id = 4;
   int action_type = config_ref->report_mode_only ? 0:1;
   
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
   //======================================================
   
   // Encode the event trigger definition 
   res = event_trigger.encode_event_trigger(&event_buf[0], &event_buf_len, trigger_data);
   if (!res){
     mdclog_write(MDCLOG_ERR, "Error : %s, %d: Could not encode subscription Request. Reason = %s\n", __FILE__, __LINE__, event_trigger.get_error().c_str());
     exit(0);
   }
   mdclog_write(MDCLOG_INFO, "Encoded event trigger definition into PDU of size %lu bytes\n", event_buf_len);  

   // create the subscription
   subscription_helper sgnb_add_subscr_req;
   subscription_response_helper subscr_response;
  
   sgnb_add_subscr_req.clear();
   sgnb_add_subscr_req.set_request(request_id, req_seq);
   sgnb_add_subscr_req.set_function_id(function_id);
   sgnb_add_subscr_req.add_action(action_id, action_type);
   
   sgnb_add_subscr_req.set_event_def(&event_buf[0], event_buf_len);


   //======================================================
   // Purely for testing purposes ... write subscription ASN binary to file 
   // FILE *pfile;
   // pfile = fopen("event_trigger.pr", "wb");
   // fwrite(event_buf, 1, event_buf_len,  pfile);
   // fclose(pfile);
   //======================================================
  

   // for each gNodeB, try MAX_SUBSCRIPTION_ATTEMPTS
   // record gNodeBs for which we could not subscribe.
   // note that there could be multiple subscriptions for each gNodeB.
   // for AC xAPP we are doing just one ...
   std::vector<std::string> failed_gNodeBs;

   for(auto &it: config_ref->gNodeB_list){
     int attempt = 0;
     int subscr_result = -1;
     
     while(1){

       if(!run_program){
	 std::cout <<"Shutdown signal received during subscription process. Quitting ....." << std::endl;
	 break;
       }
       
       mdclog_write(MDCLOG_INFO, "Sending subscription request for %s ... Attempt number = %d\n", it.c_str(), attempt);
       subscr_result = add_subscription(sub_handler_ref, xapp_ref,  sgnb_add_subscr_req, subscr_response, it);
       if (subscr_result == SUBSCR_SUCCESS){
	 break;
       }
       sleep(5);
       attempt ++;
       if (attempt > MAX_SUBSCRIPTION_ATTEMPTS){
	 break;
       }
     }
     
     if(subscr_result == SUBSCR_SUCCESS){
       mdclog_write(MDCLOG_INFO, "Successfully subscribed for gNodeB %s", (it).c_str());
     }
     else{
       failed_gNodeBs.push_back(it);
     }
   }
   
   if (failed_gNodeBs.size() == 0){
     std::cout <<"SUBSCRIPTION REQUEST :: Successfully subscribed to events for all gNodeBs " << std::endl;
   }
   else{
     std::cerr <<"SUBSCRIPTION REQUEST :: Failed to subscribe for following gNodeBs" << std::endl;
     for(const auto &e: failed_gNodeBs){
       std::cerr <<"Failed to subscribe for gNodeB " << e << std::endl;
     }
   }
   
   
}


// Main handle to delete subscription requests
// Called upon shutdown
void init::shutdown_subscribe_deletes(){
  std::vector<subscription_identifier> sub_ids;

  subscription_helper sub_helper;
  subscription_response_helper subscr_response;
  
  // get list of subscriptions
  sub_handler_ref->get_subscription_keys(sub_ids);
  
  // send delete for each one ..
  // this is synchronous, hence will block ...
  for(auto & id: sub_ids){
    std::string gnodeb_id = std::get<0>(id);
    subscription_response_helper * sub_info = sub_handler_ref->get_subscription(id);
    int subscr_result = -1;
    if(sub_info != NULL){
      sub_helper.set_request(0, 0); // requirement of subscription manager ... ?
      sub_helper.set_function_id(sub_info->get_function_id());
      mdclog_write(MDCLOG_INFO, "Sending subscription delete for gNodeB %s\n", gnodeb_id.c_str());
      subscr_result = delete_subscription(sub_handler_ref, xapp_ref,  sub_helper, subscr_response, gnodeb_id);
      if(subscr_result == SUBSCR_SUCCESS){
	mdclog_write(MDCLOG_INFO, "Successfully deleted subscription for %s, %d\n", gnodeb_id.c_str(), sub_helper.get_function_id());
      }
      else{
	mdclog_write(MDCLOG_ERR, "Error : %s, %d. Could not delete subcription  for %s, %d. Reason = %d\n", __FILE__, __LINE__, gnodeb_id.c_str(), sub_helper.get_function_id(), subscr_result);
      }
    }
    else{
      mdclog_write(MDCLOG_ERR, "Error : %s, %d. Could not get subscription for %s, %d\n", __FILE__, __LINE__, std::get<0>(id).c_str(), std::get<1>(id));
    }
  }
  
}


//Request policies on start up
// This is async : once query is sent. responses from A1 are handled on RMR threads
void init::startup_get_policies(void){

  int policy_id = 21000;

  // we simply create json from scratch for now since it is quite simple
  std::string policy_query = "{\"policy_id\":" + std::to_string(policy_id) + "}";
  unsigned char * message = (unsigned char *)calloc(policy_query.length(), sizeof(unsigned char));
  memcpy(message, policy_query.c_str(),  policy_query.length());
  mdclog_write(MDCLOG_INFO, "Sending request for policy id %d\n", policy_id);
  xapp_ref->Send(A1_POLICY_QUERY, policy_query.length(), message, link_types::HIGH_RELIABILITY);
  free(message);
  
}


// start up subroutines go hear
void init::startup(void){
  startup_subscribe_requests();
  startup_get_policies();
  
}

// shutdown subroutines go here
void init::shutdown(void ){
  std::cout <<"Initiating shutdown subroutines ..." << std::endl;
  shutdown_subscribe_deletes();
}

