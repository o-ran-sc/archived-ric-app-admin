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
 * sgnb_addition_response.cc
 *
 *  Created on: Aug 22, 2019
 *      Author: sjana
 */

#include <sgnb_addition_response.hpp>

sgnb_addition_response::sgnb_addition_response(void ) {
  

  x2ap_pdu_obj = 0;
  x2ap_pdu_obj = (X2AP_PDU_t * )calloc(1, sizeof(X2AP_PDU_t));
  assert(x2ap_pdu_obj != 0);

  successMsg = 0;
  successMsg = (X2SuccessfulOutcome_t * )calloc(1, sizeof(X2SuccessfulOutcome_t));
  assert(successMsg != 0);

  unsuccessMsg = 0;
  unsuccessMsg = (X2UnsuccessfulOutcome_t * )calloc(1, sizeof(X2UnsuccessfulOutcome_t));
  assert(unsuccessMsg != 0);

  IE_array = 0;
  IE_array = (SgNBAdditionRequestAcknowledge_IEs_t *)calloc(NUM_SGNB_ADDITION_RESPONSE_ACKNOWLEDGE_IES, sizeof(SgNBAdditionRequestAcknowledge_IEs_t));
  assert(IE_array != 0);

  IE_reject_array = 0;
  IE_reject_array = (SgNBAdditionRequestReject_IEs_t *)calloc(NUM_SGNB_ADDITION_RESPONSE_FAILURE_IES, sizeof(SgNBAdditionRequestReject_IEs_t));
  assert(IE_reject_array != 0);
  
  
  erab_admit_array = 0;
  erab_admit_array = (E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_ItemIEs_t * ) calloc(INITIAL_SIZE, sizeof(E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t));
  assert(erab_admit_array != 0);
  erab_admit_array_size = INITIAL_SIZE;


  erab_sgnb_present_array = 0;
  erab_sgnb_present_array = (E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPpresent_t *) calloc(INITIAL_SIZE, sizeof(E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPpresent_t));
  assert(erab_sgnb_present_array != 0);
  erab_sgnb_present_array_size = INITIAL_SIZE;
  
  erab_sgnb_notpresent_array = 0;
  erab_sgnb_notpresent_array = (E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPnotpresent_t *) calloc(INITIAL_SIZE, sizeof(E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPnotpresent_t));
  assert(erab_sgnb_notpresent_array != 0);
  erab_sgnb_notpresent_array_size = INITIAL_SIZE;

  SgNBAdditionRequestAcknowledge_t * sgnb_ack = &(successMsg->value.choice.SgNBAdditionRequestAcknowledge);
  for(int i = 0; i < NUM_SGNB_ADDITION_RESPONSE_ACKNOWLEDGE_IES; i++){
    ASN_SEQUENCE_ADD(&(sgnb_ack->protocolIEs), &(IE_array[i]));
  }
  
  
  SgNBAdditionRequestReject_t * sgnb_reject = &(unsuccessMsg->value.choice.SgNBAdditionRequestReject);
  for(int i = 0; i < NUM_SGNB_ADDITION_RESPONSE_FAILURE_IES; i++){
    ASN_SEQUENCE_ADD(&(sgnb_reject->protocolIEs), &(IE_reject_array[i]));
  }

}

