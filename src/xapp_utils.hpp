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

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <assert.h>
#include <thread>
#include <map>
#include <mutex>
#include <sys/epoll.h>
#include <functional>
#include <sstream>
#include <rmr/rmr.h>
#include <rmr/RIC_message_types.h>
#include <mdclog/mdclog.h>


#ifndef XAPP_UTIL
# define XAPP_UTIL


#define XAPP_NAME_LENGTH 128
#define PROTO_PORT_LENGTH 16
#define MAX_RETRIES 16
#define MAX_WAIT_TIME 10
#define EPOLL_TIMEOUT 500  //in milli-seconds
#define RMR_TIMEOUT 50// in mill-seconds
#define RMR_BUFFER_SIZE 16384
#define MAX_THREADS 8

#ifdef __GNUC__
#define likely(x)  __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif


// define RMR Send behaviour for different link-types
// controls how often we try and delay between tries, as well as method

enum  link_types {LOW_LATENCY, HIGH_RELIABILITY};
static const unsigned int link_delays[] = {1, 10}; // milli-seconds to wait before retries
static const unsigned int link_retries[] = {4, 15}; // number of times to retry
enum  tx_types {ROUTE, RTS}; // regular rmr or rts

void init_logger(const char  *AppName, mdclog_severity_t log_level);


class XaPP {
  
 public:

  XaPP(char *, char *, int);
  ~XaPP(void);
  XaPP(XaPP &&) = default;  // destructor freaks out with non-copyable thread otherwise ..

  std::string get_name(void);

  int  get_status(void);

  size_t  get_num_active_threads(void) const { return thread_group.size(); };
  
  // ideally can reduce tempate definitions to just two
  // but for now leaving it open ...
  
  // Template to allow a user defined processor to start
  // on a thread 
  template <typename messageProcessor > 
  unsigned int  StartThread(messageProcessor &&);

  // Template to allow a user defined processor AND
  // error handle to start // on a single thread each time it
  // is invoked
  template <typename messageProcessor , typename errorHandler> 
  unsigned int StartThread(messageProcessor &&, errorHandler &&);

  void Stop(void);

  // various flavours of send : first two finally call the last
  bool Send(int type,  size_t payload_len, void *payload, link_types mode = link_types::LOW_LATENCY, tx_types send_type = tx_types::ROUTE);
  bool Send(int type, size_t  payload_len, void *payload, unsigned char const  *meid, link_types mode = link_types::LOW_LATENCY, tx_types send_type = tx_types::ROUTE);
  bool Send(rmr_mbuf_t * rmr_tx_message, link_types mode = link_types::LOW_LATENCY, tx_types send_type = tx_types::ROUTE);
  
  void * get_rmr_context(void);
  
private:

  void init(int);
  void get_routes();
  void _error_handler(rmr_mbuf_t *); // pass through placeholder
  
  template<typename messageProcessor, typename errorHandler>
  void _workThread(messageProcessor &&, errorHandler &&, XaPP *);

  char _xapp_name[XAPP_NAME_LENGTH];
  char _proto_port[PROTO_PORT_LENGTH];

  int _is_ready;
  bool _listen;
  int _num_retries;
  int _retry_interval;
  int _msg_size;
  unsigned int _num_threads;

  void* _rmr_ctx;
  std::mutex *_transmit;
  std::map <unsigned int, std::thread> thread_group;
  rmr_mbuf_t * _rmr_tx_message;
 
  bool _isRunning(void);
  
};

