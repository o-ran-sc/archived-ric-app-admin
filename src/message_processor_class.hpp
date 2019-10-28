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


#pragma once
#include <iostream>
#include <cstring>
#include <chrono>
#include <rmr/RIC_message_types.h>
#include <rmr/rmr.h>
#include <mdclog/mdclog.h>
#include <NetworkProtector.h>
#include <e2ap_indication.hpp>
#include <e2ap_control.hpp>
#include <e2ap_control_response.hpp>
#include <subscription_handler.hpp>
#include <e2sm.hpp>

#ifdef __GNUC__
#define likely(x)  __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif


#define MAX_RMR_RECV_SIZE 2<<15

typedef enum{
	     NO_ERROR=0,
	     RMR_ERROR,
	     E2AP_INDICATION_ERROR,
	     E2AP_CONTROL_ERROR,
	     E2SM_INDICATION_HEADER_ERROR,
	     E2SM_CONTROL_HEADER_ERROR,
	     E2SM_CONTROL_MESSAGE_ERROR,
	     MISSING_HANDLER_ERROR,
	     BUFFER_ERROR,
	     PLUGIN_ERROR
} MessageProcessorStateTypes;
	     

typedef enum {
	      E2AP_PROC_ONLY = 0,
	      E2SM_PROC_ONLY,
	      ALL
} ProcessingLevelTypes;


class message_processor {

public:
  message_processor(int mode=ALL, bool rep=true, size_t buffer_length = 2048, size_t reporting_interval = 100);
  ~message_processor(void);
  
  bool operator() (rmr_mbuf_t *);  
  unsigned long const get_messages (void);
  void register_subscription_handler(subscription_handler *);
  void register_protector(protector * );
  void register_policy_handler (void  (*)(int, const char *, int, std::string &, bool));
  std::vector<double> & get_metrics(void);
  int get_state (void) {return state;};
private:

  E2N_E2AP_PDU_t * e2ap_recv_pdu;
  E2N_E2SM_gNB_X2_indicationHeader_t *e2sm_header; // used for decoding
  E2N_E2SM_gNB_X2_indicationMessage_t *e2sm_message; // used for decoding
  
  E2N_E2AP_PDU_t * e2ap_gen_pdu; // just a placeholder for now

  ric_indication_helper indication_data;
  ric_indication indication_processor;

  ric_control_helper control_data;
  ric_control_request control_request_processor;
  ric_control_response control_response_processor;
  
  e2sm_header_helper header_helper;
  e2sm_message_helper message_helper;
  e2sm_indication e2sm_indication_processor;
  e2sm_control e2sm_control_processor;
  
  subscription_handler * _ref_sub_handler;
  protector  * _ref_protector;
  void (* _ref_policy_handler) (int, const char *, int, std::string &, bool);
  unsigned long _num_messages;

  unsigned char *scratch_buffer;
  size_t remaining_buffer;
  size_t current_index;
  unsigned long int num_indications, num_err_indications;
  int state = NO_ERROR;
  int processing_level; // for debugging purposes
  bool report_mode_only; // suppress e2ap control
  size_t _buffer_size; // for storing encoding

  // these two parameters are used to report average processing latency.
  // processing latency is accumalated over every num_proc_packets
  // and both values are reported out on std log. After each report
  // counters are reset

  size_t _reporting_interval; // number of packets in a measurement interval
  size_t num_proc_packets;
  double processing_duration; // for storing time to process
  double processing_dev; // for storing standard deviation
  double max_proc_duration ;
};


extern int verbose_flag;