sgnb_addition_response::~sgnb_addition_response(void){

  mdclog_write(MDCLOG_INFO, "Freeing X2AP SgNB Addition Response object memory");

  E_RABs_Admitted_ToBeAdded_SgNBAddReqAckList_t  * erabs_toadd_list = &(IE_array[3].value.choice.E_RABs_Admitted_ToBeAdded_SgNBAddReqAckList);
  for(int i = 0; i < erabs_toadd_list->list.size; i++){
    erabs_toadd_list->list.array[i] = 0;
  }
  if (erabs_toadd_list->list.size > 0){
    free(erabs_toadd_list->list.array);
    erabs_toadd_list->list.array = 0;
    erabs_toadd_list->list.size = 0;
    erabs_toadd_list->list.count= 0;
  }
  
  free(erab_sgnb_present_array);
  free(erab_sgnb_notpresent_array);
  free(erab_admit_array);

  SgNBAdditionRequestAcknowledge_t *sgnb_ack  = &(successMsg->value.choice.SgNBAdditionRequestAcknowledge);
  for(int i = 0; i < sgnb_ack->protocolIEs.list.size; i++){
    sgnb_ack->protocolIEs.list.array[i] = 0;
  }

  if (sgnb_ack->protocolIEs.list.size > 0){
    free(sgnb_ack->protocolIEs.list.array);
    sgnb_ack->protocolIEs.list.array = 0;
    sgnb_ack->protocolIEs.list.size = 0;
  }
  
  SgNBAdditionRequestReject_t *sgnb_reject  = &(unsuccessMsg->value.choice.SgNBAdditionRequestReject);
   for(int i = 0; i < sgnb_reject->protocolIEs.list.size; i++){
     sgnb_reject->protocolIEs.list.array[i] = 0;
   }

   if (sgnb_reject->protocolIEs.list.size > 0){
     free(sgnb_reject->protocolIEs.list.array);
     sgnb_reject->protocolIEs.list.array = 0;
     sgnb_reject->protocolIEs.list.size = 0;
   }


  free(IE_array);
  free(IE_reject_array);
  free(successMsg);
  free(unsuccessMsg);

  x2ap_pdu_obj->choice.initiatingMessage = NULL;
  x2ap_pdu_obj->present = X2AP_PDU_PR_NOTHING;
  
  ASN_STRUCT_FREE(asn_DEF_X2AP_PDU, x2ap_pdu_obj);

  mdclog_write(MDCLOG_INFO, "Freed X2AP SgNB Addition Response object memory");
}

bool sgnb_addition_response::set_fields(X2UnsuccessfulOutcome_t * unsuccessMsg, sgnb_addition_helper &sgnb_data){
  
  unsigned ie_index = 0;

  if (unsuccessMsg == 0){
    error_string = "Invalid reference for X2AP Sgnb Addition Reject message in set_fields";
    return false;
  }
  SgNBAdditionRequestReject_IEs_t *ies_sgnb_add_reject;

  ie_index = 0;
  ies_sgnb_add_reject= &IE_reject_array[ie_index];
  ies_sgnb_add_reject->criticality = Criticality_reject;
  ies_sgnb_add_reject->id = ProtocolIE_ID_id_MeNB_UE_X2AP_ID;
  ies_sgnb_add_reject->value.present = SgNBAdditionRequestReject_IEs__value_PR_UE_X2AP_ID;
  ies_sgnb_add_reject->value.choice.UE_X2AP_ID = sgnb_data.menb_ue_x2ap_id;


  ie_index = 1;
  ies_sgnb_add_reject= &IE_reject_array[ie_index];
  ies_sgnb_add_reject->criticality = Criticality_reject;
  ies_sgnb_add_reject->id = ProtocolIE_ID_id_SgNB_UE_X2AP_ID;
  ies_sgnb_add_reject->value.present = SgNBAdditionRequestReject_IEs__value_PR_SgNB_UE_X2AP_ID;
  ies_sgnb_add_reject->value.choice.SgNB_UE_X2AP_ID = sgnb_data.sgnb_ue_x2ap_id;


  ie_index = 2;
  ies_sgnb_add_reject= &IE_reject_array[ie_index];
  ies_sgnb_add_reject->criticality = Criticality_reject;
  ies_sgnb_add_reject->id = ProtocolIE_ID_id_Cause;
  ies_sgnb_add_reject->value.present = SgNBAdditionRequestReject_IEs__value_PR_Cause;
  Cause_t *caus = &ies_sgnb_add_reject->value.choice.Cause;

  switch(sgnb_data.cause)
    {
    case Cause_PR_misc: //Misc
      caus->present = Cause_PR_misc;
      caus->choice.misc = sgnb_data.cause_desc;
      break;
    case Cause_PR_radioNetwork: //radio network
      caus->present = Cause_PR_radioNetwork;
      caus->choice.radioNetwork = sgnb_data.cause_desc;
      break;
    case Cause_PR_protocol : //protocol
      caus->present = Cause_PR_protocol;
      caus->choice.protocol = sgnb_data.cause_desc;
      break;
    case Cause_PR_transport: //transport
      caus->present = Cause_PR_transport;
      caus->choice.transport = sgnb_data.cause_desc;
      break;
      
    default:
      caus->present = Cause_PR_NOTHING;
      break;
    }

  return true;

}


