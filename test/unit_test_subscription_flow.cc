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


// globally list gnodeb-id we use
std::string gNodeBID = "abc123";

// global queue for testing
// acts like a channel 
std::queue<std::pair<std::string, std::string>> message_bus;

// global lock for testing
std::mutex get_object ;

bool is_running = true;


// ==================================================
// various mock transmission functions that simulate underlying
// transmission layer behaviour

// this function immediately fails
// simulates a channel that is not available
bool mock_fail(int mtype,  size_t len,  void * payload, std::string gNodeB_id, int mode){
  return false;
}

// silently returns without actually doing any transmission
// simulates a lost transmission
bool mock_silent(int mtype,  size_t len, void * payload, std::string gNodeB_id, int mode){
  return true;
}

// simulates a working transmission channel 
bool mock_tx(int mytpe,  size_t len,  void *payload, std::string gNodeB_id, int mode){

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
    message_bus.push(std::make_pair(gNodeB_id, msg));
  }

  
  ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
  if(msg_ok)
    return true;  
  else
    return false;
    
    
}




// simulates response :takes what is in the queue, processes it and then invokes
// subscription_handler response 
void  mock_RAN (subscription_handler &_ref_sub_handler, int delay){

  // Behaviour :
  std::string gNodeB_id;
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
  std::pair<std::string, std::string> pdu;

  bool is_resp;
  while(is_running){
    
    // test illegal  packet response : send some random data, i.e incorrect E2AP
    _ref_sub_handler.Response(RIC_INDICATION, incorrect_e2ap, incorrect_e2ap_size, gNodeBID.c_str());
    //std::cout <<"Sent random data to subscription handler" << std::endl;
    
    //test incorrect packet response :  send an E2AP which is not subscription request
    _ref_sub_handler.Response(RIC_INDICATION, e2ap_msg, e2ap_msg_size, gNodeBID.c_str());
    //std::cout <<"Sent incorrect e2ap  to subscription handler" << std::endl;
    
    // now look in the queue, pop it and send the data
    // finally send correct payload if queue not empty
    is_resp = false;
    {
      std::lock_guard<std::mutex> guard(get_object);
      if(! message_bus.empty()){
	pdu  = message_bus.front();
	gNodeB_id = pdu.first;
	memcpy(message_buf,  pdu.second.c_str(), pdu.second.length());
	message_bus.pop();
	is_resp =true;
      }
    }

    if(is_resp){
      sleep(delay);
      _ref_sub_handler.Response(RIC_SUB_RESP, message_buf, pdu.second.length(), gNodeB_id.c_str());
      //std::cout <<"Sent  response to subscription handler" << std::endl;
    }
         
  }
  
}

// wrapper function that we use to test sending subscriptions with various channels
void send_request(subscription_handler &subscription_manager, std::vector<int> & status_vector, std::vector<std::string> & gNodeBs, int index, bool (*tx)(int,  size_t, void *,  std::string, int), int mode ){

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

  auto transmitter = std::bind(tx, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, gNodeBs[index],  mode);
  res = subscription_manager.request_subscription(subscription_info, subscription_response_info , gNodeBs[index], RIC_SUB_REQ, transmitter);
  
  if (res == SUBSCR_SUCCESS ){
    status_vector[index] = -1 ;
  }
  else{
    status_vector[index] = res;
  }

  std::cout <<"Subscription = " << subscription_info.get_request_id() << " Result = " << res << std::endl;
}


// wrapper function that we use to test sending delete requests with various channels
void delete_request(subscription_handler &subscription_manager, std::vector<int> & status_vector,  std::vector<std::string> & gNodeBs, int index, int request_id, bool ( *tx)(int,  size_t, void *, std::string, int), int mode ){

  subscription_helper subscription_info;
  subscription_response_helper subscription_response_info;

 
  //verify subscription deleted
  subscription_info.set_request(0, 0);
  subscription_info.set_function_id(0);

  auto transmitter = std::bind(tx,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, gNodeBs[index], mode);
  status_vector[index] = subscription_manager.request_subscription_delete(subscription_info, subscription_response_info, gNodeBs[index], RIC_SUB_DEL_REQ,  transmitter);
  
         
};


TEST_CASE("Test various channel responses", "E2AP Subscription"){

  subscription_handler subscription_manager;
    
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "MOCK TEST SUBSCRIPTION WORK FLOW ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);

  //====================================  
  SECTION("Verify behaviour if no listener "){
    std::cout <<"+++++++++" << std::endl << "TEST WITH NO LISTENER " << std::endl;
    
    int num_sources = 10;
    std::vector<int> status_vector(num_sources, 0);
    subscription_manager.clear();
    
    std::vector<std::thread> source_list;
    std::vector<std::string> gNodeBs;
    for(int i = 0; i < num_sources; i++){
      gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
    }
    
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs),  i, std::ref(mock_fail), 0));
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
        std::vector<std::string> gNodeBs;
    for(int i = 0; i < num_sources; i++){
      gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
    }

    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs),  i, std::ref(mock_silent), 0));
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
  
}


