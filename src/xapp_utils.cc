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

/*  Author : Ashwin Sridharan
    Date    : Feb 2019
*/


#include "xapp_utils.hpp"

// Constructor that automatically determines number of threads
XaPP::XaPP(char *xapp_name, char *proto_port, int msg_size): _is_ready(0), _listen(false), _num_retries(2), _msg_size(msg_size), _num_attempts(0), _num_fails(0){

  _num_threads = std::thread::hardware_concurrency();
  strcpy(_xapp_name, xapp_name);
  strcpy(_proto_port, proto_port);
  init(_msg_size);
  get_routes();

};
  


// Constructor that takes number of threads as argument 
XaPP::XaPP(char *xapp_name, char *proto_port, int msg_size, int num_threads): _is_ready(0), _listen(false), _num_retries(2),  _msg_size(msg_size), _num_threads(num_threads),_num_attempts(0), _num_fails(0) {

  strcpy(_xapp_name, xapp_name);
  strcpy(_proto_port, proto_port);
  init(_msg_size);
  get_routes();

};
  
// Destructor closes rmr context if available
XaPP::~XaPP(void){

  // Call stop to clear thread table
  Stop();

  // free memory
  rmr_free_msg(_rmr_tx_message);
  
  if (_rmr_ctx){
    rmr_close(_rmr_ctx);
  }
  // delete mutex
  delete _transmit;
};

// Get the RMR context 
void XaPP::init(int msg_size){

  if (msg_size > RMR_BUFFER_SIZE or msg_size <= 0){
    std::stringstream ss;
    ss << "Error ::" << __FILE__ << "," << __LINE__ << " Invalid buffer size  " << msg_size << " for RMR initialization context. Must be between " << 1 << " and " << RMR_BUFFER_SIZE << std::endl;
    ss << " To change max buffer requested, update RMR_BUFFER_SIZE " << std::endl;
    mdclog_write(MDCLOG_ERR, ss.str().c_str(), "");
    throw ss.str();
  }
  
  // Initialze the rmr context
  if ( (_rmr_ctx = rmr_init(_proto_port, msg_size, RMRFL_NONE)) == NULL){
    // throw exception here ..
    std::stringstream ss;
    ss << "Error ::" << __FILE__ << "," << __LINE__ << " Error initiatilizing RMR context for " << _xapp_name << " on port " << _proto_port << " Reason = " << strerror(errno) << std::endl;
    mdclog_write(MDCLOG_ERR, ss.str().c_str(), "");
    throw ss.str();
  }
  
}


// Blocks till routing table is received
void XaPP::get_routes(void){
  
  // Wait for getting a routing table ...
  int i = 0;
  _is_ready = 0;
  while( i < MAX_WAIT_TIME){
    std::cout <<"Waiting for RMR to be ready " << std::endl;
    if ((_is_ready = rmr_ready(_rmr_ctx)) == 1){
      break;
    }
    sleep(1);
    i++;
  };

  if(!_is_ready){
    std::string identifier = __FILE__ +  std::string(", Line: ") + std::to_string(__LINE__) ; 
    std::string error_string = identifier + " Error getting routing table";
    throw std::runtime_error(error_string);
  }
  

  // Get a tx buffer in case we need to do a transmit from the main thread itself
  if ( (_rmr_tx_message = rmr_alloc_msg(_rmr_ctx, RMR_BUFFER_SIZE)) == NULL){
    // throw exception here ..
    std::string identifier = __FILE__ +  std::string(", Line: ") + std::to_string(__LINE__) ; 
    std::string error_string = identifier + " Error getting a send buffer";
    throw std::runtime_error(error_string);
  }

  std::cout <<"Route Table received. Send buffer allocated" << std::endl;
  _transmit = new std::mutex();

}

// Send method that takes TLV (type/length/value) input
bool XaPP::Send(int type,  int payload_len, void *payload){

  if (likely(_is_ready)){
    if (likely(payload_len <= RMR_BUFFER_SIZE)){
      _rmr_tx_message->mtype  = type;
      memcpy(_rmr_tx_message->payload, payload, payload_len);
      _rmr_tx_message->len = payload_len;
      return Send(_rmr_tx_message);
    }
    else{
       std::string identifier = __FILE__ +  std::string(", Line: ") + std::to_string(__LINE__) ; 
       std::string error_string = identifier + " message payload len " + std::to_string(payload_len) + " exceeds maximum buffer size " + std::to_string(RMR_BUFFER_SIZE);
       mdclog_write(MDCLOG_ERR, error_string.c_str(), "");
    }
  }
  else{
    std::string identifier = __FILE__ +  std::string(", Line: ") + std::to_string(__LINE__) ; 
    std::string error_string = identifier + " rmr not ready to send";
    mdclog_write(MDCLOG_ERR, error_string.c_str(), "");
  }
  
  return false;
}

