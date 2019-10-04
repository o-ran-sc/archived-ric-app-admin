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
/*
 * ric_indication.h
 *
 *  Created on: Jul 11, 2019
 *      Author: sjana, Ashwin Sridharan
 */

#ifndef E2AP_RIC_INDICATION_H_
#define E2AP_RIC_INDICATION_H_

  
#include <iostream>
#include <errno.h>
#include <mdclog/mdclog.h>
#include <sstream>
#include <E2AP-PDU.h>
#include <InitiatingMessage.h>
#include <RICindication.h>
#include <ProtocolIE-Field.h>

#define NUM_INDICATION_IES 7
  
  typedef struct E2APRicIndication E2APRicIndication;
  
  struct E2APRicIndication{
    E2APRicIndication(void) : req_id(1), req_seq_no(1), func_id(0), action_id(1), indication_type(0), indication_sn(0), indication_msg(0), indication_msg_size(0), indication_header(0), indication_header_size(0), call_process_id(0), call_process_id_size(0) {};
    long int req_id, req_seq_no, func_id, action_id, indication_type, indication_sn;
    
    unsigned char* indication_msg;
    size_t indication_msg_size;
    
    unsigned char* indication_header;
    size_t indication_header_size;

    unsigned char *call_process_id;
    size_t call_process_id_size;
    
  };
  
  
  class ric_indication{
    
  public:
    ric_indication(void);
    ~ric_indication(void);
    
    bool encode_e2ap_indication(unsigned char *, size_t *,  E2APRicIndication &);
    InitiatingMessage_t * get_message (void) ;
    bool set_fields(InitiatingMessage_t *, E2APRicIndication &);
    bool get_fields(InitiatingMessage_t *, E2APRicIndication &);
    std::string get_error(void) const {return error_string ; };
  private:

    E2AP_PDU_t * e2ap_pdu_obj;
    InitiatingMessage_t *initMsg;
    RICindication_IEs_t *IE_array;
    std::string error_string;
    char errbuf[128];
    size_t errbuf_len = 128;
  };


#endif /* E2AP_RIC_INDICATION_H_ */
