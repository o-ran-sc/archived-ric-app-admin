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


#include <subscription_handler.hpp>
#include <errno.h>

subscription_handler::subscription_handler(void){

  init();
  _time_out =  std::chrono::seconds(5);
  _num_retries = 2;

  // bool res;
  // unsigned char buffer[128];
  // size_t buf_len = 128;
  
  // E2N_E2AP_PDU_t e2ap_pdu;
  // subscription_request e2ap_sub_req;

  // int request_id = 2;
  // int req_seq = 1;
  // int function_id = 0;
  // int action_id = 0;
  // int action_type = 0;
  // int message_type = 1;
  
  // subscription_helper sgnb_add_subscr_req;
  
  // //sgnb_add_subscr_req.clear();
  // sgnb_add_subscr_req.set_request(request_id, req_seq);
  // sgnb_add_subscr_req.set_function_id(function_id);
  // sgnb_add_subscr_req.add_action(action_id, action_type);
  // std::string test = "This is a test";
  // sgnb_add_subscr_req.set_event_def(test.c_str(), test.length());
  // std::cout <<"Constructor ........" << std::endl;
  // // generate the request pdu
  // res = e2ap_sub_req.encode_e2ap_subscription(&buffer[0], &buf_len, &e2ap_pdu, sgnb_add_subscr_req);
  // if(! res){
  //   mdclog_write(MDCLOG_ERR, "%s, %d: Error encoding subscription pdu. Reason = ", __FILE__, __LINE__);
   
  // }
  // std::cout <<"Encoded subscription request pdu " << std::endl;
 
 
}

subscription_handler::subscription_handler(unsigned int timeout_seconds, unsigned int num_tries):_time_out(std::chrono::seconds(timeout_seconds)), _num_retries(num_tries){
  init();   
};

void subscription_handler::init(void){
  
  _data_lock = std::make_unique<std::mutex>();
  _cv = std::make_unique<std::condition_variable>();
  
}

void subscription_handler::clear(void){
  {
    std::lock_guard<std::mutex> lock(*(_data_lock).get());
    requests_table.clear();
    subscription_responses.clear();
  }
  
};

size_t subscription_handler::num_pending(void) const {
  return requests_table.size();
}

size_t subscription_handler::num_complete(void) const {
  return subscription_responses.size();
}


void subscription_handler::set_timeout(unsigned int timeout_seconds){
  _time_out = std::chrono::seconds(timeout_seconds);
}

void subscription_handler::set_num_retries(unsigned int num_tries){
  _num_retries = num_tries;
};


unsigned int subscription_handler::get_next_id(void){
  std::lock_guard<std::mutex> lock(*(_data_lock).get());
  unique_request_id ++;
  return unique_request_id;
}

bool subscription_handler::add_request_entry(int id, int status){

  // add entry in hash table if it does not exist
  auto search = requests_table.find(id);
  if(search != requests_table.end()){
    return false;
  }
  
  requests_table[id] = status;
  return true;

};

bool subscription_handler::set_request_status(int id, int status){
  
  // change status of a request only if it exists.

  auto search = requests_table.find(id);
  if(search != requests_table.end()){
    requests_table[id] = status;
    return true;
  }

  return false;
  
};


bool subscription_handler::delete_request_entry(int id){

  auto search = requests_table.find(id);
  if (search != requests_table.end()){
    requests_table.erase(search);
    return true;
  }

  return false;
};
  
bool subscription_handler::add_subscription_entry(int id, subscription_response_helper &he){

  auto search = subscription_responses.find(id);
  if (search == subscription_responses.end()){
    subscription_responses[id] = he;
    return true;
  }

  return false;
}


bool subscription_handler::delete_subscription_entry(int id){

  auto search = subscription_responses.find(id);
  if(search == subscription_responses.end()){
    return false;
  }
  else{
    subscription_responses.erase(search);
    return true;
  }
  
}

