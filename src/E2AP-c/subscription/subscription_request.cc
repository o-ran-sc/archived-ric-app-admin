

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


#include "subscription_request.hpp"
  
subscription_request::subscription_request(void){

  _name = "default";

  e2ap_pdu_obj = 0;
  e2ap_pdu_obj = (E2AP_PDU_t * )calloc(1, sizeof(E2AP_PDU_t));
  assert(e2ap_pdu_obj != 0);

  initMsg = 0;
  initMsg = (InitiatingMessage_t * )calloc(1, sizeof(InitiatingMessage_t));
  assert(initMsg != 0);

  IE_array = 0;
  IE_array = (RICsubscriptionRequest_IEs_t *)calloc(NUM_SUBSCRIPTION_REQUEST_IES, sizeof(RICsubscriptionRequest_IEs_t));
  assert(IE_array != 0);
  
  action_array = 0;
  action_array = (RICaction_ToBeSetup_ItemIEs_t *)calloc(INITIAL_LIST_SIZE, sizeof(RICaction_ToBeSetup_ItemIEs_t));
  assert(action_array != 0);
  action_array_size = INITIAL_LIST_SIZE;
  
};



// Clear assigned protocolIE list from RIC indication IE container
subscription_request::~subscription_request(void){
    
  mdclog_write(MDCLOG_INFO, "Freeing subscription request memory for");;
  
  // Sequence of actions to be admitted causes special heart-ache. Free ric subscription element manually and reset the ie pointer  
  RICsubscription_t * ricsubscription_ie = &(IE_array[2].value.choice.RICsubscription);

  for(int i = 0; i < ricsubscription_ie->ricAction_ToBeSetup_List.list.size; i++){
    ricsubscription_ie->ricAction_ToBeSetup_List.list.array[i] = 0;
  }

  if (ricsubscription_ie->ricAction_ToBeSetup_List.list.size > 0){
    free(ricsubscription_ie->ricAction_ToBeSetup_List.list.array);
    ricsubscription_ie->ricAction_ToBeSetup_List.list.size = 0;
    ricsubscription_ie->ricAction_ToBeSetup_List.list.count = 0;
    ricsubscription_ie->ricAction_ToBeSetup_List.list.array = 0;
  }
  
  free(action_array);
  RICsubscriptionRequest_t * subscription_request = &(initMsg->value.choice.RICsubscriptionRequest);
  
  for(int i = 0; i < subscription_request->protocolIEs.list.size; i++){
    subscription_request->protocolIEs.list.array[i] = 0;
  }
  
  if( subscription_request->protocolIEs.list.size > 0){
    free( subscription_request->protocolIEs.list.array);
    subscription_request->protocolIEs.list.array = 0;
    subscription_request->protocolIEs.list.size = 0;
    subscription_request->protocolIEs.list.count = 0;
  }
  
  free(IE_array);
  free(initMsg);
  e2ap_pdu_obj->choice.initiatingMessage = 0;
  
  ASN_STRUCT_FREE(asn_DEF_E2AP_PDU, e2ap_pdu_obj);
  mdclog_write(MDCLOG_INFO, "Freed subscription request memory ");
};