TEST_CASE("Test config", "E2AP Subscription"){

  subscription_handler subscription_manager;
    
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "MOCK TEST SUBSCRIPTION WORK FLOW ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);

  SECTION("Verify timeout behaviour if listener does not response"){
    std::cout <<"+++++++++" << std::endl << "TEST TIMEOUT BEHAVIOUR  " << std::endl;
    
    int res;
    int num_sources = 1;
    int timeout_val = 2;
    int num_tries = 1;
    std::vector<int> status_vector(num_sources, 0);

    std::vector<std::string> gNodeBs;
    for(int i = 0; i < num_sources; i++){
      gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
    }

    subscription_manager.clear();
    subscription_manager.set_timeout(timeout_val);
    subscription_manager.set_num_retries(num_tries);
    
    auto start = std::chrono::steady_clock::now();
    send_request(subscription_manager, status_vector, gNodeBs, 0, mock_silent, 0);
    auto end = std::chrono::steady_clock::now();

    auto diff = end - start;
    
    REQUIRE(status_vector[0] == SUBSCR_ERR_TIMEOUT);
    REQUIRE(diff.count() >= num_tries * timeout_val);

    num_tries = 2;
    subscription_manager.set_num_retries(num_tries);
    status_vector.clear();

    start = std::chrono::steady_clock::now();
    send_request(subscription_manager, status_vector, gNodeBs,  0, mock_silent, 0);
    end = std::chrono::steady_clock::now();

    diff = end - start;
    
    REQUIRE(status_vector[0] == SUBSCR_ERR_TIMEOUT);
    REQUIRE(diff.count() >= num_tries * timeout_val);

  }

}

TEST_CASE("Test sunny day scenarios", "E2AP Subscription"){

  subscription_handler subscription_manager;
    
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "MOCK TEST SUBSCRIPTION WORK FLOW ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);

  SECTION("Verify subscription request/response success"){
    std::cout <<"+++++++++" << std::endl << "TEST WITH SUBSCRIPTION SUCCESS " << std::endl;    
    
    subscription_manager.clear();
    
    int num_sources = 10;
    int num_sinks = 5;
    
    std::vector<std::thread> source_list;
    std::vector<std::thread> sink_list;
    
    std::vector<int> status_vector(num_sources, 0);
    
    // First Test null cases in queries,i.e non-existing request 
    subscription_identifier id = std::make_tuple (gNodeBID, 10);
     REQUIRE(subscription_manager.is_subscription_entry(id) == false);
     id = std::make_tuple(gNodeBID, 1);
     REQUIRE(subscription_manager.is_request_entry(id) == false);
     REQUIRE(subscription_manager.get_request_status(id) == -1);
     
     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), 1));
     }

     // generate the gnodeb list for which we are subscribing
     // default ran_function_id is zero 
     std::vector<std::string> gNodeBs;
     for(int i = 0; i < num_sources; i++){
       gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
     }
     
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs),  i, std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] < 0);
       id = std::make_tuple(gNodeBs[i], 0);
       REQUIRE(subscription_manager.is_subscription_entry(id) == true);
     }
     
     // stop the sinks
     is_running =false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
     
     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);
     
     // test getting subscription :
     // case 1: fake request
     id = std::make_tuple(gNodeBID, 0);
     const subscription_response_helper *  sub_info = subscription_manager.get_subscription(id);
     REQUIRE(sub_info == NULL);

     // case 2: valid request : get all the keys and use them
     std::vector<subscription_identifier> key_list;
     subscription_manager.get_subscription_keys(key_list);
     REQUIRE(key_list.size() == subscription_manager.num_complete());
     for(auto &e: key_list){
       sub_info = subscription_manager.get_subscription(e);
       REQUIRE(sub_info != NULL);
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
    std::vector<std::string> gNodeBs;
    
    // generate the gnodeb list for which we are subscribing
    // default ran_function_id is zero 
    for(int i = 0; i < num_sources; i++){
      gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
    }
    
    // start up the sinks
    is_running = true;
    for(int i = 0; i < num_sinks; i++){
      sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), 1));
    }
    
    // First do subscriptions ...
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs),  i, std::ref(mock_tx), 0));
    }
    
    
    for(int i = 0; i < num_sources; i++){
      source_list[i].join();
      REQUIRE(status_vector[i] < 0 );
      subscription_identifier id = std::make_tuple (gNodeBs[i], 0);       
      REQUIRE(subscription_manager.is_subscription_entry(id) == true);
      sub_resp_info = subscription_manager.get_subscription(id);
      REQUIRE(sub_resp_info != NULL);
      REQUIRE(sub_resp_info->get_request_id() == 0);
      
    }
    
    REQUIRE(subscription_manager.num_complete() == num_sources);
    REQUIRE(subscription_manager.num_pending() == 0);
    
    
    // Store ids ..
    std::vector<int> completed_requests = status_vector;
    
    // Delete successes
    source_list.clear();
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs), i, -1 * completed_requests[i],  std::ref(mock_tx), 0));
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

  
}

