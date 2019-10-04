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
 *  Created on: Aug 14, 2019
 *      Author: Ashwin Sridharan
 */

#ifndef SGNB_ADDITION_REQUEST_
#define SGNB_ADDITION_REQUEST_

  
#include <iostream>
#include <errno.h>
#include <mdclog/mdclog.h>
#include <sstream>
#include <sgnb_addition_helpers.hpp>

 
#include <X2AP-PDU.h>
#include <X2InitiatingMessage.h>
#include <ProtocolIE-Field.h>
#include <SgNBAdditionRequest.h>
#include <E-RABs-ToBeAdded-SgNBAddReq-Item.h>
#include <E-RABs-ToBeAdded-SgNBAddReq-Item-SgNBPDCPpresent.h>
#include <GBR-QosInformation.h>
#include <E-RABs-ToBeAdded-SgNBAddReq-Item-SgNBPDCPnotpresent.h>


#define NUM_SGNB_ADDITION_REQUEST_IES 8
#define INITIAL_SIZE 4

class sgnb_addition_request {
public:
  sgnb_addition_request(void);
  ~sgnb_addition_request(void);

  bool encode_sgnb_addition_request(unsigned char *, size_t *, sgnb_addition_helper &);
  bool set_fields(X2InitiatingMessage_t *, sgnb_addition_helper &);
  bool get_fields(X2InitiatingMessage_t * ,sgnb_addition_helper &);
  std::string get_error(void) const {return error_string; };
  
private:

  X2AP_PDU_t * x2ap_pdu_obj;
  X2InitiatingMessage_t * initMsg;
  SgNBAdditionRequest_IEs_t * IE_array;

  E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t * erab_array;
  size_t erab_array_size;
  
  E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent_t * erab_sgnb_present_array;
  E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent_t * erab_sgnb_notpresent_array;
  size_t erab_sgnb_present_array_size ;
  size_t erab_sgnb_notpresent_array_size;


  
  std::string error_string;
  char errbuf[128];
  size_t errbuf_len = 128;
};


#endif