bool subscription_request::encode_e2ap_subscription(unsigned char *buf, size_t *size, E2AP_PDU_t *e2ap_pdu, subscription_helper &dinput){

  bool res;
  
  e2ap_pdu_obj->choice.initiatingMessage = initMsg;
  e2ap_pdu_obj->present = E2AP_PDU_PR_initiatingMessage;
  
  res = set_fields(e2ap_pdu_obj->choice.initiatingMessage, dinput);
  if (!res){
    return false;
  }
  
  //xer_fprint(stdout, &asn_DEF_E2AP_PDU, e2ap_pdu_obj);
  int ret_constr = asn_check_constraints(&asn_DEF_E2AP_PDU, (void *) e2ap_pdu_obj, errbuf, &errbuf_len);
  if(ret_constr){
    error_string.assign(errbuf, errbuf_len);
    error_string = "Constraints failed for encoding subscription request. Reason = " + error_string;
    return false;
  }
  
  asn_enc_rval_t retval = asn_encode_to_buffer(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2AP_PDU, e2ap_pdu_obj, buf, *size);
    
  if(retval.encoded == -1){
    error_string.assign(strerror(errno));
    error_string = "Error encoding Subscription Delete Request. Reason = " + error_string;
    return false;
  }
  else {
    if(*size < retval.encoded){
      std::stringstream ss;
      ss  <<"Error encoding Subscription Delete Request . Reason =  encoded pdu size " << retval.encoded << " exceeds buffer size " << *size << std::endl;
      error_string = ss.str();
      retval.encoded = -1;
      return false;
    }
  }
    
  *size = retval.encoded;
  return true;
    
}


bool subscription_request::set_fields(InitiatingMessage_t *ref_initMsg, subscription_helper &helper){

  if (ref_initMsg == 0){
    error_string = "Error : Invalid reference when setting fields for subscription request";
    return false;
  }
  
  int ie_index;
  int result = 0;
  
  ref_initMsg->procedureCode = ProcedureCode_id_ricSubscription;
  ref_initMsg->criticality = Criticality_reject;
  ref_initMsg->value.present = InitiatingMessage__value_PR_RICsubscriptionRequest;

  RICsubscriptionRequest_t * subscription_request = &(ref_initMsg->value.choice.RICsubscriptionRequest);

  // Reset field count for protocolIEs
  subscription_request->protocolIEs.list.count = 0;
  
  ie_index = 0;
  RICsubscriptionRequest_IEs_t *ies_ricreq = &IE_array[ie_index];
  ies_ricreq->criticality = Criticality_reject;
  ies_ricreq->id = ProtocolIE_ID_id_RICrequestID;
  ies_ricreq->value.present = RICsubscriptionRequest_IEs__value_PR_RICrequestID;
  RICrequestID_t *ricrequest_ie = &ies_ricreq->value.choice.RICrequestID;
  ricrequest_ie->ricRequestorID = helper.get_request_id();
  ricrequest_ie->ricRequestSequenceNumber = helper.get_req_seq();
  result = ASN_SEQUENCE_ADD(&subscription_request->protocolIEs, ies_ricreq);
  if (result == -1){
     error_string = "Erorr : Unable to assign memory to add RIC request id to subscription request";
    return false;
  }
     
  ie_index = 1;
  RICsubscriptionRequest_IEs_t *ies_ranfunc = &IE_array[ie_index];
  ies_ranfunc->criticality = Criticality_reject;
  ies_ranfunc->id = ProtocolIE_ID_id_RANfunctionID;
  ies_ranfunc->value.present = RICsubscriptionRequest_IEs__value_PR_RANfunctionID;
  RANfunctionID_t *ranfunction_ie = &ies_ranfunc->value.choice.RANfunctionID;
  *ranfunction_ie = helper.get_function_id();
  ASN_SEQUENCE_ADD(&subscription_request->protocolIEs, ies_ranfunc);
  if (result == -1){
    error_string = "Erorr : Unable to assign memory to add RAN function id to subscription request";
    return false;
  }


  ie_index = 2;
  RICsubscriptionRequest_IEs_t *ies_actid = &IE_array[ie_index];
  ies_actid->criticality = Criticality_reject;
  ies_actid->id = ProtocolIE_ID_id_RICsubscription;
  ies_actid->value.present = RICsubscriptionRequest_IEs__value_PR_RICsubscription;
  RICsubscription_t *ricsubscription_ie = &ies_actid->value.choice.RICsubscription;

  ricsubscription_ie->ricEventTriggerDefinition.buf = (uint8_t *) helper.get_event_def();
  ricsubscription_ie->ricEventTriggerDefinition.size = helper.get_event_def_size();
   
  std::vector<Action> * ref_action_array = helper.get_list();
  
  // do we need to reallocate ?
  if(action_array_size < ref_action_array->size()){
    action_array_size = 2 * ref_action_array->size();
    RICaction_ToBeSetup_ItemIEs_t * new_ref = 0;
    new_ref = (RICaction_ToBeSetup_ItemIEs_t *  )realloc(action_array, action_array_size);
    assert(new_ref != 0);
    action_array = new_ref;
  }
  
  // reset the list count on ricAction_ToBeSetup_List;
  ricsubscription_ie->ricAction_ToBeSetup_List.list.count = 0;
    
  for(int i = 0; i < ref_action_array->size(); i ++){
    action_array[i].criticality = Criticality_ignore;
    action_array[i].id = ProtocolIE_ID_id_RICaction_ToBeSetup_Item ;
    action_array[i].value.present = RICaction_ToBeSetup_ItemIEs__value_PR_RICaction_ToBeSetup_Item;
    action_array[i].value.choice.RICaction_ToBeSetup_Item.ricActionID = (*ref_action_array)[i].get_id();
    action_array[i].value.choice.RICaction_ToBeSetup_Item.ricActionType = (*ref_action_array)[i].get_type();

    result = ASN_SEQUENCE_ADD(&ricsubscription_ie->ricAction_ToBeSetup_List, &(action_array[i]));
    if (result == -1){
      error_string = "Erorr : Unable to assign memory to add Action item to set up list";
      return false;
    }
    

  }
  
  result = ASN_SEQUENCE_ADD(&subscription_request->protocolIEs, ies_actid);
  if (result == -1){
    error_string = "Erorr : Unable to assign memory to action item to be setup list  subscription request";
    return false;
  }

  return true;
};



