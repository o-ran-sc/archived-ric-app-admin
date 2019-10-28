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

   A mock e2term for testing 
   -- accepts a subscription request 
  -- accepts a delete subscription request
  

  -- when subscription is active,  sends out indication messages with X2AP SgNBAdditionRequest at specified rate
     and monitors control request

  -- when delete is requested, stops sending ...
  

*/


#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include <chrono>
#include <xapp_utils.hpp>
#include <getopt.h>
#include <rmr/RIC_message_types.h>
#include <xapp_utils.hpp>
#include <subscription_request.hpp>
#include <subscription_response.hpp>
#include <subscription_delete_request.hpp>
#include <subscription_delete_response.hpp>

#include <e2sm.hpp>
#include <e2ap_indication.hpp>
#include <e2ap_control.hpp>
#include <e2ap_control_response.hpp>
#include <sgnb_addition_request.hpp>
#include <sgnb_addition_response.hpp>

#define X2_SGNB_ADDITION_REQUEST "test-data/X2AP-PDU-SgNBAdditionRequest.per"


unsigned long int num_indications = 0;
unsigned long int num_controls = 0;
unsigned long int num_accepts = 0;
unsigned long int num_rejects = 0;
unsigned long int num_errors = 0;

bool verbose_flag = false;
bool RunProgram = true;
bool subscription_active = false;
int action_type = E2N_RICindicationType::E2N_RICindicationType_report;



void usage(char *command){
    std::cout <<"Usage : " << command << " ";
    std::cout <<" --name[-n] xapp_instance_name ";
    std::cout <<" --port[-p] port to listen on e.g tcp:4591 ";
    std::cout << "--verbose ";
    std::cout <<" --rate[-r] rate to send indication messages";
    std::cout << std::endl;
}

void EndProgram(int signum){
  std::cout <<"Signal received. Stopping program ....." << std::endl;
  RunProgram = false;
}