bool sgnb_addition_response::set_fields(X2SuccessfulOutcome_t * successMsg, sgnb_addition_helper &sgnb_data){

  unsigned ie_index = 0;

  if (successMsg == 0){
    error_string = "Invalid reference for X2AP Sgnb Addition Acknowledge message in set_fields";
    return false;
  }

  SgNBAdditionRequestAcknowledge_IEs_t *ies_sgnb_add_ack;

  ie_index = 0;
  ies_sgnb_add_ack= &IE_array[ie_index];
  ies_sgnb_add_ack->criticality = Criticality_reject;
  ies_sgnb_add_ack->id = ProtocolIE_ID_id_MeNB_UE_X2AP_ID;
  ies_sgnb_add_ack->value.present = SgNBAdditionRequestAcknowledge_IEs__value_PR_UE_X2AP_ID;
  ies_sgnb_add_ack->value.choice.UE_X2AP_ID = sgnb_data.menb_ue_x2ap_id;

  
  ie_index = 1;
  ies_sgnb_add_ack= &IE_array[ie_index];
  ies_sgnb_add_ack->criticality = Criticality_reject;
  ies_sgnb_add_ack->id = ProtocolIE_ID_id_SgNB_UE_X2AP_ID;
  ies_sgnb_add_ack->value.present = SgNBAdditionRequestAcknowledge_IEs__value_PR_SgNB_UE_X2AP_ID;
  ies_sgnb_add_ack->value.choice.SgNB_UE_X2AP_ID = sgnb_data.sgnb_ue_x2ap_id;

  ie_index = 2;
  ies_sgnb_add_ack= &IE_array[ie_index];
  ies_sgnb_add_ack->criticality = Criticality_reject;
  ies_sgnb_add_ack->id = ProtocolIE_ID_id_SgNBtoMeNBContainer;
  ies_sgnb_add_ack->value.present = SgNBAdditionRequestAcknowledge_IEs__value_PR_SgNBtoMeNBContainer;
  SgNBtoMeNBContainer_t *container = &ies_sgnb_add_ack->value.choice.SgNBtoMeNBContainer;
  container->buf = sgnb_data.menb_sgnb_container;
  container->size = sgnb_data.menb_sgnb_container_size;

  ie_index = 3;
  ies_sgnb_add_ack = &IE_array[ie_index];
  ies_sgnb_add_ack->criticality = Criticality_reject;
  ies_sgnb_add_ack->id =  ProtocolIE_ID_id_E_RABs_Admitted_ToBeAdded_SgNBAddReqAckList;
  ies_sgnb_add_ack->value.present = SgNBAdditionRequestAcknowledge_IEs__value_PR_E_RABs_Admitted_ToBeAdded_SgNBAddReqAckList;
  E_RABs_Admitted_ToBeAdded_SgNBAddReqAckList_t  * erabs_toadd_list = &(ies_sgnb_add_ack->value.choice.E_RABs_Admitted_ToBeAdded_SgNBAddReqAckList);


  int lcount = sgnb_data.erab_list.size();
  //std::cout <<"Adding " << lcount << " erabs in response" << std::endl;
  
  //resize empty array in constructor
  if (lcount >= erab_admit_array_size){
    erab_admit_array_size = 2*lcount;

    E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_ItemIEs_t * new_ref = 0;
    new_ref = (E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_ItemIEs_t *)realloc(erab_admit_array, erab_admit_array_size * sizeof(E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_t));
    assert(new_ref != 0);
    erab_admit_array = new_ref;

    erab_sgnb_present_array_size = 2 * lcount;
    E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPpresent_t  * new_present = 0;
    new_present = (E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPpresent_t  *)realloc(erab_sgnb_present_array, erab_sgnb_present_array_size * sizeof(E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPpresent_t ));
    assert(new_present != 0);
    erab_sgnb_present_array = new_present;

    erab_sgnb_notpresent_array_size = 2 * lcount;
    E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPnotpresent_t  * new_notpresent = 0;
    new_notpresent = (E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPnotpresent_t  *)realloc(erab_sgnb_notpresent_array, erab_sgnb_notpresent_array_size * sizeof(E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPnotpresent_t ));
    assert(new_notpresent != 0);
    erab_sgnb_notpresent_array = new_notpresent;
  }

  std::vector<struct erab_item> * ref_erab_input  = &(sgnb_data.erab_list);
  erabs_toadd_list->list.count = 0;

  int sgnb_present_index = 0;
  int sgnb_notpresent_index = 0;

  for(unsigned int i = 0; i < lcount; i++){

    erab_admit_array[i].id = ProtocolIE_ID_id_E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item ;
    erab_admit_array[i].criticality = Criticality_ignore;
    erab_admit_array[i].value.present =  E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_ItemIEs__value_PR_E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item;

    E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_t * erab_item = &(erab_admit_array[i].value.choice.E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item);

    erab_item->e_RAB_ID = (*ref_erab_input)[i].erab_id;
    erab_item->en_DC_ResourceConfiguration.pDCPatSgNB =  (*ref_erab_input)[i].pdcp_at_sgnb;
    erab_item->en_DC_ResourceConfiguration.mCGresources =  (*ref_erab_input)[i].mcg_resources;
    erab_item->en_DC_ResourceConfiguration.sCGresources =  (*ref_erab_input)[i].scg_resources;


    erab_item->resource_configuration.present = (E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item__resource_configuration_PR)(*ref_erab_input)[i].sgnb_pdcp_present;
    if( erab_item->resource_configuration.present == E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item__resource_configuration_PR_sgNBPDCPpresent){
      
      erab_sgnb_present_array[sgnb_present_index].s1_DL_GTPtunnelEndpoint.transportLayerAddress.buf = (*ref_erab_input)[i].sgnb_item.transport_layer_addr;
      erab_sgnb_present_array[sgnb_present_index].s1_DL_GTPtunnelEndpoint.transportLayerAddress.size = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_size;
      erab_sgnb_present_array[sgnb_present_index].s1_DL_GTPtunnelEndpoint.transportLayerAddress.bits_unused  = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_unused;

      erab_sgnb_present_array[sgnb_present_index].s1_DL_GTPtunnelEndpoint.gTP_TEID.buf = (*ref_erab_input)[i].sgnb_item.gtp_tei;
      erab_sgnb_present_array[sgnb_present_index].s1_DL_GTPtunnelEndpoint.gTP_TEID.size = (*ref_erab_input)[i].sgnb_item.gtp_tei_size;

      /* Force all optional parameters that are not set to NULL */
      erab_sgnb_present_array[sgnb_present_index].sgNB_UL_GTP_TEIDatPDCP = NULL;

      /* validate constraints ..*/
      int ret_constr;
      ret_constr = asn_check_constraints(&asn_DEF_GTP_TEI, &erab_sgnb_present_array[sgnb_present_index].s1_DL_GTPtunnelEndpoint.gTP_TEID, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for GTP TEID.. Reason = " + error_string;
	return false;
      }

      ret_constr = asn_check_constraints(&asn_DEF_GTPtunnelEndpoint, &erab_sgnb_present_array[sgnb_present_index].s1_DL_GTPtunnelEndpoint, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for GTP tunnel endpoint .. Reason = " + error_string;
	return false;
      }

      ret_constr = asn_check_constraints(&asn_DEF_E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPpresent, &erab_sgnb_present_array[sgnb_present_index], errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for E_RABS_ToBeAdded_SgNBAddReq_SgNBPDCPpresent.. Reason = " + error_string;
	return false;
      }

      erab_item->resource_configuration.choice.sgNBPDCPpresent = &erab_sgnb_present_array[sgnb_present_index];
      sgnb_present_index ++;

    }
    else if (erab_item->resource_configuration.present ==  E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item__resource_configuration_PR_sgNBPDCPnotpresent){

      erab_sgnb_notpresent_array[sgnb_notpresent_index].sgNB_DL_GTP_TEIDatSCG.transportLayerAddress.buf = (*ref_erab_input)[i].sgnb_item.transport_layer_addr;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].sgNB_DL_GTP_TEIDatSCG.transportLayerAddress.size = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_size;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].sgNB_DL_GTP_TEIDatSCG.transportLayerAddress.bits_unused  = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_unused;

      erab_sgnb_notpresent_array[sgnb_notpresent_index].sgNB_DL_GTP_TEIDatSCG.gTP_TEID.buf = (unsigned char *)(*ref_erab_input)[i].sgnb_item.gtp_tei;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].sgNB_DL_GTP_TEIDatSCG.gTP_TEID.size = (*ref_erab_input)[i].sgnb_item.gtp_tei_size;

      erab_sgnb_notpresent_array[sgnb_notpresent_index].secondary_sgNB_DL_GTP_TEIDatSCG = NULL;

      /* validate constraints ..*/
      int ret_constr;
      ret_constr = asn_check_constraints(&asn_DEF_GTPtunnelEndpoint, &erab_sgnb_notpresent_array[sgnb_notpresent_index].sgNB_DL_GTP_TEIDatSCG, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for GTP tunnel endpoint .. Reason = " + error_string;
	return false;
      }


      ret_constr = asn_check_constraints(&asn_DEF_E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_Item_SgNBPDCPnotpresent, &erab_sgnb_notpresent_array[sgnb_notpresent_index], errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for E_RABS_ToBeAdded_SgNBAddReq_SgNBPDCPnotpresent.. Reason = " + error_string;
	return false;
      }

      erab_item->resource_configuration.choice.sgNBPDCPnotpresent = &erab_sgnb_notpresent_array[sgnb_notpresent_index]; ;
      sgnb_notpresent_index ++;
    }
    else {
      std::cerr <<"Could not find a pdcp present or not present flag. not adding erab item .." << std::endl;
      continue;
    }

    // check constraint
    int ret_constr = asn_check_constraints(&asn_DEF_E_RABs_Admitted_ToBeAdded_SgNBAddReqAck_ItemIEs, &erab_admit_array[i], errbuf, &errbuf_len);
    if(ret_constr){
      error_string.assign(&errbuf[0], errbuf_len);
      error_string = "Error in constraints for E_RABS_Admitted_ToBeAdded_SgNBAddReqAck_ItemIEs.. Reason = " + error_string;
      return false;
    }

    ASN_SEQUENCE_ADD(erabs_toadd_list, &erab_admit_array[i]);
  }

  return true;
}