// main workhorse thread which does the listen->process->respond loop 
template <typename messageProcessor, typename errorHandler>
void XaPP::_workThread(messageProcessor && msg_fn, errorHandler && error_handler, XaPP *parent){


  // Get the thread id 
  std::thread::id my_id = std::this_thread::get_id();
  std::stringstream thread_id;
  std::stringstream ss;
  
  thread_id << my_id;

  // Stats counters 
  unsigned long recvs = 0;
  unsigned long attempts = 0;
  unsigned long fails = 0;
 
  // Get the rmr context from parent (all threads and parent use same rmr context. rmr context is expected to be thread safe)
  void *rmr_context = parent->get_rmr_context(); 
  assert(rmr_context != NULL);
  
  // Get buffer specific to this thread
  rmr_mbuf_t *rmr_message = NULL;
  rmr_message = rmr_alloc_msg(rmr_context, RMR_BUFFER_SIZE);
  assert(rmr_message != NULL);

  // Create an epoll instance
  int rcv_fd, ep_fd;
  struct epoll_event eve, trigger;
  rcv_fd = rmr_get_rcvfd(rmr_context);
  assert(rcv_fd > 0);
  
  ep_fd = epoll_create1(0);
  assert(ep_fd > 0);
  
  trigger.events = EPOLLIN|EPOLLET|EPOLLONESHOT;
  trigger.data.fd = rcv_fd; 

  if (epoll_ctl (ep_fd, EPOLL_CTL_ADD, rcv_fd, &trigger) < 0){
    ss << __FILE__ << "," << __LINE__ << " Thread " << thread_id.str() << " Error registering epoll file descriptor" << " Reason = " << strerror(errno) << std::endl;
    mdclog_write(MDCLOG_ERR, ss.str().c_str(), "");
    throw std::runtime_error(ss.str());
  }

  
  int num_fds = 0;
  bool send_ok;
  
  mdclog_write(MDCLOG_INFO, "Starting thread %s",  thread_id.str().c_str());

  // the workhorse loop 
  while(parent->_isRunning()){
    num_fds = epoll_wait(ep_fd, &eve, 1, EPOLL_TIMEOUT);
    
    if(num_fds && eve.data.fd == rcv_fd){
      rmr_message = rmr_torcv_msg(rmr_context, rmr_message, RMR_TIMEOUT);
      //rmr_message = rmr_rcv_msg(rmr_context, rmr_message);
      
      // Re-arm the trigger
      if (epoll_ctl (ep_fd, EPOLL_CTL_MOD, rcv_fd, &trigger) < 0){
	ss << __FILE__ << "," << __LINE__ << " Thread " << thread_id.str() << " Error re-arming epoll" << " Reason = " << strerror(errno) << std::endl;
	mdclog_write(MDCLOG_ERR, ss.str().c_str(), "");
	throw std::runtime_error(ss.str());
      }
    
    }
    else{
      continue;
    };
  
    if (rmr_message && rmr_message->state == RMR_OK){

      recvs++;
      bool res = msg_fn(rmr_message);

      // is there anything to send ?
      if (res && rmr_message != NULL && likely (rmr_message->len > 0 && rmr_message->len <= RMR_BUFFER_SIZE)){

	rmr_message->sub_id = RMR_VOID_SUBID; // do we change this ? 
	send_ok = false;
	
	if (unlikely(rmr_message->mtype == A1_POLICY_RESP)){
	  // for a1 messages we use send in high reliability mode and RTS
	  send_ok = Send(rmr_message, HIGH_RELIABILITY, RTS);
	}
	else{
	  rmr_message->state = 0; // fix for nng
	  send_ok = Send(rmr_message);   
	}
	attempts ++;
	
      	if (send_ok == false){
      	  error_handler(rmr_message);
	  fails ++;
      	}
	
      }
      
    }
  }
  

  // Clean up 
  try{
    rmr_free_msg(rmr_message);
  }
  catch(std::runtime_error &e){
    std::string identifier = __FILE__ +  std::string(", Line: ") + std::to_string(__LINE__) ; 
    std::string error_string = identifier = " Error freeing RMR message ";
    mdclog_write(MDCLOG_ERR, error_string.c_str(), "");
  }
  
  mdclog_write(MDCLOG_INFO, "Finished  thread %s : Recv = %lu, Tx Attempts = %lu, Tx Fail = %lu", thread_id.str().c_str(), recvs, attempts, fails);
}

// Template to allow a user defined processor to start
// on a specific thread 
template <typename messageProcessor>
unsigned int XaPP::StartThread(messageProcessor && msg_fn){

  std::lock_guard<std::mutex> guard(*_transmit);
  _listen = true;

  _num_threads++;
  thread_group.insert(std::make_pair(_num_threads, std::thread( ([&](){_workThread(msg_fn, std::bind(&XaPP::_error_handler, this, std::placeholders::_1), this);}))));
  return _num_threads;

};


// Template to allow a user defined processor and error handler to start
// on a specific thread 
template <typename messageProcessor, typename  errorHandler>
unsigned int XaPP::StartThread(messageProcessor && msg_fn, errorHandler && error_handler){

  std::lock_guard<std::mutex> guard(*_transmit);
  _listen = true;

  _num_threads++;
  thread_group.insert(std::make_pair(_num_threads, std::thread( ([&](){_workThread(msg_fn, error_handler, this);}))));
  return _num_threads;

};


#endif