subscription_response_helper *  const subscription_handler::get_subscription(int id){
  auto search = subscription_responses.find(id);
  if(search == subscription_responses.end()){
    return NULL;
  }
  else{
    return &(subscription_responses[id]);
  }
};


// Handles responses from RMR
void subscription_handler::Response(int message_type, unsigned char *payload, int payload_length){

  bool res;
  int id;
  int type;
  int procedureCode;
  bool valid_response  =false;
  
  E2N_E2AP_PDU_t * e2ap_recv;
  asn_dec_rval_t retval;

  subscription_response sub_resp;
  subscription_delete_response sub_del_resp;

  subscription_response_helper he_response;


  e2ap_recv = 0;
  retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), payload, payload_length);

  if(retval.code != RC_OK){
    mdclog_write(MDCLOG_ERR, "%s, %d: Error decoding E2AP PDU of RMR type %d. Bytes decoded = %lu out of %d\n", __FILE__, __LINE__, message_type, retval.consumed, payload_length);
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
    return ;
  }
  
  type = e2ap_recv->present;
  mdclog_write(MDCLOG_INFO, "Received RMR message of type = %d", type);
  
  if(type == E2N_E2AP_PDU_PR_successfulOutcome){
    
    procedureCode =  e2ap_recv->choice.successfulOutcome->procedureCode;
    mdclog_write(MDCLOG_INFO, "Received E2N_E2AP PDU  successful outcome message with procedureCode = %d", procedureCode);  

    if( procedureCode == E2N_ProcedureCode_id_ricSubscription){  
      // subscription response
      // decode the message
      sub_resp.get_fields(e2ap_recv->choice.successfulOutcome, he_response);

      {
	std::lock_guard<std::mutex> lock(*(_data_lock.get()));
	// get the id
	id = he_response.get_request_id();
	// get status of id 
	int req_status = get_request_status(id);
	if (req_status == request_pending ){
	  res = add_subscription_entry(id, he_response);
	  if(res)
	    set_request_status(id, request_success);
	  
	  else{
	    set_request_status(id, request_duplicate);
	    mdclog_write(MDCLOG_ERR, "Error:: %s, %d: Request %d seems to be a duplicate\n", __FILE__, __LINE__, id);
	  }
	  
	  valid_response = true;
	}
	else if (req_status > 0){
	  // we don't change status of response since it was not in pending
	  // we simply fail
	  mdclog_write(MDCLOG_ERR, "Error:: %s, %d: Request %d is not in request_pending state, is in State = %d\n", __FILE__, __LINE__, id, req_status);
	  
	}
	else{
	  mdclog_write(MDCLOG_ERR,  "%s, %d: Could not find id %d in request queue for subscription", __FILE__, __LINE__,  id);
	}	  
	
      }
      
    }
    
    else if( procedureCode == E2N_ProcedureCode_id_ricSubscriptionDelete){
      
      res = sub_del_resp.get_fields(e2ap_recv->choice.successfulOutcome, he_response);
      {
	std::lock_guard<std::mutex> lock(*(_data_lock.get()));
	// get the id
	id = he_response.get_request_id();
	int req_status = get_request_status(id);
	if (req_status == delete_request_pending ){
	  // Remove the subscription from the table 
	  res = delete_subscription_entry(id);
	  if(res){
	    set_request_status(id, delete_request_success);
	    valid_response = true;
	  }
	  else{
	    set_request_status(id, delete_request_failed);
	    std::string error_string = "Error deleting subscription entry for id = ";
	    mdclog_write(MDCLOG_ERR,  "%s, %d: %s, %d", __FILE__, __LINE__, error_string.c_str(), id);
	    valid_response = true;
	  } 
	}      
	else if (req_status > 0){
	  // we don't change status since it was not in pending
	  // we simply fail
	  mdclog_write(MDCLOG_ERR, "Error:: %s, %d: Request %d for deletion  is not in delete_pending state, is in State = %d\n", __FILE__, __LINE__, id, req_status);
	}
	else{
	  mdclog_write(MDCLOG_ERR,  "%s, %d: Could not find id  %d in request queue for deletion ", __FILE__, __LINE__,  id);
	}

      }
    }

    else{
      mdclog_write(MDCLOG_ERR,  "%s, %d: Subscription Handler Response received E2AP PDU success  response with an non-subscription response related type  %d", __FILE__, __LINE__, procedureCode);
    }
    
  }
  
  else if(type == E2N_E2AP_PDU_PR_unsuccessfulOutcome){
    
    procedureCode = e2ap_recv->choice.unsuccessfulOutcome->procedureCode;
    mdclog_write(MDCLOG_INFO, "Received E2AP PDU  unsuccessful outcome message with procedureCode = %d", procedureCode);  
    
    if(procedureCode == E2N_ProcedureCode_id_ricSubscription){
      
      sub_resp.get_fields(e2ap_recv->choice.unsuccessfulOutcome, he_response);
      {
	std::lock_guard<std::mutex> lock(*(_data_lock.get()));  
	id = he_response.get_request_id();
	int req_status = get_request_status(id);
	if(req_status == request_pending){
	  set_request_status(id, request_failed);
	  valid_response = true;
	  mdclog_write(MDCLOG_ERR, "Subscription request %d failed", id);
	}
	else if (req_status > 0){
	  // we don't changet status since it was not in pending
	  // we simply fail
	  mdclog_write(MDCLOG_ERR, "Error:: %s, %d: Request %d is not in request_pending state, is in State = %d\n", __FILE__, __LINE__, id, req_status);
	}
	else{
	  mdclog_write(MDCLOG_ERR,  "%s, %d: Could not find id   %d in request queue for subscription ", __FILE__, __LINE__, id);
	}
      }
    }
    
    else if(procedureCode == E2N_ProcedureCode_id_ricSubscriptionDelete){
      
      res = sub_del_resp.get_fields(e2ap_recv->choice.unsuccessfulOutcome, he_response);	
      {
	std::lock_guard<std::mutex> lock(*(_data_lock.get()));
	// get the id
	id = he_response.get_request_id();
	int req_status = get_request_status(id);
	if(req_status == delete_request_pending){
	  set_request_status(id, delete_request_failed);
	  mdclog_write(MDCLOG_INFO, "Subscription delete request %d failed", id);
	  valid_response = true;
	}
	else if (req_status > 0){
	  mdclog_write(MDCLOG_ERR, "Error:: %s, %d: Request %d for deletion  is not in delete_pending state, is in State = %d\n", __FILE__, __LINE__, id, req_status);
	}
	else{
	  mdclog_write(MDCLOG_ERR,  "%s, %d: Could not find id  %d  in request queue for deletion ", __FILE__, __LINE__, id);
	}
	
      }
    }
    else{
      mdclog_write(MDCLOG_ERR,  "%s, %d: Susbcription Handler Response received E2AP PDU failure response with a non-subscription response related type  %d", __FILE__, __LINE__,  procedureCode);

    }
  }
  else{
    mdclog_write(MDCLOG_ERR,  "%s, %d: Susbcription Handler Response received E2AP PDU with non response type  %d", __FILE__, __LINE__, type);
  }
  
  
  ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
  
  // wake up all waiting users ...
  if(valid_response){
    _cv.get()->notify_all();
  }
  
}


int const subscription_handler::get_request_status(int id){
  auto search = requests_table.find(id);
  if (search == requests_table.end()){
    return -1;
  }
  
  return search->second;
}
				   
 bool subscription_handler::is_subscription_entry(int id){
  auto search = subscription_responses.find(id);
  if (search != subscription_responses.end())
    return true;
  else
    return false;
}

bool subscription_handler::is_request_entry(int id){
  auto search = requests_table.find(id);
  if (search != requests_table.end())
    return true;
  else
    return false;
}