bool  Message_Handler(rmr_mbuf_t *message){

  bool res;
  int i;
  unsigned char meid[32];
  unsigned char src[32];
  
  subscription_helper he;
  subscription_response_helper he_resp;
  
  subscription_request sub_req;
  subscription_response sub_resp;

  subscription_delete sub_del_req;
  subscription_delete_response sub_del_resp;
  
  ric_control_request control_req;
  ric_control_response control_resp;
  ric_control_helper  control_data;

  e2sm_control e2sm_control_proc;
  
  asn_dec_rval_t retval;
  size_t mlen;
  
  E2N_E2AP_PDU_t *e2ap_pdu_recv = 0;
  X2N_X2AP_PDU_t *x2ap_pdu_recv = 0;
  E2N_E2SM_gNB_X2_eventTriggerDefinition_t *event = 0;
  std::vector<Action> * actions;

  bool send_msg = true;
  
  rmr_get_meid(message, meid);
  rmr_get_src(message, src);
  
  switch(message->mtype){
  case RIC_SUB_REQ:
    
    std::cout <<"*** Received message from src = " << src  << " for gNodeB = " << meid << " of size = " << message->len  << " and type = " << message->mtype << std::endl;
    
    e2ap_pdu_recv = 0;
    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_pdu_recv), message->payload, message->len);

    if(retval.code == RC_OK){
      he.clear();
      sub_req.get_fields(e2ap_pdu_recv->choice.initiatingMessage, he);
    }
    else{
      std::cerr <<"Error decoding E2AP Subscription response PDU. Reason = " << strerror(errno) << std::endl;
      send_msg = false;
      goto finished_sub_req;

    }

    std::cout <<"==============================\nReceived Subscription Request with ID = " << he.get_request_id() << std::endl;
    //xer_fprint(stdout, &asn_DEF_E2AP_PDU, e2ap_pdu_recv);


    // get action type (we support only one action in subscription request currently ....
    actions = he.get_list();
    action_type = (*actions)[0].get_type();
    std::cout << "Action type in subscription request ID = "<<  he.get_request_id() << " set to " << action_type << std::endl;
    
    // decode the event trigger ...
    {
      e2sm_event_trigger_helper event_trigger_data;
      event = 0; // used for decoding
      retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2SM_gNB_X2_eventTriggerDefinition, (void**)&(event), (unsigned char *)he.get_event_def(), he.get_event_def_size());
      if (retval.code != RC_OK){
	std::cerr <<"Error decoding event trigger in subscription request. Reason = " << strerror(errno) << std::endl;
	send_msg = false;
	goto finished_sub_req;
      }
      //std::cout <<"++++++++++++++ EVENT TRIGGER ++++++++++++++++++++++++++++++++++" << std::endl;
      //xer_fprint(stdout, &asn_DEF_E2N_E2SM_gNB_X2_eventTriggerDefinition, event);
      //std::cout <<"++++++++++++++ EVENT TRIGGER ++++++++++++++++++++++++++++++++++" << std::endl;
      
    }
    
    // set up response object
    he_resp.set_request(he.get_request_id(), he.get_req_seq());
    he_resp.set_function_id(he.get_function_id());
    i = 0;

    // ideally should move all actions to not admitted if failed
    // but we ignore admitted list anyway when we set up the PDU
    // for now, just copy to both lists :)  ...
    for(auto &e : *(he.get_list())){
      he_resp.add_action(e.get_id());
      he_resp.add_action(e.get_id(), 1, 2);
      
      i++;
    }
    mlen = RMR_BUFFER_SIZE;

    res = sub_resp.encode_e2ap_subscription_response(&message->payload[0], &mlen,  he_resp, true);
    if (!res){
      std::cerr << "Error encoding subscription response successful. Reason = " << sub_resp.get_error() << std::endl;
      send_msg = false;
      goto finished_sub_req;
    }
    message->mtype = RIC_SUB_RESP;
    subscription_active = true;
    
    
  finished_sub_req:
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
    ASN_STRUCT_FREE(asn_DEF_E2N_E2SM_gNB_X2_eventTriggerDefinition, event);
    if(send_msg){
      message->len = mlen;
      
      // also set subscription id ?
      std::cout <<"Sending Subscription Response with RMR type " << message->mtype << " and size = " << message->len << std::endl;
      std::cout <<"======================================" << std::endl;
      return true;
    }
    else{
      break;
    }
    
  case RIC_SUB_DEL_REQ:

    e2ap_pdu_recv = 0;
    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_pdu_recv), message->payload, message->len);
    if(retval.code == RC_OK){
      he.clear();
      sub_del_req.get_fields(e2ap_pdu_recv->choice.initiatingMessage, he);
    }
    else{
      std::cerr <<"Error decoding E2AP Subscription Delete Request PDU. Reason = " << strerror(errno) << std::endl;
      send_msg = false;
      goto finished_sub_del;
    }

    std::cout <<"==============================\nReceived Subscription Delete Request with ID = " << he.get_request_id() << std::endl;
    xer_fprint(stdout, &asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
    std::cout <<"==============================\nReceived Subscription Delete Request with ID = " << he.get_request_id() << std::endl;

    // FILE *pfile;
    // pfile = fopen("subscription_delete.per", "wb");
    // fwrite(message->payload, 1, message->len, pfile);
    // fclose(pfile);
    
    // set up response object
    std::cout <<"Generating response ...." << std::endl;
    he_resp.clear();
    he_resp.set_request(he.get_request_id(), he.get_req_seq());
    he_resp.set_function_id(he.get_function_id());
    mlen = RMR_BUFFER_SIZE;    
    res = sub_del_resp.encode_e2ap_subscription_delete_response(&message->payload[0], &mlen,  he_resp, true);
    if (!res){
      std::cerr << "Error encoding subscription delete  response failure . Reason = " << sub_resp.get_error() << std::endl;
      send_msg = false;
      goto finished_sub_del;
    }

  finished_sub_del:
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
    
    if(send_msg){
      message->mtype = RIC_SUB_DEL_RESP;
      message->len = mlen;
      subscription_active = false;
      return true;
    }
    else{
      break;
    }
    
  case RIC_CONTROL_REQ:
    num_controls ++;
    e2ap_pdu_recv = 0;
    retval =  asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_pdu_recv), message->payload, message->len);
    if(retval.code != RC_OK){
      std::cerr <<"Error decoding RIC Control Request" << std::endl;
      goto finished_ctrl;
    }
    
    if (verbose_flag){
      std::cout <<"++++++++ RECEIVED CONTROL REQUEST +++++++++++++++++++++++++++++" << std::endl;
      xer_fprint(stdout,  &asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
    }
    
    res = control_req.get_fields(e2ap_pdu_recv->choice.initiatingMessage, control_data);
    if(!res){
      std::cout <<"Error getting data from E2AP control request" << std::endl;
      break;
    }

    // Decode the X2AP PDU : directly embedded in Control Msg IE
    x2ap_pdu_recv = 0;
    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_X2N_X2AP_PDU, (void **)&x2ap_pdu_recv, control_data.control_msg, control_data.control_msg_size );
    if (retval.code != RC_OK){
      std::cerr <<"Error decoding X2AP PDU in control request message of size " << control_data.control_msg_size  << std::endl;
      num_errors ++;
      goto finished_ctrl;
    }
    
    if(verbose_flag){
      xer_fprint(stdout, &asn_DEF_X2N_X2AP_PDU, x2ap_pdu_recv);
      std::cout <<"++++++++ RECEIVED CONTROL REQUEST +++++++++++++++++++++++++++++" << std::endl;
      
    }

    if(x2ap_pdu_recv->present == X2N_X2AP_PDU_PR_successfulOutcome && x2ap_pdu_recv->choice.successfulOutcome->procedureCode == X2N_ProcedureCode_id_sgNBAdditionPreparation && x2ap_pdu_recv->choice.successfulOutcome->value.present == X2N_SuccessfulOutcome__value_PR_SgNBAdditionRequestAcknowledge ){
      num_accepts ++;
    }
    else if ( x2ap_pdu_recv->present == X2N_X2AP_PDU_PR_unsuccessfulOutcome && x2ap_pdu_recv->choice.unsuccessfulOutcome->procedureCode == X2N_ProcedureCode_id_sgNBAdditionPreparation && x2ap_pdu_recv->choice.unsuccessfulOutcome->value.present == X2N_UnsuccessfulOutcome__value_PR_SgNBAdditionRequestReject ){
      num_rejects ++;
    }
    else{
      std::cerr <<"Unknown X2AP PDU : message type = " << x2ap_pdu_recv->present << std::endl;
    }

    
    // generate control ack ? 
    // control_data.control_status = 0;
    // mlen = RMR_BUFFER_SIZE;
    // res = control_resp.encode_e2ap_control_response(message->payload, &mlen, control_data, true);

    // if (!res){
    //   std::cerr <<"Error encoding control response ack. Reason = " << control_resp.get_error() << std::endl;
    // }
  finished_ctrl:
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_pdu_recv);
    ASN_STRUCT_FREE(asn_DEF_X2N_X2AP_PDU, x2ap_pdu_recv);
    
    break;

  default:
    std::cerr <<"Error ! Unknown RMR message of type " << message->mtype << " received" << std::endl;
    break;
  }
  
  return false;
};




