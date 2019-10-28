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
   Sends different kind of policy requests (valid/invalid) and prints out response 
*/

#include <limits>
#include <map>
#include <getopt.h>
#include <csignal>
#include <time.h>
#include <xapp_utils.hpp>
#include <vector>
#include <rmr/RIC_message_types.h>

std::string gNodeB = "";

bool rcv_message(rmr_mbuf_t *message){
  std::string response;
  switch(message->mtype){
  case DC_ADM_INT_CONTROL_ACK:
    std::cout <<"Received response = " << (char *)message->payload << " of len = " << strlen((char *)message->payload) << "Actual = " << message->len << std::endl;
    break;

  case DC_ADM_GET_POLICY_ACK:
    std::cout <<"Received Policy  = " << (char *)message->payload << " of len = " << strlen((char *)message->payload) << "Actual = " << message->len << std::endl;
    break;
    
  default:
    std::cout <<"Unknown RMR message of type " << message->mtype << " received" << std::endl;
  }
  
  return false;
}


void usage(char *command){
    std::cout <<"Usage : " << command << " ";
    std::cout <<" --name[-n] xapp_instance_name ";
    std::cout <<" --port[-p] port to listen on (default is tcp:4561) ";
    std::cout <<"--schema[-s] schema file";
    
    std::cout << std::endl;
}


void msg_error(rmr_mbuf_t *message){
  mdclog_write(MDCLOG_ERR, "Error sending message of length %d and type %d, Reason %d",  message->len,  message->mtype, errno );
};


int main(int argc, char *argv[]){

  char name[128] = "test_a1_client";
  char port[16] = "tcp:4560";
  unsigned int num_threads = 1;
  std::unique_ptr<XaPP> my_xapp;
  
  // Parse command line options
  static struct option long_options[] = 
    {

	/* Thse options require arguments */
	{"name", required_argument, 0, 'n'},
        {"port", required_argument, 0, 'p'},
	
    };


   while(1) {

	int option_index = 0;
	char c = getopt_long(argc, argv, "n:p:", long_options, &option_index);

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
	  
	  
	case 'h':
	  usage(argv[0]);
	  exit(0);
	  
	default:
	  usage(argv[0]);
	  exit(1);
        }
   };

   init_logger(name, MDCLOG_INFO);
   
   mdclog_write(MDCLOG_INFO, "XaPP name specified = %s", name);
   mdclog_write(MDCLOG_INFO, "XaPP port specified = %s", port);

   mdclog_write(MDCLOG_INFO,"XaPP listener threads specified = auto");
   my_xapp = std::make_unique<XaPP>(name, port, 1024, 1);
   
   
   // Start receiving loop ...
   std::vector<int> thread_ids(num_threads);
   for(unsigned int i = 0; i < num_threads; i++){
     thread_ids[i] = (*my_xapp).StartThread(&rcv_message, msg_error);
     i++;
   };
   
   bool enforce = true;
   int block_rate = 2;
   int window_length = 20;
   int trigger_threshold = 40;
   char buffer[1024];
   while(1){
     
    // Send a valid config
     std::string message_string ;
     std::string start = "{";
     std::string end = "}";

     message_string = start + "\"enforce\":" + (enforce? "true":"false") + ",";
     message_string += std::string("\"blocking_rate\":") + std::to_string(block_rate) + ",";
     message_string += std::string("\"window_length\":") + std::to_string(window_length) + ",";
     message_string += std::string("\"trigger_threshold\":") + std::to_string(trigger_threshold) + end;
     memcpy(buffer, message_string.c_str(), message_string.length());     
     my_xapp.get()->Send(DC_ADM_INT_CONTROL,  message_string.length(),  buffer);

     
     sleep(2);
     
     // // Send an invalid config
     message_string = start + "\"enfce\":" + (enforce? "true":"false") + ",";
     message_string += std::string("\"blocking_rate\":") + std::to_string(block_rate) + ",";
     message_string += std::string("\"window_length\":") + std::to_string(window_length) + end;  
     memcpy(buffer, message_string.c_str(), message_string.length());     
     my_xapp.get()->Send(DC_ADM_INT_CONTROL,  message_string.length(),  buffer);
     sleep(2);
     
     // Send invalid JSON
     message_string.assign("\"enforce\":false,");
     message_string += std::string("\"blocking_rate\":") + std::to_string(block_rate) + ",";
     message_string += std::string("\"window_length\":") + std::to_string(window_length) + end;  
     memcpy(buffer, message_string.c_str(), message_string.length());
     my_xapp.get()->Send(DC_ADM_INT_CONTROL, message_string.length(), buffer);
     
     sleep(2);


     // Send request for policy
     // we don't care about contents of request for now ...
     std::cout <<"Sending request to get policy" << std::endl;
     my_xapp.get()->Send(DC_ADM_GET_POLICY, message_string.length(), buffer);
     sleep(2);

    window_length += 1;
     
   }
   
   (*my_xapp).Stop();  
   
}
