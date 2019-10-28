/*==================================================================================

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
   Date   : Feb 2019
*/


/* 
   Unit testing of subscription handler
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>


#include <string.h>
#include <stdio.h>
#include <iostream>
#include <csignal>
#include <chrono>
#include <subscription_handler.hpp>
#include <e2sm.hpp>
#include <queue>
#include <mutex>
#include <rmr/RIC_message_types.h>
#include <thread>


// global queue for testing
std::queue<std::string> message_bus;

// global lock for testing
std::mutex get_object ;

bool is_running = true;

bool mock_fail(int mtype,  size_t len,  void * payload, int mode){
  return false;
}

bool mock_silent(int mtype,  size_t len, void * payload, int mode){
  return true;
}


bool mock_tx(int mytpe,  size_t len,  void *payload, int mode){

  bool res;
  int i;
  subscription_helper he;
  subscription_response_helper he_resp;
  
  subscription_request sub_req;
  subscription_response sub_resp;

  subscription_delete sub_del_req;
  subscription_delete_response sub_del_resp;
  asn_dec_rval_t retval;

  E2N_E2AP_PDU_t * e2ap_pdu_recv;
  unsigned char buffer[256];
  size_t buf_size = 256;
  bool msg_ok = false;
  
  e2ap_pdu_recv = 0;
  retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_pdu_recv), payload, len);
  if(retval.code != RC_OK){
    std::cerr <<"Error decoding E2N_E2AP Subscription response PDU. Reason = " << strerror(errno) << std::endl;
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
    return false;
  }
  
  int procedure_code = e2ap_pdu_recv->choice.initiatingMessage->procedureCode;
  
  if(procedure_code == E2N_ProcedureCode_id_ricSubscription){

    he.clear();
    sub_req.get_fields(e2ap_pdu_recv->choice.initiatingMessage, he);
    
 
    // set up response object
    he_resp.set_request(he.get_request_id(), he.get_req_seq());
    he_resp.set_function_id(he.get_function_id());
    i = 0;

    // we simply copy over actions to both admitted and not
    // admitted list for now ..
    // in future, may need to be more selective
    for(auto &e : *(he.get_list())){
	he_resp.add_action(e.get_id());
	he_resp.add_action(e.get_id(), 1, 2);
      i++;
    }
    
    if(mode == 0){
      res = sub_resp.encode_e2ap_subscription_response(buffer, &buf_size,  he_resp, true);
      if (!res){
	std::cerr << "Error encoding subscription response successful. Reason = " << sub_resp.get_error() << std::endl;
      }
      else{
	msg_ok = true;
      }
    }
    else{
      res = sub_resp.encode_e2ap_subscription_response(buffer, &buf_size, he_resp, false);
      if (!res){
	std::cerr << "Error encoding subscription response failure . Reason = " << sub_resp.get_error() << std::endl;
      }
      else{
	msg_ok = true;
      }

    };
  
  }

  else if (procedure_code == E2N_ProcedureCode_id_ricSubscriptionDelete){

    he.clear();
    sub_del_req.get_fields(e2ap_pdu_recv->choice.initiatingMessage, he);
    
    // set up response object
    he_resp.clear();
    he_resp.set_request(he.get_request_id(), he.get_req_seq());
    he_resp.set_function_id(he.get_function_id());
    if(mode == 0){
      res = sub_del_resp.encode_e2ap_subscription_delete_response(buffer, &buf_size,  he_resp, true);
      if (!res){
	std::cerr << "Error encoding subscription delete  response failure . Reason = " << sub_resp.get_error() << std::endl;
      }
      else{
	msg_ok = true;
      }
    }
    else{
      res = sub_del_resp.encode_e2ap_subscription_delete_response(buffer, &buf_size,  he_resp, false);
      if (!res){
	std::cerr << "Error encoding subscription delete  response failure . Reason = " << sub_resp.get_error() << std::endl;
      }
      else{
	msg_ok = true;
	std::cout <<"Sending delete failures ..." << std::endl;
      }
      
    }
  }
  else{
    std::cout <<"Illegal request" << std::endl;
  }

  
  // push to queue
  if(msg_ok){
    std::lock_guard<std::mutex> guard(get_object);
    std::string msg((char *)buffer, buf_size);
    //std::cout <<"Pushed to queue" << std::endl;
    message_bus.push(msg);
  }

  
  ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
  if(msg_ok)
    return true;
  
  else
    return false;
    
    
}



// Randomly generate number of subscription response and delete
// response packets and push to queue
void random_tx(int num_packets){
  subscription_response_helper he_resp;
  subscription_response sub_resp;
  subscription_delete sub_del_req;
  subscription_delete_response sub_del_resp;
  bool res;
  unsigned char buffer[256];
  size_t buf_size = 256;

  he_resp.add_action(10);
  
  // generate subscription responses
  for(int i = 0; i < num_packets; i++){
    
    // set up response object
    he_resp.set_request(i, 1);
    he_resp.set_function_id(0); 
    buf_size = 256;
    res = sub_resp.encode_e2ap_subscription_response(buffer, &buf_size,  he_resp, true);
    {
        std::lock_guard<std::mutex> guard(get_object);
	std::string msg((char *)buffer, buf_size);
	message_bus.push(msg);
    }
    
    buf_size = 256;
    res = sub_resp.encode_e2ap_subscription_response(buffer, &buf_size,  he_resp, false);
    {
        std::lock_guard<std::mutex> guard(get_object);
	std::string msg((char *)buffer, buf_size);
	message_bus.push(msg);
    }
    
    buf_size = 256;
    res = sub_del_resp.encode_e2ap_subscription_delete_response(buffer, &buf_size,  he_resp, true);
    {
      std::lock_guard<std::mutex> guard(get_object);
      std::string msg((char *)buffer, buf_size);
      message_bus.push(msg);
    }

    buf_size = 256;
    res = sub_del_resp.encode_e2ap_subscription_delete_response(buffer, &buf_size,  he_resp, false);
    {
      std::lock_guard<std::mutex> guard(get_object);
      std::string msg((char *)buffer, buf_size);
      message_bus.push(msg);
    }
    
    
  }
}


void  mock_RAN (subscription_handler &_ref_sub_handler){
  // Behaviour :
  
  unsigned char incorrect_e2ap[128];
  size_t incorrect_e2ap_size = 128;
  for(int i = 0; i < 128; i++){
    incorrect_e2ap[i] = 'b';
  }

  FILE *pfile = fopen("test-data/e2ap_indication_test.per", "r");
  if(pfile == NULL){
    std::cout <<  "Error opening e2ap_indication_test.per" << std::endl;
    exit(-1);
  }
  unsigned char e2ap_msg[512];
  size_t e2ap_msg_size = fread((char *)e2ap_msg, sizeof(char), 512, pfile);
  fclose(pfile);

  unsigned char message_buf[512];
  std::string pdu;
  
  while(is_running){
    // send some random data, i.e incorrect E2AP
    _ref_sub_handler.Response(RIC_INDICATION, incorrect_e2ap, incorrect_e2ap_size);
    //std::cout <<"Sent random data to subscription handler" << std::endl;
    
    //  send an E2AP which is not subscription request
    _ref_sub_handler.Response(RIC_INDICATION, e2ap_msg, e2ap_msg_size);
    //std::cout <<"Sent incorrect e2ap  to subscription handler" << std::endl;
    
    // now look in the queue, pop it and send the data
    // finally send correct payload
    {
      std::lock_guard<std::mutex> guard(get_object);
      if(! message_bus.empty()){
	pdu  = message_bus.front();
	memcpy(message_buf,  pdu.c_str(), pdu.length());
	message_bus.pop();
      }
    }
    
    _ref_sub_handler.Response(RIC_SUB_RESP, message_buf, pdu.length());
    //std::cout <<"Sent  response to subscription handler" << std::endl;
      
    
    
    sleep(1);
  }
  
}


void send_request(subscription_handler &subscription_manager, std::vector<int> & status_vector, int index, bool (*tx)(int,  size_t, void *, int), int mode ){
  subscription_helper subscription_info;
  subscription_request sub_req;
  subscription_response_helper subscription_response_info;

  int function_id = 0;
  int action_id = 4;
  int action_type = 0;

  int request_id = 1;
  int req_seq = 1;
  int message_type = 1;
  int procedure_code = 27;
  std::string egnb_id = "Testgnb";
  std::string plmn_id = "Testplmn";
  
  unsigned char event_buf[128];
  size_t event_buf_len = 128;
  int res;    


  e2sm_event_trigger_helper trigger_data;
  e2sm_event_trigger event_trigger;
  
  trigger_data.egNB_id = egnb_id;
  trigger_data.plmn_id = plmn_id;
  trigger_data.egNB_id_type = 2;
  trigger_data.interface_direction = 1;
  trigger_data.procedure_code = procedure_code;
  trigger_data.message_type = message_type;

  res = event_trigger.encode_event_trigger(&event_buf[0], &event_buf_len, trigger_data);
  
  subscription_info.clear();
  subscription_info.set_request(request_id, req_seq);
  subscription_info.set_function_id(function_id);
  subscription_info.add_action(action_id, action_type);
  subscription_info.set_event_def(&event_buf[0], event_buf_len);

  auto transmitter = std::bind(tx, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, mode);
  res = subscription_manager.RequestSubscription(subscription_info, subscription_response_info , RIC_SUB_REQ, transmitter);
  
  if (res == SUBSCR_SUCCESS ){
    // store -ve of request id 
    status_vector[index] = -1 * subscription_info.get_request_id();
  }
  else{
    status_vector[index] = res;
  }

  std::cout <<"Subscription = " << subscription_info.get_request_id() << " Result = " << res << std::endl;
}

void delete_request(subscription_handler &subscription_manager, std::vector<int> & status_vector,  int index, int request_id, bool ( *tx)(int,  size_t, void *, int), int mode ){

  subscription_helper subscription_info;
  subscription_response_helper subscription_response_info;

 
  //verify subscription deleted
  subscription_info.set_request(request_id, 1);
  subscription_info.set_function_id(0);

  auto transmitter = std::bind(tx,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, mode);
  status_vector[index] = subscription_manager.RequestSubscriptionDelete(subscription_info, subscription_response_info, RIC_SUB_DEL_REQ, transmitter);
  
         
};


TEST_CASE("Test subscription work flow", "E2AP Subscription"){

  subscription_handler subscription_manager;
    
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "MOCK TEST SUBSCRIPTION WORK FLOW ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_DEBUG);
  mdclog_attr_destroy(attr);

  unsigned char node_buffer[32];
  std::string gNodeB = "TEST_GNOBDE";

  std::copy(gNodeB.begin(), gNodeB.end(), node_buffer);
  node_buffer[gNodeB.length()] = '\0';
  

  //====================================
  
  SECTION("Verify behaviour if no listener "){
    std::cout <<"+++++++++" << std::endl << "TEST WITH NO LISTENER " << std::endl;
    
    int num_sources = 10;
    std::vector<int> status_vector(num_sources, 0);
    subscription_manager.clear();
    
    std::vector<std::thread> source_list;
    
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), i, std::ref(mock_fail), 0));
    }
    
    for(int i = 0; i < num_sources; i++){
      source_list[i].join();
      REQUIRE(status_vector[i] == SUBSCR_ERR_TX);
    }
    
    REQUIRE(subscription_manager.num_complete() == 0);
    REQUIRE(subscription_manager.num_pending() == 0);
  }

  SECTION("Verify behaviour if listener does not respond"){
    std::cout <<"+++++++++" << std::endl << "TEST WHEN LISTENER DOES NOT RESPOND " << std::endl;
    
    int num_sources = 10;
    std::vector<int> status_vector(num_sources, 0);

    subscription_manager.clear();

    std::vector<std::thread> source_list;
    
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), i, std::ref(mock_silent), 0));
    }
    
    for(int i = 0; i < num_sources; i++){
      source_list[i].join();
    }

    for(int i = 0; i < num_sources; i++){
      REQUIRE(status_vector[i] == SUBSCR_ERR_TIMEOUT);
    }

    REQUIRE(subscription_manager.num_complete() == 0);
    REQUIRE(subscription_manager.num_pending() == 0);
    
  }
  
  SECTION("Verify timeout behaviour if listener does not response"){
    std::cout <<"+++++++++" << std::endl << "TEST TIMEOUT BEHAVIOUR  " << std::endl;
    
    int res;
    int num_sources = 1;
    int timeout_val = 2;
    int num_tries = 1;
    std::vector<int> status_vector(num_sources, 0);
     
    subscription_manager.clear();
    subscription_manager.set_timeout(timeout_val);
    subscription_manager.set_num_retries(num_tries);
    
    auto start = std::chrono::steady_clock::now();
    send_request(subscription_manager, status_vector, 0, mock_silent, 0);
    auto end = std::chrono::steady_clock::now();

    auto diff = end - start;
    
    REQUIRE(status_vector[0] == SUBSCR_ERR_TIMEOUT);
    REQUIRE(diff.count() >= num_tries * timeout_val);

    num_tries = 2;
    subscription_manager.set_num_retries(num_tries);
    status_vector.clear();

    start = std::chrono::steady_clock::now();
    send_request(subscription_manager, status_vector, 0, mock_silent, 0);
    end = std::chrono::steady_clock::now();

    diff = end - start;
    
    REQUIRE(status_vector[0] == SUBSCR_ERR_TIMEOUT);
    REQUIRE(diff.count() >= num_tries * timeout_val);

  }

  SECTION("Verify rejection of illegal pdus"){
    std::cout <<"+++++++++" << std::endl <<"TEST WITH ILLEGAL PDU PARAMS" << std::endl;
    subscription_helper subscription_info;
    subscription_response_helper subscription_response_info;

    subscription_manager.clear();
    int res;
    int function_id = 60000;
    int action_id = 4;
    int action_type = 0;
    
    int request_id = 1;
    int req_seq = 1;
    int message_type = 1;
    int procedure_code = 27;

    unsigned char event_buf[] = "Hello world";
    size_t event_buf_len = strlen((const char *)event_buf);
    
    subscription_info.clear();
    subscription_info.set_request(request_id, req_seq);
    subscription_info.set_function_id(function_id);
    subscription_info.add_action(action_id, action_type);
    subscription_info.set_event_def(&event_buf[0], event_buf_len);


    auto transmitter = std::bind(mock_silent, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, 0);
    res = subscription_manager.RequestSubscription(subscription_info, subscription_response_info , RIC_SUB_REQ, transmitter);
    REQUIRE(res == SUBSCR_ERR_ENCODE);

    

  }
  
  SECTION("Verify subscription request/response fail"){
    std::cout <<"+++++++++" << std::endl << "TEST WITH SUBSCRIPTION FAILURE " << std::endl;
    
    subscription_manager.clear();
    
    int num_sources = 20;
    int num_sinks = 5;
    
    std::vector<std::thread> source_list;
    std::vector<std::thread> sink_list;

    std::vector<int> status_vector(num_sources, 0);

    // start up the sinks
    is_running = true;
    for(int i = 0; i < num_sinks; i++){
      sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager)));
    }
    
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), i, std::ref(mock_tx), -1));
    }

    
    for(int i = 0; i < num_sources; i++){
      source_list[i].join();
      REQUIRE(status_vector[i] == SUBSCR_ERR_FAIL);
    }

    // stop the sinks
    is_running =false;
    for(int i = 0; i < num_sinks; i++){
      sink_list[i].join();
    }
      
    REQUIRE(subscription_manager.num_complete() == 0);
    REQUIRE(subscription_manager.num_pending() == 0);

  }

   SECTION("Verify subscription request/response success"){
     std::cout <<"+++++++++" << std::endl << "TEST WITH SUBSCRIPTION SUCCESS " << std::endl;
    

     subscription_manager.clear();
    
     int num_sources = 10;
     int num_sinks = 5;
     
     std::vector<std::thread> source_list;
     std::vector<std::thread> sink_list;
     
     std::vector<int> status_vector(num_sources, 0);

     // Test null cases in queries
     REQUIRE(subscription_manager.is_subscription_entry(10) == false);
     REQUIRE(subscription_manager.is_request_entry(1) == false);
     REQUIRE(subscription_manager.get_request_status(1) == -1);
     
     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager)));
     }
     
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), i, std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] < 0);
       REQUIRE(subscription_manager.is_subscription_entry(-1 * status_vector[i]) == true);
     }
     
     // stop the sinks
     is_running =false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
     
     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);

     const subscription_response_helper *  sub_info = subscription_manager.get_subscription(-1);
     REQUIRE(sub_info == NULL);

     sub_info = subscription_manager.get_subscription(-1 * status_vector[0]);
     REQUIRE(sub_info != NULL);
     REQUIRE(sub_info->get_request_id() == -1 * status_vector[0]);
     
   }

   SECTION("Delete requests for non-existent subscription requests"){
     std::cout <<"+++++++++" << std::endl << "TEST SUBSCRIPTION DELETE WITH NO CORRESPONDING REQUEST" << std::endl;
    
     subscription_manager.clear();
     REQUIRE(subscription_manager.get_request_status(0) == -1);
     REQUIRE(subscription_manager.is_subscription_entry(0) == false);
     REQUIRE(subscription_manager.is_request_entry(0) == false);
     
     int num_sources = 10;
     
     std::vector<std::thread> source_list;   
     std::vector<int> status_vector(num_sources, 0);
     srand(100);
     for(int i = 0; i < num_sources; i++){
       int req_id = rand()%1000;
       source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), i,req_id ,  std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] == SUBSCR_ERR_MISSING);
     }
     
     
   }

   

   SECTION("Delete requests that have succeeeded"){
     std::cout <<"+++++++++" << std::endl << "TEST WITH SUBSCRIPTION DELETES " << std::endl;
    
     subscription_manager.clear();
    
     int num_sources = 10;
     int num_sinks = 5;
     const subscription_response_helper * sub_resp_info;
     
     std::vector<std::thread> source_list;
     std::vector<std::thread> sink_list;
     
     std::vector<int> status_vector(num_sources, 0);
     
     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager)));
     }

     // First do subscriptions ...
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), i, std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] < 0 );
       REQUIRE(subscription_manager.is_subscription_entry(-1 * status_vector[i]) == true);
       sub_resp_info = subscription_manager.get_subscription(-1 * status_vector[i]);
       REQUIRE(sub_resp_info != NULL);
       REQUIRE(sub_resp_info->get_request_id() == -1 * status_vector[i]);
       
     }

     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);


     // Store ids ..
     std::vector<int> completed_requests = status_vector;
     
     // Delete successes
     source_list.clear();
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), i, -1 * completed_requests[i],  std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] == SUBSCR_SUCCESS);
     }

     REQUIRE(subscription_manager.num_pending() == 0);


     // stop the sinks
     is_running =false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
     REQUIRE(subscription_manager.num_complete() == 0);     
     REQUIRE(subscription_manager.num_pending() == 0);
  
   }

   SECTION("Deletes that fail"){

     std::cout <<"+++++++++" << std::endl << "TEST WITH SUBSCRIPTION DELETES THAT FAIL " << std::endl;
    
     subscription_manager.clear();
    
     int num_sources = 10;
     int num_sinks = 5;
     const subscription_response_helper * sub_resp_info;
     
     std::vector<std::thread> source_list;
     std::vector<std::thread> sink_list;   
     std::vector<int> status_vector(num_sources, 0);
     
     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager)));
     }

     // First do subscriptions ...
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), i, std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] < 0 );
       REQUIRE(subscription_manager.is_subscription_entry(-1 * status_vector[i]) == true);
       sub_resp_info = subscription_manager.get_subscription(-1 * status_vector[i]);
       REQUIRE(sub_resp_info != NULL);
       REQUIRE(sub_resp_info->get_request_id() == -1 * status_vector[i]);
       
     }

     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);


     // Store ids ..
     std::vector<int> completed_requests = status_vector;
    
     // Delete failures
     source_list.clear();
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), i, -1 * completed_requests[i],  std::ref(mock_tx), 1));
     }

     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] == SUBSCR_ERR_FAIL);
     }


     // stop the sinks
     is_running = false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
     
     // subscriptions are still there
     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);

     
   }

   SECTION("Deletes that timed out "){

     std::cout <<"+++++++++" << std::endl << "TEST WITH SUBSCRIPTION DELETES THAT TIMEOUT " << std::endl;
     
     subscription_manager.clear();
    
     int num_sources = 10;
     int num_sinks = 5;
     const subscription_response_helper * sub_resp_info;
     
     std::vector<std::thread> source_list;
     std::vector<std::thread> sink_list;
     
     std::vector<int> status_vector(num_sources, 0);
     
     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager)));
     }

     // First do subscriptions ...
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), i, std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] <  0 );
       REQUIRE(subscription_manager.is_subscription_entry(-1 * status_vector[i]) == true);
       sub_resp_info = subscription_manager.get_subscription(-1 * status_vector[i]);
       REQUIRE(sub_resp_info != NULL);
       REQUIRE(sub_resp_info->get_request_id() == -1 * status_vector[i]);
       
     }

     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);


     // Store ids ..
     std::vector<int> completed_requests = status_vector;
          
     // Delete with  time-outs 
     source_list.clear();
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), i, -1 * completed_requests[i],  std::ref(mock_silent), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] == SUBSCR_ERR_TIMEOUT);
     }

     // stop the sinks
     is_running = false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
    
     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);

   }
   
   SECTION("Spurious messages"){
     std::cout <<"+++++++++" << std::endl << "TEST WITH SPURIOUS RESPONSES" << std::endl;
   
     // In this section, we basically inject
     // spurious messages to subscription handler.
     // There are no outcomes. basically
     // handler should be able to ignore these messages

     int num_packets = 50;
     int num_sinks = 10;
     std::vector<std::thread> sink_list;
     
     subscription_manager.clear();
     std::cout <<"Message queue size prior to fill  = " << message_bus.size() << std::endl;
     random_tx(num_packets);
     std::cout <<"Message queue size post  fill  = " << message_bus.size() << std::endl;


     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager)));
     }

     // wait for queue to drain out
     while(! message_bus.empty()){
       sleep(2);
     }
     
     // stop the sinks
     is_running =false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
     REQUIRE(subscription_manager.num_complete() == 0);     
     REQUIRE(subscription_manager.num_pending() == 0);

     std::cout <<"Message queue size at end  = " << message_bus.size() << std::endl;

   }
   
};

   

