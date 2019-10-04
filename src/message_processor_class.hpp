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

#define BUFFER_LENGTH 2048

class message_processor {

public:
  message_processor();
  ~message_processor(void);
  
  bool operator() (rmr_mbuf_t *);  
  unsigned long const get_messages (void);
  void register_subscription_handler(SubscriptionHandler *);
  void register_protector(protector * );
  void register_policy_handler (void  (*)(int, const char *, int, std::string &, bool));
  std::vector<double> & get_metrics(void);
  
private:

  E2AP_PDU_t * e2ap_recv_pdu;
  E2SM_gNB_X2_indicationHeader_t *e2sm_header; // used for decoding
  E2SM_gNB_X2_indicationMessage_t *e2sm_message; // used for decoding
  
  E2AP_PDU_t * e2ap_gen_pdu; // just a placeholder for now

  E2APRicIndication indication_data;
  ric_indication indication_processor;

  ric_control_helper control_data;
  ric_control_request control_request_processor;
  ric_control_response control_response_processor;
  
  e2sm_header_helper header_helper;
  e2sm_message_helper message_helper;
  e2sm_indication e2sm_indication_processor;
  e2sm_control e2sm_control_processor;
  
  SubscriptionHandler * _ref_sub_handler;
  protector  * _ref_protector;
  void (* _ref_policy_handler) (int, const char *, int, std::string &, bool);
  unsigned long _num_messages;

  unsigned char *scratch_buffer;
  size_t remaining_buffer;
  unsigned int current_index;
  unsigned long int num_indications, num_err_indications;
};


extern bool report_mode_only;
extern int verbose_flag;