TEST_CASE("Test rainy day scenarios", "E2AP Subscription"){

  subscription_handler subscription_manager;
    
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "MOCK TEST SUBSCRIPTION WORK FLOW ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);

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

    unsigned char event_buf[] = "Hello world";
    size_t event_buf_len = strlen((const char *)event_buf);
    
    subscription_info.clear();
    subscription_info.set_request(request_id, req_seq);
    subscription_info.set_function_id(function_id);
    subscription_info.add_action(action_id, action_type);
    subscription_info.set_event_def(&event_buf[0], event_buf_len);

    std::vector<std::string> gNodeBs;
    auto transmitter = std::bind(mock_silent, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, gNodeBID, 0);
    res = subscription_manager.request_subscription(subscription_info, subscription_response_info , gNodeBID, RIC_SUB_REQ, transmitter);
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
    
    // generate the gnodeb list for which we are subscribing
    // default ran_function_id is zero 
    std::vector<std::string> gNodeBs;
    for(int i = 0; i < num_sources; i++){
      gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
    }

    // start up the sinks
    is_running = true;
    for(int i = 0; i < num_sinks; i++){
      sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), 1));
    }
    
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs), i, std::ref(mock_tx), -1));
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

  SECTION("Delete requests for non-existent subscription requests"){
    std::cout <<"+++++++++" << std::endl << "TEST SUBSCRIPTION DELETE WITH NO CORRESPONDING REQUEST" << std::endl;
    
    subscription_manager.clear();
    subscription_identifier id = std::make_tuple (gNodeBID, 0); 
    REQUIRE(subscription_manager.get_request_status(id) == -1);
    REQUIRE(subscription_manager.is_subscription_entry(id) == false);
    REQUIRE(subscription_manager.is_request_entry(id) == false);
    
     int num_sources = 10;
     
     std::vector<std::thread> source_list;   
     std::vector<int> status_vector(num_sources, 0);
     srand(100);
     std::vector<std::string> gNodeBs;

     // generate the gnodeb list for which we are subscribing
     // default ran_function_id is zero 
     for(int i = 0; i < num_sources; i++){
       gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
     }

     for(int i = 0; i < num_sources; i++){
       int req_id = rand()%1000;
       source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs),  i,req_id ,  std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] == SUBSCR_ERR_MISSING);
     }
     
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
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), 1));
     }

     // generate the gnodeb list for which we are subscribing
     // default ran_function_id is zero 
     std::vector<std::string> gNodeBs;
     for(int i = 0; i < num_sources; i++){
       gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
     }

     // First do subscriptions ...
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs), i, std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] < 0 );
       subscription_identifier id = std::make_tuple(gNodeBs[i], 0); 
       REQUIRE(subscription_manager.is_subscription_entry(id) == true);
       sub_resp_info = subscription_manager.get_subscription(id);
       REQUIRE(sub_resp_info != NULL);
       REQUIRE(sub_resp_info->get_request_id() == 0);
       
     }

     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);


     // Store status results 
     std::vector<int> completed_requests = status_vector;
    
     // Delete failures : mock_tx set to respond with failure
     source_list.clear();
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs), i, -1 * completed_requests[i],  std::ref(mock_tx), 1));
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
     
     // subscriptions are still there (did not get deleted)
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
     std::vector<std::string> gNodeBs;

     // generate the gnodeb list for which we are subscribing
     // default ran_function_id is zero 
     for(int i = 0; i < num_sources; i++){
       gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
     }
     
     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), 1));
     }

     // First do subscriptions ...
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs), i, std::ref(mock_tx), 0));
     }
     
     
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE(status_vector[i] <  0 );
       subscription_identifier id = std::make_tuple(gNodeBs[i], 0); 
       REQUIRE(subscription_manager.is_subscription_entry(id) == true);
       sub_resp_info = subscription_manager.get_subscription(id);
       REQUIRE(sub_resp_info != NULL);
       REQUIRE(sub_resp_info->get_request_id() == 0);
       
     }

     REQUIRE(subscription_manager.num_complete() == num_sources);
     REQUIRE(subscription_manager.num_pending() == 0);


     // Store ids ..
     std::vector<int> completed_requests = status_vector;
          
     // Delete with  time-outs 
     source_list.clear();
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(delete_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs), i, -1 * completed_requests[i],  std::ref(mock_silent), 0));
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
   

   
  SECTION("Verify timeout behaviour if transmitter sends after delay"){
    std::cout <<"+++++++++" << std::endl << "TEST DELAYED ARRIVAL OF SUBSCRIPTIONS " << std::endl;    
    
    subscription_manager.clear();
    int num_sources = 10;
    int num_sinks = 5;
    
    std::vector<std::thread> source_list;
    std::vector<std::thread> sink_list;
    
    std::vector<int> status_vector(num_sources, 0);

    // set subscription manager timeout on short fuse
    int time_out = 1;
    int num_tries = 1;
    subscription_manager.set_timeout(time_out);
    subscription_manager.set_num_retries(num_tries);

    // start up the sinks with delayed response
    is_running = true;
    int delay = 5;
    for(int i = 0; i < num_sinks; i++){
      sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), delay));
    }

    
    // generate the gnodeb list for which we are subscribing
    // default ran_function_id is zero 
    std::vector<std::string> gNodeBs;
    for(int i = 0; i < num_sources; i++){
      gNodeBs.push_back("test-gnodeb-" + std::to_string(i));
    }
     
    for(int i = 0; i < num_sources; i++){
      source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs),  i, std::ref(mock_tx), 0));
    }
     
     
    for(int i = 0; i < num_sources; i++){
      source_list[i].join();
      REQUIRE(status_vector[i]  == SUBSCR_ERR_TIMEOUT);
    }
    
    // stop the sinks
    is_running =false;
    for(int i = 0; i < num_sinks; i++){
      sink_list[i].join();
    }
    
    REQUIRE(subscription_manager.num_complete() == 0);
    REQUIRE(subscription_manager.num_pending() == 0);
     
  }
  
   SECTION("Duplicate requests"){
     std::cout <<"+++++++++" << std::endl << "TEST DUPLICATE SUBSCRIPTION REQUESTS " << std::endl;
     
     subscription_manager.clear();
     
     int num_sources = 20;
     int num_sinks = 5;
     
     std::vector<std::thread> source_list;
     std::vector<std::thread> sink_list;
     
     std::vector<int> status_vector(num_sources, 0);
     
     // generate IDENTICAL  gnodeb list for which we are subscribing
     // default ran_function_id is zero 
     std::vector<std::string> gNodeBs;
     for(int i = 0; i < num_sources; i++){
       gNodeBs.push_back("test-gnodeb-" + std::to_string(0));
     }

     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), 1));
     }
     
     // send out subscriptions 
     for(int i = 0; i < num_sources; i++){
       source_list.push_back(std::thread(send_request, std::ref(subscription_manager), std::ref(status_vector), std::ref(gNodeBs), i, std::ref(mock_tx), 0));
     }

     // exactly ONE subscription should succeed. all others should fail with SUBSCR_ERR_DUPLICATE
     for(int i = 0; i < num_sources; i++){
       source_list[i].join();
       REQUIRE( (status_vector[i] == -1  || status_vector[i] == SUBSCR_ERR_DUPLICATE));
       
     }
     
     // stop the sinks
     is_running =false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
     
     REQUIRE(subscription_manager.num_complete() == 1);     
   }


   SECTION("Duplicate responses"){
     // this scenario can happen if there was an initial successful
     // subscription with <gnodeb-id, ran-function-id> request
     // followed by another one. The response for the second one should
     // result in a duplicate subscription error


     std::cout <<"+++++++++" << std::endl << "TEST DUPLICATE SUBSCRIPTION RESPONSES" << std::endl;
     
     subscription_manager.clear();

     int num_sources = 1;
     int num_sinks = 1;
     std::vector<int> status_vector (num_sources, 0);
     std::vector<std::string> gNodeBs;
     gNodeBs.push_back("test-gnodeb");
     
     std::vector<std::thread> sink_list;
     // start up the sinks
     is_running = true;
     for(int i = 0; i < num_sinks; i++){
       sink_list.push_back(std::thread(mock_RAN, std::ref(subscription_manager), 1));
     }

     // send a subscription  : this should succeed
     send_request(subscription_manager, status_vector, gNodeBs, 0, mock_tx, 0);
     REQUIRE(status_vector[0] == -1);
     REQUIRE(subscription_manager.num_pending() == 0);
     REQUIRE(subscription_manager.num_complete() == 1);


     // now send same subscription again
     send_request(subscription_manager, status_vector, gNodeBs, 0, mock_tx, 0);
     REQUIRE(status_vector[0] == SUBSCR_ERR_DUPLICATE);
     REQUIRE(subscription_manager.num_pending() == 0);
     REQUIRE(subscription_manager.num_complete() == 1);
     
     // stop the sinks
     is_running =false;
     for(int i = 0; i < num_sinks; i++){
       sink_list[i].join();
     }
     
   }
     
}

   