int main(int argc, char *argv[]){

  char name[128] = "mock_subscription_mgr";
  char port[16] = "tcp:4591";
  double rate = 1; // default rate to send at
  int verbose_flag = 0;
  
  // Parse command line options
  static struct option long_options[] =
    {
        /* Thse options require arguments */
        {"name", required_argument, 0, 'n'},
        {"port", required_argument, 0, 'p'},
	{"rate", required_argument, 0, 'r'},
	{"verbose", no_argument, &verbose_flag, 1},

    };


   while(1) {

        int option_index = 0;
        char c = getopt_long(argc, argv, "n:p:a:r:", long_options, &option_index);
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
	  

	case 'r':
	  rate = atof(optarg);
	  if (rate < 0){
	    std::cerr <<"Error : Transmit rate must be >= 0/sec" << std::endl;
	    exit(-1);
	  }
	  
	  if (rate > 1000){
	    std::cerr <<"Error : maximum allowed rate = 1000/sec" << std::endl;
	    exit(-1);
	  }
	  break;
	  
	case 'h':
	  usage(argv[0]);
	  exit(0);

	  
	default:
	  std::cout <<"Error ! " << std::endl;
	  usage(argv[0]);
	  exit(1);
        }
   };

   int log_level = MDCLOG_WARN;
   if (verbose_flag){
     log_level = MDCLOG_INFO;
   }
   init_logger(name, static_cast<mdclog_severity_t>(log_level));

   XaPP my_xapp = XaPP(name, port, 16384, 1);
   my_xapp.Start(Message_Handler);

   
   
   e2sm_indication e2sm_indication_obj;
   bool res;
   FILE *pfile;
   

   //==== e2sm indication header : just some
   e2sm_header_helper indication_header;
   // random fill for now 
   indication_header.egNB_id="hello";
   indication_header.plmn_id="there";
   indication_header.interface_direction = 1;
   indication_header.egNB_id_type = 2;
   unsigned char buf_header[32];
   size_t buf_header_size = 32;
   
   res = e2sm_indication_obj.encode_indication_header(&buf_header[0], &buf_header_size, indication_header);
   if(!res){
     std::cout <<"Error encoding indication header. Reason = " << e2sm_indication_obj.get_error() << std::endl;
     exit(-1);
   }

   //====== x2ap sgnb addition request created by us
   unsigned char x2ap_buf[1024];
   size_t x2ap_buf_size = 1024;
   pfile = fopen(X2_SGNB_ADDITION_REQUEST, "r");
   if(pfile == NULL){
     std::cerr <<"Error ! Could not find test per file " << X2_SGNB_ADDITION_REQUEST << std::endl;
     exit(-1);
   }
   
   x2ap_buf_size = fread((char *)x2ap_buf, sizeof(char), 1024, pfile);
   fclose(pfile);

  
   //==== e2ap indication for generated x2ap pdus
   ric_indication_helper dinput ;   
   dinput.action_id = 100;
   dinput.func_id = 10;
   dinput.indication_sn = 100;
   dinput.req_id = 6;
   dinput.req_seq_no = 11;

   

   /* encoding pdu put here */
   size_t data_size = 16384;
   unsigned char data[data_size];
   
   ric_indication indication_pdu;


   // prepare packet to send. we send
   // same packet every time for now
   dinput.indication_header = buf_header;
   dinput.indication_header_size = buf_header_size;
   dinput.indication_msg = x2ap_buf;
   dinput.indication_msg_size = x2ap_buf_size;
   dinput.indication_type = 1; // for now always ask for control back
   res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
   if (!res){
     std::cout <<"Error encoding E2AP Indication PDU. Reason = " << indication_pdu.get_error().c_str() <<  std::endl;
     exit(-1);
   }
       

   //Register signal handler to stop 
   signal(SIGINT, EndProgram);
   signal(SIGTERM, EndProgram);

   unsigned long int interval = 0;

   if (rate > 0){
     interval = 1000.0/rate;
   }
   else{
     interval = 10;
   }
   
   auto start_time = std::chrono::steady_clock::now();
   int count = 0;


   while(RunProgram){

     if ( subscription_active && rate > 0 ){
       my_xapp.Send(RIC_INDICATION, data_size, data);
       num_indications ++;
     }
     
     std::this_thread::sleep_for(std::chrono::milliseconds(interval));
     count ++;
   }
   
   
   auto end_time = std::chrono::steady_clock::now();
   double  duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

   std::cout <<"Number of SgNB Addition Request (in E2AP Indication) sent = " << num_indications << std::endl;
   std::cout <<"Number of control responses received = " << num_controls << std::endl;
   std::cout <<"Number of SgNB Addition Acknowledges = " << num_accepts << std::endl;
   std::cout <<"NUmber of SgNB Addition Rejects = " << num_rejects << std::endl;
   std::cout <<"Number of E2AP control requests we could not decode = " << num_errors << std::endl;
   
   std::cout <<"Duration = " << duration  << " E2AP Indication Tx Rate = " << num_indications/duration << std::endl;
   my_xapp.Stop();

   std::cout <<"Stopped RMR thread .." << std::endl;
   return 0;
      

};





   
