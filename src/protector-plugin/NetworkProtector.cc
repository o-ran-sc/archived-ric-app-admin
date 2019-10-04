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


#include "NetworkProtector.h"


#include <mutex>          // std::mutex
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()


protector::protector(bool enforce,   int windowSize_, int threshold_, double blockRate_):  m_enforce(enforce),  m_windowSize(windowSize_), m_threshold(threshold_), m_blockRate(blockRate_), m_req(0), m_rej(0)
{	
  m_counter = 0;
  m_window_ref = std::make_unique<sliding_window>(m_windowSize);
  m_access = std::make_unique<std::mutex>();
}


bool protector::operator()(unsigned char *msg_ref, size_t msg_size, unsigned char * buffer, size_t *buf_size )
{
  
  bool res = true;
  
  std::lock_guard<std::mutex> lck(*(m_access.get()));

  X2AP_PDU_t * x2ap_recv = 0;

  // /* Decode */
  asn_dec_rval_t dec_res = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_X2AP_PDU, (void **)&x2ap_recv, msg_ref, msg_size);

  /* Is this an SgNB Addition request ? */
  if (dec_res.code == RC_OK){
    if (x2ap_recv->present == X2AP_PDU_PR_initiatingMessage){
      if (x2ap_recv->choice.initiatingMessage->procedureCode ==  ProcedureCode_id_sgNBAdditionPreparation ){
	if (x2ap_recv->choice.initiatingMessage->value.present ==  X2InitiatingMessage__value_PR_SgNBAdditionRequest){
	  mdclog_write(MDCLOG_INFO, "Processing X2AP SgNB Addition Request message\n");
	  res = true;
	}
	else{
	  mdclog_write(MDCLOG_ERR, "Error :: %s, %d:: X2AP Message (%d) not an SgNB Addition Request\n", __FILE__, __LINE__, x2ap_recv->choice.initiatingMessage->value.present);
	  res = false;
	}
      }
      else{
	mdclog_write(MDCLOG_ERR, "Error :: %s, %d:: X2AP procedure code  %d does not match required %d\n", __FILE__, __LINE__, x2ap_recv->choice.initiatingMessage->procedureCode, ProcedureCode_id_sgNBAdditionPreparation);
	res = false;
      }
    }
    else{
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d:: Not an X2AP initiating message. X2AP Message type %d\n", __FILE__, __LINE__, x2ap_recv->present);
      res = false;
    }
  }
  else{
    mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: Could not decode X2AP PDU of size %lu bytes \n", __FILE__, __LINE__, msg_size);
    res = false;
  }

  if (res){
    
    //std::cout <<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
    //xer_fprint(stdout,  &asn_DEF_X2AP_PDU, x2ap_recv);
    //std::cout <<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;

    res = sgnb_req.get_fields(x2ap_recv->choice.initiatingMessage, sgnb_data);
    if (!res){
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: could not get  fields from SgNB Addition Request. Reason = %s\n", __FILE__, __LINE__, sgnb_req.get_error().c_str());
    }
    
    if (res){
      // Admission control
      
      m_req ++;
      // update sliding window
      m_window_ref.get()->update_window(1);
      if (m_window_ref.get()->net_events  > m_threshold && m_enforce){
	res = selectiveBlock();
      }
      else{
	res = true;
      }

      if (!res){
          m_rej ++;
      }

      /*
	Generate response message 
	Do we need to do this ? 
	What if indication is report type ?
	plugin is agnostic to subscription for now, so yes, we always generate
	an appropriate response
      */
      
      // generate sgnb addition response message (ack or reject)
      // if rejecting , we use cause = Misc and sub-cause  = om_intervention
      sgnb_data.cause = Cause_PR_misc;
      sgnb_data.cause_desc = CauseMisc_om_intervention;
      res = sgnb_resp.encode_sgnb_addition_response(buffer, buf_size, sgnb_data, res);
      if (! res){
	mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: could not encode SgNB Addition Response PDU. Reason = %s\n", __FILE__, __LINE__, sgnb_resp.get_error().c_str());
      }
    }
  }

  ASN_STRUCT_FREE(asn_DEF_X2AP_PDU, x2ap_recv);
  return res;
  
}

bool protector::configure(bool enforce, int windowSize_, int threshold_, double blockRate_)
{
  std::lock_guard<std::mutex> lck(*(m_access.get()));
  

  
  m_windowSize=windowSize_;
  bool res = m_window_ref.get()->resize_window(m_windowSize);
  if (!res){
    return false;
  }

  m_enforce = enforce;
  m_threshold=threshold_;
  m_blockRate=blockRate_;

  mdclog_write(MDCLOG_INFO, "Policy : Enforce mode set to %d\n", m_enforce);
  mdclog_write(MDCLOG_INFO, "Policy:  Trigger threshold set to %d\n", m_threshold);
  mdclog_write(MDCLOG_INFO, "Policy : Blocking rate set to %f\n", m_blockRate);
  mdclog_write(MDCLOG_INFO, "Policy : Window Size set to %d\n", m_windowSize);
  return true;
}

unsigned long int protector::get_requests(void) const {
  return m_req;
}

unsigned long int protector::get_rejects(void) const {
  return m_rej;
}

void protector::clear()
{
  std::lock_guard<std::mutex> lck(*(m_access.get()));
  m_req = 0;
  m_rej = 0;
  m_window_ref.get()->clear();
}


bool protector::selectiveBlock() 
{
  unsigned int num = (rand() % 100) + 1;    
  if (num > m_blockRate)  //not blocking
    return true;
  else                    //blocking
    return false;
}