bool sgnb_addition_response::encode_sgnb_addition_response(unsigned char* data_buf, size_t *data_size, sgnb_addition_helper &sgnb_data, bool sgnb_response)
{

  bool res;
  if(sgnb_response == true){

    x2ap_pdu_obj->present = X2AP_PDU_PR_successfulOutcome;
    x2ap_pdu_obj->choice.successfulOutcome = successMsg;
    
    successMsg->procedureCode = ProcedureCode_id_sgNBAdditionPreparation;
    successMsg->criticality = Criticality_ignore;
    successMsg->value.present = X2SuccessfulOutcome__value_PR_SgNBAdditionRequestAcknowledge;

    res = set_fields(successMsg, sgnb_data);

  } else {

    x2ap_pdu_obj->present = X2AP_PDU_PR_unsuccessfulOutcome;
    x2ap_pdu_obj->choice.unsuccessfulOutcome = unsuccessMsg;


    unsuccessMsg->procedureCode = ProcedureCode_id_sgNBAdditionPreparation;
    unsuccessMsg->criticality = Criticality_ignore;
    unsuccessMsg->value.present = X2UnsuccessfulOutcome__value_PR_SgNBAdditionRequestReject;

    res = set_fields(unsuccessMsg, sgnb_data);
  }

  if (!res){
    return false;
  }
  int ret_constr = asn_check_constraints(&asn_DEF_X2AP_PDU, x2ap_pdu_obj, errbuf, &errbuf_len);
  if(ret_constr){
    error_string.assign(&errbuf[0], errbuf_len);
    error_string = "Error encoding X2AP Sgnb Addition Response  message. Reason = " + error_string;
    return false;
  }

  //xer_fprint(stdout, &asn_DEF_X2AP_PDU, x2ap_pdu_obj);

  asn_enc_rval_t retval = asn_encode_to_buffer(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_X2AP_PDU, x2ap_pdu_obj, data_buf, *data_size);
  if(retval.encoded == -1){
    error_string.assign(strerror(errno));
    return false;
  }

  else {
    if(*data_size < retval.encoded){
      std::stringstream ss;
      ss  <<"Error encoding SgNB Addition Request Response. Reason =  encoded pdu size " << retval.encoded << " exceeds buffer size " << *data_size << std::endl;
      error_string = ss.str();
      return false;
    }
  }

  *data_size = retval.encoded;
  return true;
}