// Send method that takes TLV (type/length/value) input + MEID
bool XaPP::Send(int type,  int payload_len, void *payload, unsigned char const * meid){
  if (!_is_ready){
    return false;
  }

  _rmr_tx_message->mtype  = type;  
  memcpy(_rmr_tx_message->payload, payload, payload_len);
  _rmr_tx_message->len = payload_len;
  rmr_str2meid(_rmr_tx_message, meid);
  return Send(_rmr_tx_message);
  
}


// Send method that takes a buffer
bool XaPP::Send(rmr_mbuf_t * rmr_tx_message){

  if(likely(_is_ready && rmr_tx_message->len <= RMR_BUFFER_SIZE  && rmr_tx_message->len > 0)){
    int i = 0;
    rmr_tx_message->sub_id = RMR_VOID_SUBID;
    while(i <= _num_retries){
      
      //rmr_tx_message->state = 0; // fix for nng
      rmr_tx_message = rmr_send_msg(_rmr_ctx, rmr_tx_message);
      _num_attempts ++;
      
      if (! rmr_tx_message){
        // CRITICAL EROR .. log it 
        std::string identifier = __FILE__ +  std::string(", Line: ") + std::to_string(__LINE__) ; 
        std::string error_string = identifier + " rmr_send returned NULL";
        mdclog_write(MDCLOG_ERR, error_string.c_str(), "");
	return false;
      }
      else if (rmr_tx_message->state == RMR_OK){
	return true;
      }
      else  if(rmr_tx_message->state == RMR_ERR_RETRY){
	i++;
	_num_fails++;
      }
      else {
	mdclog_write(MDCLOG_ERR, "Error : %s, %d. Unable to transmit RMR message. RMR state = %d, %s\n", __FILE__, __LINE__, rmr_tx_message->state, strerror(errno));
	return false;
      }
    
    }
  }
  else{
    mdclog_write(MDCLOG_ERR, "Error: %s, %d  Invalid state/message for RMR tx. Ready = %d, Message len = %d\n", __FILE__, __LINE__, _is_ready, rmr_tx_message->len);
    return false;
  }
  
  std::stringstream ss;
  ss << "Error ::" << __FILE__ << "," << __LINE__ << " Could not send message of type " << rmr_tx_message->mtype << " and size " << rmr_tx_message->len << ". Reason = " << strerror(errno);
  mdclog_write(MDCLOG_ERR, ss.str().c_str(), "");
  
  return false;
}




void XaPP::Stop(void){
  // Get the mutex lock 
  std::lock_guard<std::mutex> guard(*_transmit);
  _listen = false;

  //Wait for all threads to stop
  for(auto & t: thread_group){
    std::thread::id my_id = t.second.get_id();
    std::stringstream thread_id;
    thread_id << my_id;
    t.second.join();

  }

  // Clear thread table ...
  thread_group.clear();
  
}

// default error handler if non specified by user
// pass through for now
void XaPP::_error_handler(rmr_mbuf_t *message){
};




//----------------------------------------
// Some get/set methods
//---------------------------------------

std::string XaPP::getName(void){
  return std::string(_xapp_name);
}

int XaPP::getStatus(void){
  return _is_ready;
}

bool XaPP::_isRunning(void){
  return _listen;
}


void * XaPP::get_rmr_context(void){
  return _rmr_ctx;
}

void XaPP::set_num_retries(int num_retries){
  if (num_retries < 0 || num_retries > MAX_RETRIES){
    throw "[xapp_utils] : Illegal value of num_retries. Must be positive integer between 0 and MAX_RETRIES\n";
  }
  
  _num_retries = num_retries;
}

int XaPP::get_num_retries(void){
  return _num_retries;
}


unsigned long  XaPP::get_Send_attempts(void){
  return _num_attempts;
};


unsigned long XaPP::get_Send_fails(void){
  return _num_fails;
};




void init_logger(const char  *AppName, mdclog_severity_t log_level)
{
    mdclog_attr_t *attr;
    mdclog_attr_init(&attr);
    mdclog_attr_set_ident(attr, AppName);
    mdclog_init(attr);
    mdclog_level_set(log_level);
    mdclog_attr_destroy(attr);
}
