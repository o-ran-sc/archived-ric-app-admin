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


protector::protector( bool report){	
  m_access = std::make_unique<std::mutex>();
  report_mode_only = report;

  // there is always a default policy with id 0 (never gets deleted, can only be re-configured)
  // default values from policy constructor will be used.
  policy_list.insert(std::pair<int, protector_policy>(0, protector_policy()));

}

// constructor that over-rides default values for policy 0
protector::protector( bool enforce, int window_size, int threshold, double blocking_rate, bool report){	
  m_access = std::make_unique<std::mutex>();
  report_mode_only = report;

  // there is always a default policy with id 0 (never gets deleted, can only be re-configured)
  policy_list.insert(std::pair<int, protector_policy>(0, protector_policy(enforce, window_size, threshold, blocking_rate)));
  
}


bool protector::operator()(unsigned char *msg_ref, size_t msg_size, unsigned char * buffer, size_t *buf_size ){
  
  bool res = true;
  protector_policy * policy_ref;
  
  std::lock_guard<std::mutex> lck(*(m_access.get()));
  
  X2N_X2AP_PDU_t * x2ap_recv = 0;
  asn_dec_rval_t dec_res;
  
  // /* Decode */
  dec_res = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_X2N_X2AP_PDU, (void **)&x2ap_recv, msg_ref, msg_size);
  
  if (dec_res.code == RC_OK){
    /* Is this an SgNB Addition request ? */
    mdclog_write(MDCLOG_DEBUG, "Decoded X2AP PDU successfully. Processing X2 message fields to ascertain type etc ...\n");
    if (x2ap_recv->present == X2N_X2AP_PDU_PR_initiatingMessage){
      if (x2ap_recv->choice.initiatingMessage->procedureCode ==  X2N_ProcedureCode_id_sgNBAdditionPreparation ){
	if (x2ap_recv->choice.initiatingMessage->value.present ==  X2N_InitiatingMessage__value_PR_SgNBAdditionRequest){
	  mdclog_write(MDCLOG_DEBUG, "Processing X2AP SgNB Addition Request message\n");
	  res = true;
	}
	else{
	  mdclog_write(MDCLOG_ERR, "Error :: %s, %d:: X2AP Message (%d) not an SgNB Addition Request\n", __FILE__, __LINE__, x2ap_recv->choice.initiatingMessage->value.present);
	  res = false;
	}
      }
      else{
	mdclog_write(MDCLOG_ERR, "Error :: %s, %d:: X2AP procedure code  %ld does not match required %ld\n", __FILE__, __LINE__, x2ap_recv->choice.initiatingMessage->procedureCode, X2N_ProcedureCode_id_sgNBAdditionPreparation);
	res = false;
      }
    }
    else{
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d:: Not an X2AP initiating message. X2AP Message type %d\n", __FILE__, __LINE__, x2ap_recv->present);
      res = false;
    }
  }
  else{
    mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: Could not decode X2AP PDU of size %lu bytes\n", __FILE__, __LINE__, msg_size);
    res = false;
  }
  
  if (res){
    
    mdclog_write(MDCLOG_DEBUG, "Extracting SgNB Addition Request fields...");

    //std::cout <<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
    //xer_fprint(stdout,  &asn_DEF_X2AP_PDU, x2ap_recv);
    //std::cout <<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;

    res = sgnb_req.get_fields(x2ap_recv->choice.initiatingMessage, sgnb_data);
    if (!res){
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: could not get  fields from SgNB Addition Request. Reason = %s\n", __FILE__, __LINE__, sgnb_req.get_error().c_str());
    }
    
    if (res){
      mdclog_write(MDCLOG_DEBUG, "Decoded and extracted X2AP PDU data. Number of erabs = %lu\n", sgnb_data.get_list()->size());
      mdclog_write(MDCLOG_DEBUG, "Applying admission control logic ...");

      // Find if policy associated with this subscription id
      auto it_policy = policy_list.find(sgnb_data.subscriber_profile_id);
      
      if (it_policy == policy_list.end()){
	// apply default policy
	policy_ref = & policy_list[0];
      }
      else{
	policy_ref = & (it_policy->second);
      }

      net_requests ++;
      policy_ref->_req++;	
      policy_ref->_window_ref.get()->update_window(1);

      // apply blocking probability if m_enforce
      if ( policy_ref->_enforce && policy_ref->_window_ref.get()->net_events  > policy_ref->_threshold){
	res = selectiveBlock(policy_ref->_block_rate);
      }
      else{
	res = true;
      }

      if (!res){
	policy_ref->_rej ++;
	net_rejects ++;
      }
      
      mdclog_write(MDCLOG_DEBUG, "Plugin decision for sgnb request = %d\n", res);
      
      /*
	Generate response message if flag is set
      */
      
      if(! report_mode_only){
	// generate sgnb addition response message (ack or reject)
	// if rejecting , we use cause = Misc and sub-cause  = om_intervention
	mdclog_write(MDCLOG_DEBUG, "Generating X2AP response ..\n");
	sgnb_data.cause = X2N_Cause_PR_misc;
	sgnb_data.cause_desc = X2N_CauseMisc_om_intervention;
	try{
	  res = sgnb_resp.encode_sgnb_addition_response(buffer, buf_size, sgnb_data, res);
	  if (! res){
	    mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: could not encode SgNB Addition Response PDU. Reason = %s\n", __FILE__, __LINE__, sgnb_resp.get_error().c_str());
	  }
	}
	catch(std::exception &e){
	  mdclog_write(MDCLOG_ERR, "Error:: %s, %d : Caught exception %s\n", __FILE__, __LINE__, e.what());
	}
	
      }
      else{
	res = true;
      }
    }
  }
 
  
  ASN_STRUCT_FREE(asn_DEF_X2N_X2AP_PDU, x2ap_recv);
  return res;
  
}
  