bool subscription_request:: get_fields(InitiatingMessage_t * init_msg,  subscription_helper & dout)
{

  if (init_msg == 0){
    error_string = "Error. Invalid reference when getting fields from subscription request";
    return false;
  }
  
  RICrequestID_t *requestid;
  RANfunctionID_t * ranfunctionid;
  RICsubscription_t * ricsubscription;
    
  for(int edx = 0; edx < init_msg->value.choice.RICsubscriptionRequest.protocolIEs.list.count; edx++) {
    RICsubscriptionRequest_IEs_t *memb_ptr = init_msg->value.choice.RICsubscriptionRequest.protocolIEs.list.array[edx];
    
    switch(memb_ptr->id)
      {
      case (ProtocolIE_ID_id_RICrequestID):
	requestid = &memb_ptr->value.choice.RICrequestID;
	dout.set_request(requestid->ricRequestorID, requestid->ricRequestSequenceNumber);
	break;
	  
      case (ProtocolIE_ID_id_RANfunctionID):
	ranfunctionid = &memb_ptr->value.choice.RANfunctionID;
	dout.set_function_id(*ranfunctionid);
	break;
	  
      case (ProtocolIE_ID_id_RICsubscription):
	ricsubscription = &memb_ptr->value.choice.RICsubscription;
	dout.set_event_def(ricsubscription->ricEventTriggerDefinition.buf, ricsubscription->ricEventTriggerDefinition.size);
	  
	for(int index = 0; index < ricsubscription->ricAction_ToBeSetup_List.list.count; index ++){
	  RICaction_ToBeSetup_ItemIEs_t * item = (RICaction_ToBeSetup_ItemIEs_t *)ricsubscription->ricAction_ToBeSetup_List.list.array[index];
	  dout.add_action(item->value.choice.RICaction_ToBeSetup_Item.ricActionID, item->value.choice.RICaction_ToBeSetup_Item.ricActionType);
	};
	  
	break;
      }
      
  }
    
  //asn_fprint(stdout, &asn_DEF_E2AP_PDU, e2pdu);
  return true;
};



