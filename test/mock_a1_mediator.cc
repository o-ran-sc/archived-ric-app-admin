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

   A sample test client to demonstrate A1 functionality.
   Sends different kind of policy requests (valid/invalid), create/update/delete and prints out response 
*/

#include <limits>
#include <map>
#include <getopt.h>
#include <csignal>
#include <time.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <xapp_utils.hpp>
#include <vector>
#include <rmr/RIC_message_types.h>

#define MAX_TIMEOUTS 2

std::string gNodeB = "";
std::mutex notify_lock;
std::condition_variable notify_var;

bool rcv_message(rmr_mbuf_t *message){
  switch(message->mtype){
  case A1_POLICY_RESP:
    {
      std::lock_guard<std::mutex> lck(notify_lock);
      std::cout <<"A1 Mediator received response = " << (char *)message->payload << " of len = " << strlen((char *)message->payload) << std::endl;
    }
    // released the lock. notify the sleep thread (if any)
    notify_var.notify_all();
    break;

  default:
    std::cout <<"Unknown RMR message of type " << message->mtype << " received" << std::endl;
  }
  
  return false;
}


void usage(char *command){
    std::cout <<"Usage : " << command << " " << std::endl;
    std::cout <<" --name[-n] xapp_instance_name " << std::endl;
    std::cout <<" --port[-p] port to listen on (default is tcp:4561) " << std::endl;
    std::cout << " --op[-o] operation mode from {0, 1, 2} : 0 is CREATE, 1 is UPDATE, 2 is DELETE" << std::endl;
    std::cout << " --window [-w] window size in seconds (default is 60 seconds)" << std::endl;
    std::cout << " --blockrate [-b] blocking rate percentage (default is 90%)" << std::endl;
    std::cout << " --trigger[-t] trigger threshold (default is 40 requests in window)" << std::endl;
    std::cout << " --enforce  set policy to enforce if flag prvoided (default is 0 i.e not enforce)" << std::endl;
    std::cout << " --class [-c] subscriber profile id to which policy must be applied (default is 5)" << std::endl;
    std::cout << " --instance[-i]  policy instance id (default is ac-xapp-1)" << std::endl;
    std::cout << std::endl;
}


void msg_error(rmr_mbuf_t *message){
  mdclog_write(MDCLOG_ERR, "Error sending message of length %d and type %d, Reason %d",  message->len,  message->mtype, errno );
};


int main(int argc, char *argv[]){

  
  char name[128] = "test_a1_client";
  char port[16] = "tcp:9000";
  unsigned int num_threads = 1;
  std::unique_ptr<XaPP> my_xapp;
  std::string schema_file;

  enum OPERATIONS{CREATE, UPDATE, DELETE};
  static const char * op_strings[] = {"CREATE", "UPDATE", "DELETE"};
  
  OPERATIONS op = CREATE;
  std::string instance_id = "ac-xapp-1";
  int class_id = 5;
  int enforce = 0;
  int blocking_rate = 90; // percentage
  int window_length = 60; // seconds
  int trigger_threshold = 40;

  std::chrono::seconds time_out(1);
  
  // Parse command line options
  static struct option long_options[] = 
    {

     /* Thse options require arguments */
     {"name", required_argument, 0, 'n'},
     {"port", required_argument, 0, 'p'},
     {"window", required_argument, 0, 'w'},
     {"blockrate", required_argument, 0, 'b'},
     {"trigger", required_argument, 0, 't'},
     {"class", required_argument, 0, 'c'},
     {"op", required_argument, 0, 'o'},
     {"instance", required_argument, 0, 'i'},
     {"enforce", no_argument, &enforce, 1},

    };


  while(1) {

    int option_index = 0;
    char c = getopt_long(argc, argv, "n:p:w:b:t:c:o:i:", long_options, &option_index);

    if(c == -1){
      break;
    }
    
    switch(c)
      {
       
      case 0:
	/* An option flag was set. 
	   Do nothing for now */
	break;
	  
      case 'n':
	strcpy(name, optarg);
	break;
	  
      case 'p':
	strcpy(port, optarg);
	break;
	  
      case 'w':
	window_length = atoi(optarg);
	break;

      case 't':
	trigger_threshold = atoi(optarg);
	break;

      case 'b':
	blocking_rate = atof(optarg);
	break;

      case 'o':
	op = static_cast<OPERATIONS>(atoi(optarg));
	break;

      case 'i':
	instance_id.assign(optarg);
	break;

      case 'c':
	class_id = atoi(optarg);
	break;
	
      case 'h':
	usage(argv[0]);
	exit(0);
	  
      default:
	usage(argv[0]);
	exit(1);
      }
  };

  int log_level = MDCLOG_INFO;
  init_logger(name, static_cast<mdclog_severity_t>(log_level));
 
  mdclog_write(MDCLOG_INFO, "XaPP name specified = %s", name);
  mdclog_write(MDCLOG_INFO, "XaPP port specified = %s", port);

   init_logger(name, MDCLOG_INFO);
   
   mdclog_write(MDCLOG_INFO, "XaPP name specified = %s", name);
   mdclog_write(MDCLOG_INFO, "XaPP port specified = %s", port);

   mdclog_write(MDCLOG_INFO,"XaPP listener threads specified = auto");
   my_xapp = std::make_unique<XaPP>(name, port, 16384);
   
   
   // Start receiving loop ...
   std::vector<int> thread_ids(num_threads);
   for(unsigned int i = 0; i < num_threads; i++){
     thread_ids[i] = (*my_xapp).StartThread(rcv_message, msg_error);
     i++;
   };
   

   char buffer[1024];
   std::string message_string ;
   std::stringstream policy;
   std::stringstream msg;
   bool res = false;
   switch(op){
     
   case CREATE:
   case UPDATE:
     policy <<"{ " << "\"enforce\":true, " << "\"window_length\":" << window_length << " , \"trigger_threshold\":" << trigger_threshold << ",  \"blocking_rate\":" << blocking_rate << ", \"class\":" << class_id << " }" ;
     
     // Send a create/update
     msg << "{ " << "\"policy_type_id\":" << 21000 << "," << "\"policy_instance_id\":\"" << instance_id << "\", \"operation\":\"" << op_strings[op]  << "\", \"payload\" :" << policy.str() << "}";
     res = true;
     break;
     
   case DELETE:
     // send a delete 
     msg << "{ " << "\"policy_type_id\":" << 21000 << "," << "\"policy_instance_id\": \"" << instance_id << "\", \"operation\": \"" << op_strings[op]  <<  "\" }";
     res = true;
     break;
     
   default:
     std::cerr <<"Not yet supported " << std::endl;

   }

   if(res){
     message_string = msg.str();
     std::cout <<"Sending message = " << message_string << std::endl;
     memcpy(buffer, message_string.c_str(), message_string.length());
     my_xapp.get()->Send(A1_POLICY_REQ,  message_string.length(),  buffer, link_types::HIGH_RELIABILITY);
   }

   std::unique_lock<std::mutex> lck(notify_lock);
   
   // release lock and got to sleep waiting to be notified
   notify_var.wait_for(lck,  std::chrono::seconds(5));
     
   // finish 
   (*my_xapp).Stop();  
   
}
