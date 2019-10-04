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


#pragma once

#ifndef SUBSCRIPTION_HANDLER
#define SUBSCRIPTION_HANDLER

#include <mdclog/mdclog.h>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <chrono>

#include <subscription_request.hpp>
#include <subscription_response.hpp>
#include <subscription_delete_request.hpp>
#include <subscription_delete_response.hpp>

using namespace std;

typedef enum {
    request_pending = 1,
    request_success,
    request_failed,
    delete_request_pending,
    delete_request_success,
    delete_request_failed,
    request_duplicate
}Subscription_Status_Types;


/* Class to process subscription related messages 
   each subscription request is assigned a unique internally
generated request id for tracking purposes. this is because
the e2 subscription request does not carry any gnodeb id information

*/

class SubscriptionHandler {

public:
  SubscriptionHandler(void);
  SubscriptionHandler(unsigned int, unsigned int);
  
  void init(void);
  template <typename Transmitter>
  bool RequestSubscription(subscription_helper &,  subscription_response_helper &, int , Transmitter &&);

  template<typename Transmitter>
  bool RequestSubscriptionDelete(subscription_helper  &, subscription_response_helper &, int , Transmitter &&);


  void  Response(int, unsigned char *, int);
  int const get_request_status(int);
  subscription_response_helper * const get_subscription(int);

  unsigned int get_next_id(void);
  void set_timeout(unsigned int);
  void set_timeout_flag(bool);
  void set_num_retries(unsigned int);
  
  bool is_subscription_entry(int); 
  bool is_request_entry(int);

  void clear(void);
  size_t  num_pending(void) const;
  size_t  num_complete(void) const ;


  
private:

  void ProcessSubscriptionResponse(unsigned char *, int len);
  void ProcessSubscriptionFailure(unsigned char *, int len);

  void ProcessDeleteResponse(unsigned char *, int len);
  void ProcessSubscriptionDeleteFailure(unsigned char *, int len);

  bool add_request_entry(int, int);
  bool set_request_status(int, int);
  bool delete_request_entry(int);
 
  bool get_subscription_entry(int, int);
  bool add_subscription_entry(int, subscription_response_helper &he);
  bool delete_subscription_entry(int);
  
  std::unordered_map<int, int> requests_table;
  std::unordered_map<int, subscription_response_helper> subscription_responses; // stores list of successful subscriptions
  
  std::unique_ptr<std::mutex> _data_lock;
  std::unique_ptr<std::condition_variable> _cv;

  std::chrono::seconds _time_out;
  unsigned int _num_retries = 2;
  bool _time_out_flag = true;
  unsigned int unique_request_id = 0;
  
};

template <typename Transmitter>
bool SubscriptionHandler::RequestSubscription(subscription_helper &he, subscription_response_helper &response, int TxCode, Transmitter && tx){
  
  bool res;
  unsigned char buffer[512];
  size_t buf_len = 512;

  // get a new unique request id ...
  unsigned int new_req_id = get_next_id();
  std::cout <<"Using id = " << new_req_id << std::endl;
  he.set_request(new_req_id, he.get_req_seq());
  
  E2AP_PDU_t *e2ap_pdu = 0;
  subscription_request e2ap_sub_req;
  
  // generate the request pdu
  res = e2ap_sub_req.encode_e2ap_subscription(&buffer[0], &buf_len, e2ap_pdu, he);
  if(! res){
    mdclog_write(MDCLOG_ERR, "%s, %d: Error encoding subscription pdu. Reason = ", __FILE__, __LINE__);
    return false;
  }
  
  // put entry in request table
  {
    std::lock_guard<std::mutex> lock(*(_data_lock.get()));
    res = add_request_entry(he.get_request_id(), request_pending);
    if(! res){
      mdclog_write(MDCLOG_ERR, "Error adding new subscription request %d to queue",  he.get_request_id());
      return false;
    }
  }

  // acquire lock ...
  std::unique_lock<std::mutex> _local_lock(*(_data_lock.get()));


  
  // Send the message
  res = tx(TxCode,  buf_len, buffer);
  if (!res){
    // clear state
    delete_request_entry(he.get_request_id());
    mdclog_write(MDCLOG_ERR, "Error transmitting subscription request %d", he.get_request_id());
    return false;
  };

  
  // record time stamp ..
  auto start = std::chrono::system_clock::now();
  
  while(1){


      
    // wait to be woken up
    _cv.get()->wait_for(_local_lock, _time_out);
    
    // we have woken and acquired data_lock 
    // check status and return appropriate object
    
    int status = get_request_status(he.get_request_id());
    
    if (status == request_success){
      mdclog_write(MDCLOG_INFO, "Successfully subscribed for request %d", he.get_request_id());
      break;
    }
    
    if (status == request_pending){
      // woken up spuriously or timed out 
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> f = end - start;
      
      if (_time_out_flag && f > _num_retries * _time_out){
	delete_request_entry(he.get_request_id());
	mdclog_write(MDCLOG_ERR, "Subscription request %d timed out waiting for response ", he.get_request_id());

	// Release data lock
	_local_lock.unlock();
	return false;
      }
      else{
	mdclog_write(MDCLOG_INFO, "Subscription request %d Waiting for response ....", he.get_request_id());     
	continue;
      }
    }

    // if we are hear, some spurious
    // status obtained or request failed . we return false
    delete_request_entry(he.get_request_id());
    
    // release data lock
    _local_lock.unlock();

    return false;
    
  };
	     
  // retreive the subscription response and clear queue
  response = subscription_responses[he.get_request_id()];
  delete_request_entry(he.get_request_id());

  // release data lock
  _local_lock.unlock();
  
  return true;
};