// configure an existing policy 
bool protector::configure(bool enforce, int windowSize_, int threshold_, double blockRate_, int id){
  std::lock_guard<std::mutex> lck(*(m_access.get()));
  std::stringstream ss;
  
  // basic validation of  input
  if(windowSize_ <= 0){
    ss << "Illegal value for window size = " << windowSize_ << " when configuring policy " << std::endl;
    error_string = ss.str();
    return false;
  }

  if(threshold_ < 0){
    ss << "Illegal value for trigger threshold = " << threshold_ << " when configuring policy " << std::endl;
    error_string = ss.str();
    return false;
  }

  if(blockRate_ < 0 || blockRate_ > 100){
    ss << "Illegal value for blocking rate = " << blockRate_ << " when configuring policy " << std::endl;
    error_string = ss.str();
    return false;
  }
  if (id < 0){
    ss << "Illegal value for class id  = " << id << " when configuring policy " << std::endl;
    error_string = ss.str();
    return false;
  }
	       
  // find policy
  auto policy_it = policy_list.find(id);
  if (policy_it == policy_list.end()){
    mdclog_write(MDCLOG_ERR, "Error : %s, %d : No policy with id %d found for configuration\n", __FILE__, __LINE__,  id);
    return false;
  }
  
  bool res = policy_it->second._window_ref.get()->resize_window(windowSize_);
  if (!res){
    error_string = policy_it->second._window_ref.get()->get_error();
    return false;
  }
  policy_it->second._window_size = windowSize_;
  
  // enforce is set globally
  policy_it->second._enforce = enforce;
  
  policy_it->second._threshold=threshold_;
  policy_it->second._block_rate=blockRate_;
  mdclog_write(MDCLOG_DEBUG, "Configured policy with id %d with enforce=%d, window size = %d, threshold = %d, blocking rate = %f\n", id, policy_it->second._enforce, policy_it->second._window_size, policy_it->second._threshold, policy_it->second._block_rate);
  
  return true;
}

