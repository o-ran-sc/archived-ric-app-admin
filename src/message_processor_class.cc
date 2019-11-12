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


#include <message_processor_class.hpp>


int verbose_flag = 0;

message_processor::message_processor(int mode, bool report_mode, size_t buffer_length, size_t reporting_interval):  _ref_sub_handler(NULL), _ref_protector(NULL), _ref_policy_handler(NULL), _num_messages(0), current_index (0), num_indications(0), num_err_indications(0){
  processing_level = mode;
  report_mode_only = report_mode;
  _buffer_size = buffer_length;
  scratch_buffer = 0;
  scratch_buffer = (unsigned char *)calloc(_buffer_size, sizeof(unsigned char));
  assert(scratch_buffer != 0);

};


void message_processor::register_subscription_handler(subscription_handler * sub){
  _ref_sub_handler = sub;
}


void message_processor::register_protector(protector * p){
  _ref_protector = p;
}

void message_processor::register_policy_handler(void (*f1)(int, const char *, int, std::string &, bool)){
  _ref_policy_handler = f1;
}

message_processor::~message_processor(void){
  free(scratch_buffer);
}

// main message processing 
bool message_processor::operator()(rmr_mbuf_t *message){

  bool res;
  bool send_msg = false;
  asn_dec_rval_t rval;
  size_t buf_size = _buffer_size;
  size_t mlen;
  _num_messages ++;
  std::string response;

  //FILE *pfile;
  //std::string filename = "/opt/out/e2ap_" + std::to_string(_num_messages) + ".per";

  state = NO_ERROR;
  mdclog_write(MDCLOG_DEBUG, "Received RMR message of type %d and size %d\n", message->mtype, message->len);
  
  // Sanity check on RMR. Max size ?
  if (message->len > MAX_RMR_RECV_SIZE){
    state = RMR_ERROR;
    mdclog_write(MDCLOG_ERR, "Error : %s, %d, RMR message larger than %d. Ignoring ...", __FILE__, __LINE__, MAX_RMR_RECV_SIZE);
    return false;
  }
  
  
  // main message processing code
  switch(message->mtype){
    
  case RIC_INDICATION:
    
    if (unlikely(_ref_protector == NULL)){
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d: No plugin registered to handle ric indication message\n", __FILE__, __LINE__);
      state = MISSING_HANDLER_ERROR;
      break;
    }
    
    //pfile = fopen(filename.c_str(), "wb");
    //fwrite(message->payload, sizeof(char), message->len, pfile);
    //fclose(pfile);

    e2ap_recv_pdu = 0;
    e2sm_header = 0;
    
    rval = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv_pdu), message->payload, message->len);
    
      
    if(likely(rval.code == RC_OK)){
      num_indications ++;
      res = indication_processor.get_fields(e2ap_recv_pdu->choice.initiatingMessage, indication_data);
      if (unlikely(!res)){
	state = E2AP_INDICATION_ERROR;
	num_err_indications ++;
	mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: Could not get fields from RICindication message\n", __FILE__, __LINE__);
	goto finished;
      }
      //std::cout <<"+++++++++++++++++++++++ E2AP Indication ++++++++++++++++++++++++" << std::endl;
      //xer_fprint(stdout, &asn_DEF_E2N_E2AP_PDU, e2ap_recv_pdu);
      //std::cout <<"+++++++++++++++++++++++ E2AP Indication ++++++++++++++++++++++++" << std::endl;
    }
    else{
      num_err_indications ++;
      state = E2AP_INDICATION_ERROR;
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d: Error decoding E2AP PDU\n", __FILE__, __LINE__);
      goto finished;
    }
    
    mdclog_write(MDCLOG_DEBUG, "E2AP INDICATION :: Successfully received E2AP Indication  with id = %ld, sequence no = %ld, Number of indications = %lu, Number of erroneous indications = %lu\n", indication_data.req_id, indication_data.req_seq_no, num_indications, num_err_indications);

    
    // do we progress ahead ?
    if(processing_level == E2AP_PROC_ONLY){
      goto finished;
    }
    
    //Decode the SM header
    mdclog_write(MDCLOG_DEBUG, "Decoding e2sm header of size %lu\n", indication_data.indication_header_size);
    rval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2SM_gNB_X2_indicationHeader, (void**)&(e2sm_header), indication_data.indication_header, indication_data.indication_header_size);
    
    if (likely(rval.code == RC_OK)){
      res = e2sm_indication_processor.get_header_fields(e2sm_header, header_helper);
      if (unlikely(!res)){
	state = E2SM_INDICATION_HEADER_ERROR;
	mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: Could not get fields from E2SM HEADER\n", __FILE__, __LINE__);
	goto finished;
      }
      
      // std::cout <<"+++++++++++++++++++++++ E2SM Indication Header ++++++++++++++++++++++++" << std::endl;
      // xer_fprint(stdout, &asn_DEF_E2SM_gNB_X2_indicationHeader, e2sm_header);
      // std::cout <<"+++++++++++++++++++++++ E2SM Indication Header ++++++++++++++++++++++++" << std::endl;	
    }
    else{
      state = E2SM_INDICATION_HEADER_ERROR;
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d: Error decoding E2SM Header.", __FILE__, __LINE__);
      goto finished;
    }
    mdclog_write(MDCLOG_DEBUG, "E2SM INDICATION HEADER :: Successfully decoded E2SM Indication Header of size %lu\n", indication_data.indication_header_size);
    
          
    // do we progress ahead ?
    if(processing_level == E2SM_PROC_ONLY){
      goto finished;
    }
    

    // NOTE : We assume RICindicationMessage contains payload (not E2SM message)
    // Send payload to plugin
    current_index = 0;
    remaining_buffer = _buffer_size;
    mdclog_write(MDCLOG_DEBUG, "Processing E2AP Indication message of size %lu\n", indication_data.indication_msg_size);
    
    res = (*_ref_protector)(indication_data.indication_msg, indication_data.indication_msg_size, scratch_buffer, &buf_size);
    if(unlikely(!res)){
      state = PLUGIN_ERROR;
      goto finished;
    }
     			    
    // Do we respond ? Depends on RIC indication type ...
    // if RICindicationType == report, then no response
    // else if RICindicationType == insert, generate control ...
    if (report_mode_only|| indication_data.indication_type == E2N_RICindicationType::E2N_RICindicationType_report){
      mdclog_write(MDCLOG_DEBUG, "Indication type is report. Not generating control\n");
      goto finished;
    }

    // we assume for now that just like in E2AP indication, response message is directly put in E2AP control 
    control_data.control_msg = &scratch_buffer[current_index];
    control_data.control_msg_size = buf_size; // re-use for size
    current_index += buf_size ;
    remaining_buffer = _buffer_size - current_index;
    mdclog_write(MDCLOG_DEBUG, "Encoded X2AP response of size %lu bytes. Remaining buffer = %lu bytes\n", buf_size, remaining_buffer);
    
    if (current_index >= _buffer_size){
      state = BUFFER_ERROR;
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d: Error buffer size %lu too small to encode further objects \n", __FILE__, __LINE__, _buffer_size);
      goto finished;
    }

      
    // Encode the control header
    // Control header is same as indication header ( except interface direction ?)
    header_helper.interface_direction = 0;
    res = e2sm_control_processor.encode_control_header(&scratch_buffer[current_index], &remaining_buffer, header_helper);
    
    if (likely(res)){
      control_data.control_header = &scratch_buffer[current_index];
      control_data.control_header_size = remaining_buffer;	
      current_index += remaining_buffer ;
      remaining_buffer = _buffer_size - current_index;
      
    }
    else{
      state = E2SM_CONTROL_HEADER_ERROR;
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d: Error encoding E2SM control header. Reason = %s\n", __FILE__, __LINE__, e2sm_control_processor.get_error().c_str());
      goto finished;
    }
    
    if (current_index >= _buffer_size){
      state = BUFFER_ERROR;
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d: Error buffer size %lu too small to encode further objects \n", __FILE__, __LINE__, _buffer_size);
      goto finished;
    }
    
    // Generate control message
    mlen = _buffer_size;
    control_data.req_id = indication_data.req_id;
    control_data.req_seq_no = indication_data.req_seq_no;
    control_data.func_id = indication_data.func_id;
    control_data.control_ack = 2; // no ack required       
    res = control_request_processor.encode_e2ap_control_request(message->payload, &mlen, control_data);
    
    if(likely(res)){
      send_msg = true;
      message->len = mlen;
      message->mtype = RIC_CONTROL_REQ;
      mdclog_write(MDCLOG_DEBUG,  "E2AP CONTROL MESSAGE :: Successfully generated E2AP Control Message\n");
    }
    else{
      state = E2AP_CONTROL_ERROR;
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d: Error encoding E2AP control . Reason = %s\n", __FILE__, __LINE__, control_request_processor.get_error().c_str());
      goto finished;
    }
    
    // Record id for tracking .... (tbd)
  finished:
    ASN_STRUCT_FREE(asn_DEF_E2N_E2SM_gNB_X2_indicationHeader, e2sm_header);
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv_pdu);
    
    break;
    
  case (RIC_SUB_RESP):
  case (RIC_SUB_DEL_RESP):
  case (RIC_SUB_FAILURE):
  case ( RIC_SUB_DEL_FAILURE ):
    if (_ref_sub_handler != NULL){
      mdclog_write(MDCLOG_INFO, "Received subscription message of type = %d", message->mtype);
      _ref_sub_handler->Response(message->mtype, message->payload, message->len);
    }
    else{
      state = MISSING_HANDLER_ERROR;
      mdclog_write(MDCLOG_ERR, " Error :: %s, %d : Subscription handler not assigned in message processor !", __FILE__, __LINE__);
    }
    
    break;
    
    
  case DC_ADM_INT_CONTROL:
    {
      if(_ref_policy_handler != NULL){
	// Need to apply config. Since config may need to be
	// applied across all threads, we do a callback to the parent thread.
	// wait for config to be applied and then send response
	_ref_policy_handler(DC_ADM_INT_CONTROL, (const char *) message->payload, message->len, response, true);
	std::memcpy( (char *) message->payload, response.c_str(),  response.length());
	message->len = response.length();
	message->mtype = DC_ADM_INT_CONTROL_ACK;
	send_msg = true;
      }
      else{
	state = MISSING_HANDLER_ERROR;
	mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: Policy handler not assigned in message processor !", __FILE__, __LINE__);
      }
    }
    break;

  case DC_ADM_GET_POLICY:
    {
      if(_ref_policy_handler != NULL){
	_ref_policy_handler(DC_ADM_GET_POLICY,  (const char *) message->payload, message->len, response, false);
	std::memcpy((char *)message->payload, response.c_str(), response.length());
	message->len = response.length();
	message->mtype = DC_ADM_GET_POLICY_ACK;
	send_msg = true;
      }
      else{
	state = MISSING_HANDLER_ERROR;
	mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: Policy handler not assigned in message processor !", __FILE__, __LINE__);
      }
    }
    break;
    
  default:
    mdclog_write(MDCLOG_ERR, "Error :: Unknown message type %d received from RMR", message->mtype);

  };

  if(send_msg){
    return true;
  }
  else{
    return false;
  }
};

   
unsigned long const message_processor::get_messages (void){
    return _num_messages;
};