template <typename Transmitter>
bool SubscriptionHandler::RequestSubscriptionDelete(subscription_helper &he, subscription_response_helper &response, int TxCode, Transmitter && tx){
  
  bool res;

  // First check if we have this subscription
  if(! is_subscription_entry(he.get_request_id())){
    mdclog_write(MDCLOG_ERR, "subscription with id %d  does not exist. Cannot be deleted",  he.get_request_id());	
    return false;
  }  
  
  // Also check if such a request is queued
  if (is_request_entry(he.get_request_id())){
    mdclog_write(MDCLOG_ERR, "Subscription delete request  with id %d  already in queue",  he.get_request_id());
    return false;
  }

  subscription_delete e2ap_sub_req_del;
  
  // generate the delete request pdu
  unsigned char buffer[128];
  size_t buf_len = 128;
  
  res = e2ap_sub_req_del.encode_e2ap_subscription(&buffer[0], &buf_len, he);
  if(! res){
    mdclog_write(MDCLOG_ERR, "%s, %d: Error encoding subscription delete request pdu. Reason = %s", __FILE__, __LINE__, e2ap_sub_req_del.get_error().c_str());
    return false;
  }
  
  // put entry in request table
  res = add_request_entry(he.get_request_id(), delete_request_pending);
  if(! res){
    mdclog_write(MDCLOG_ERR, "Error adding new subscription request %d to queue",  he.get_request_id());
    return false;
  }
  
  std::unique_lock<std::mutex> _local_lock(*(_data_lock.get()));

  // Send the message
  res = tx(TxCode,  buf_len, buffer);

  if (!res){
    delete_request_entry(he.get_request_id());
    mdclog_write(MDCLOG_ERR, "Error transmitting delete subscription request %d", he.get_request_id());
    return false;
  };

  
  // record time stamp ..
  auto start = std::chrono::system_clock::now();

  while(1){
    
    // wait to be woken up
    _cv.get()->wait_for(_local_lock, _time_out);
    
    // check status and return appropriate object
    int status = get_request_status(he.get_request_id());
    if (status == delete_request_success){
      delete_request_entry(he.get_request_id());
      mdclog_write(MDCLOG_INFO, "Successfully deleted subscription id %d", he.get_request_id());
      _local_lock.unlock();
      break;
    }
    
    if (status == delete_request_pending){
      // woken up spuriously or timed out 
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> f = end - start;
      
      if (_time_out_flag && f > _num_retries * _time_out){
	delete_request_entry(he.get_request_id());
	mdclog_write(MDCLOG_ERR, "Subscription delete request %d timed out waiting for response ", he.get_request_id());
	_local_lock.unlock();
	return false;
      }
      else{
	mdclog_write(MDCLOG_INFO, "Subscription delete request %d Waiting for response ....", he.get_request_id()); 
      }
      
      continue;
    }

    // if we are hear, some spurious
    // status obtained. we return false
    
    delete_request_entry(he.get_request_id());
    _local_lock.unlock();
    return false;

  };

  return true;
};

#endif