// add a policy
bool protector::add_policy(bool enforce, int windowSize_, int threshold_, double blockRate_, int id){
  std::lock_guard<std::mutex> lck(*(m_access.get()));
  std::stringstream ss;


    // find policy
  auto policy_it = policy_list.find(id);
  if (policy_it != policy_list.end()){
    ss <<"Error : " << __FILE__ << "," << __LINE__ << ": " << "Policy with id " << id << " already exists. Cannot be added" << std::endl;
    error_string = ss.str();
    mdclog_write(MDCLOG_ERR, "%s\n", error_string.c_str());
    return false;
  }

  // basic validation of  input
  if(windowSize_ <= 0){
    ss << "Illegal value for window size = " << windowSize_ << " when adding policy " << std::endl;
    error_string = ss.str();
    return false;
  }
  if(threshold_ < 0){
    ss << "Illegal value for trigger threshold = " << threshold_ << " when adding policy " << std::endl;
    error_string = ss.str();
    return false;
  }

  if(blockRate_ < 0 || blockRate_ > 100){
    ss << "Illegal value for blocking rate = " << blockRate_ << " when adding policy " << std::endl;
    error_string = ss.str();
    return false;
  }
  if (id < 0){
    ss << "Illegal value for class id  = " << id << " when adding policy " << std::endl;
    error_string = ss.str();
    return false;
  }
	       

  // create the policy
  try{
    policy_list.insert(std::pair<int,  protector_policy> (id, protector_policy(enforce, windowSize_, threshold_, blockRate_)));
  }
  catch(std::exception &e){
    ss <<"Error : " << __FILE__ << "," << __LINE__ << ": " << "Error creating policy. Reason = " << e.what() << std::endl;
    error_string = ss.str();
    mdclog_write(MDCLOG_ERR, "%s\n", error_string.c_str());
    return false;
  }

  mdclog_write(MDCLOG_DEBUG, "Added new policy with id %d with enforce=%d, window size = %d, threshold = %d, blocking rate = %f\n", id, enforce, windowSize_, threshold_, blockRate_);

  return true;
}

// delete a policy
bool protector::delete_policy(int id){
  std::lock_guard<std::mutex> lck(*(m_access.get()));
  std::stringstream ss;
  auto policy_it = policy_list.find(id);
  if (policy_it == policy_list.end()){
    ss <<"Error : " << __FILE__ << "," << __LINE__ << ": " << " No policy with id  = " << id << " found" << std::endl;
    error_string = ss.str();
    mdclog_write(MDCLOG_ERR, "%s\n", error_string.c_str());
    return false;
  }

  policy_list.erase(policy_it);
  mdclog_write(MDCLOG_DEBUG, "Deleted policy %d\n", id);
  return true;
}


// query a policy : responsibility of caller to ensure
// vector is empty
// returns parameters of policy in the vector
bool protector::query_policy(int id, std::vector<double> & info){

  std::lock_guard<std::mutex> lck(*(m_access.get()));
  auto policy_it = policy_list.find(id);
  if (policy_it == policy_list.end()){
    return false;
  }

  info.push_back(policy_it->second._enforce);
  info.push_back(policy_it->second._window_size);
  info.push_back(policy_it->second._threshold);
  info.push_back(policy_it->second._block_rate);
  return true;
}

// returns requests that fall under a policy
// if id is -1, returns total requests
// if non-existent policy, returns -1
// counters are cumulative
long int protector::get_requests(int id) const {
  if (id == -1){
    return net_requests;
  }

  std::lock_guard<std::mutex> lck(*(m_access.get()));
  auto policy_it = policy_list.find(id);
  if (policy_it == policy_list.end()){
    return -1;
  }
  else{
    return policy_it->second._req;
  }
  
}

// returns requests that fall under a policy
// if id is -1 , returns total rejects
// if non-existent policy, returns -1
// counters are cumulative
long int protector::get_rejects(int id) const {

  if (id == -1){
    return net_rejects;
  }

  std::lock_guard<std::mutex> lck(*(m_access.get()));
  auto policy_it = policy_list.find(id);
  if (policy_it == policy_list.end()){
    return -1;
  }
  else{
    return policy_it->second._rej;
  }

}

// returns list of active policies in
// supplied vector (policy is indexed by subscriber profile id)
void protector::get_active_policies(std::vector<int> & active){
  std::lock_guard<std::mutex> lck(*(m_access.get()));
  for (const auto &e : policy_list){
    active.push_back(e.first);
  }
}

// returns true if policy active else false
bool protector::is_active(int id){
  auto policy_it = policy_list.find(id);
  if (policy_it == policy_list.end()){
    return false;
  }
  else{
    return true;
  }
}

// clears counters for all policies 
void protector::clear()
{

  std::lock_guard<std::mutex> lck(*(m_access.get()));
  
  for(auto &e : policy_list){
    e.second._window_ref.get()->clear();
    e.second._counter = 0;
    e.second._req = 0;
    e.second._rej = 0;
  }

  net_requests = 0;
  net_rejects  = 0;
}


bool protector::selectiveBlock(double block_rate) 
{
  unsigned int num = (rand() % 100) + 1;    
  if (num > block_rate)  //not blocking
    return true;
  else                    //blocking
    return false;
}

