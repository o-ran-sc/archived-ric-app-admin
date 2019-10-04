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

#include <sgnb_addition_request.hpp>

  
sgnb_addition_request::sgnb_addition_request(void){
  x2ap_pdu_obj = 0;
  x2ap_pdu_obj = (X2AP_PDU_t * )calloc(1, sizeof(X2AP_PDU_t));
  assert(x2ap_pdu_obj != 0);

  initMsg = 0;
  initMsg = (X2InitiatingMessage_t * )calloc(1, sizeof(X2InitiatingMessage_t));
  assert(initMsg != 0);

  IE_array = 0;
  IE_array = (SgNBAdditionRequest_IEs_t *)calloc(NUM_SGNB_ADDITION_REQUEST_IES, sizeof(SgNBAdditionRequest_IEs_t));
  assert(IE_array != 0);

  erab_array = 0;
  erab_array = (E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t * ) calloc(INITIAL_SIZE, sizeof(E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t));
  assert(erab_array != 0);
  erab_array_size = INITIAL_SIZE;

  erab_sgnb_present_array = 0;
  erab_sgnb_present_array = (E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent_t *) calloc(INITIAL_SIZE, sizeof(E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent_t));
  assert(erab_sgnb_present_array != 0);
  erab_sgnb_present_array_size = INITIAL_SIZE;
  
  erab_sgnb_notpresent_array = 0;
  erab_sgnb_notpresent_array = (E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent_t *) calloc(INITIAL_SIZE, sizeof(E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent_t));
  assert(erab_sgnb_notpresent_array != 0);
  erab_sgnb_notpresent_array_size = INITIAL_SIZE;
  
  x2ap_pdu_obj->present = X2AP_PDU_PR_initiatingMessage;
  x2ap_pdu_obj->choice.initiatingMessage = initMsg;

  SgNBAdditionRequest_t * ric_indication = &(initMsg->value.choice.SgNBAdditionRequest);
  for(int i = 0; i < NUM_SGNB_ADDITION_REQUEST_IES; i++){
    ASN_SEQUENCE_ADD(&(ric_indication->protocolIEs), &(IE_array[i]));
  }

  
};


sgnb_addition_request::~sgnb_addition_request(void){

  mdclog_write(MDCLOG_INFO, "Freeing X2AP SgNB Addition Request object memory");

  E_RABs_ToBeAdded_SgNBAddReqList_t  * erabs_toadd_list = &(IE_array[7].value.choice. E_RABs_ToBeAdded_SgNBAddReqList);
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
  
  free(erab_array);

  // Free BitRate allocation if any
  SgNBAdditionRequest_IEs_t *ies_sgnb_addition_req;
  ies_sgnb_addition_req= &IE_array[3];
  UEAggregateMaximumBitRate_t * ue_rates = &(ies_sgnb_addition_req->value.choice.UEAggregateMaximumBitRate);

  // clear buffer if previously allocated
  if (ue_rates->uEaggregateMaximumBitRateDownlink.size > 0){
    ue_rates->uEaggregateMaximumBitRateDownlink.size = 0;
    free(ue_rates->uEaggregateMaximumBitRateDownlink.buf);
  }

  
  if (ue_rates->uEaggregateMaximumBitRateUplink.size > 0){
    ue_rates->uEaggregateMaximumBitRateUplink.size = 0;
    free(ue_rates->uEaggregateMaximumBitRateUplink.buf);
  }
  
  SgNBAdditionRequest_t *sgnb_addition  = &(initMsg->value.choice.SgNBAdditionRequest);
  for(int i = 0; i < sgnb_addition->protocolIEs.list.size; i++){
    sgnb_addition->protocolIEs.list.array[i] = 0;
  }


  if (sgnb_addition->protocolIEs.list.size > 0){
    free(sgnb_addition->protocolIEs.list.array);
    sgnb_addition->protocolIEs.list.array = 0;
    sgnb_addition->protocolIEs.list.size = 0;
  }
  free(IE_array);

  free(initMsg);
  
  x2ap_pdu_obj->choice.initiatingMessage = 0;
  ASN_STRUCT_FREE(asn_DEF_X2AP_PDU, x2ap_pdu_obj);
  mdclog_write(MDCLOG_INFO, "Freed X2AP SgNB Addition Request object mempory");
}



bool sgnb_addition_request::encode_sgnb_addition_request(unsigned char *buf, size_t *size, sgnb_addition_helper & dinput){

  initMsg->procedureCode = ProcedureCode_id_sgNBAdditionPreparation;
  initMsg->criticality = Criticality_ignore;
  initMsg->value.present = X2InitiatingMessage__value_PR_SgNBAdditionRequest;

  bool res;
  res = set_fields(initMsg, dinput);
  if (!res){
    return false;
  }
  
  int ret_constr = asn_check_constraints(&asn_DEF_X2AP_PDU, x2ap_pdu_obj, errbuf, &errbuf_len);
  if(ret_constr){
    error_string.assign(&errbuf[0], errbuf_len);
    error_string = "Error encoding X2AP Sgnb Addition Request  message. Reason = " + error_string;
    return false;
  }

  //xer_fprint(stdout, &asn_DEF_X2AP_PDU, x2ap_pdu_obj);
  
  asn_enc_rval_t retval = asn_encode_to_buffer(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_X2AP_PDU, x2ap_pdu_obj, buf, *size);
  if(retval.encoded == -1){
    error_string.assign(strerror(errno));
    return false;
  }

  else {
    if(*size < retval.encoded){
      std::stringstream ss;
      ss  <<"Error encoding SgNB Addition Request definition. Reason =  encoded pdu size " << retval.encoded << " exceeds buffer size " << *size << std::endl;
      error_string = ss.str();
      return false;
    }
  }

  *size = retval.encoded;
  return true;

}


bool sgnb_addition_request::set_fields(X2InitiatingMessage_t * init_msg, sgnb_addition_helper &dinput){

  unsigned ie_index = 0;
  
  if (initMsg == 0){
    error_string = "Invalid reference for X2AP Sgnb Addition Request message in set_fields";
    return false;
  }

  
  SgNBAdditionRequest_IEs_t *ies_sgnb_addition_req;

  ie_index = 0;
  ies_sgnb_addition_req= &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id = ProtocolIE_ID_id_MeNB_UE_X2AP_ID;
  ies_sgnb_addition_req->value.present = SgNBAdditionRequest_IEs__value_PR_UE_X2AP_ID;
  ies_sgnb_addition_req->value.choice.UE_X2AP_ID = dinput.menb_ue_x2ap_id;
  

  ie_index = 1;
  ies_sgnb_addition_req= &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id = ProtocolIE_ID_id_NRUESecurityCapabilities;
  ies_sgnb_addition_req->value.present = SgNBAdditionRequest_IEs__value_PR_NRUESecurityCapabilities;
  NRUESecurityCapabilities_t * nrue_sec = &(ies_sgnb_addition_req->value.choice.NRUESecurityCapabilities);

  nrue_sec->nRencryptionAlgorithms.buf =dinput.encryption_algs;
  nrue_sec->nRencryptionAlgorithms.size = dinput.encryption_algs_size;
  nrue_sec->nRencryptionAlgorithms.bits_unused = dinput.encryption_algs_unused;

  nrue_sec->nRintegrityProtectionAlgorithms.buf = dinput.integrity_protection_algs;
  nrue_sec->nRintegrityProtectionAlgorithms.size = dinput.integrity_protection_algs_size;
  nrue_sec->nRintegrityProtectionAlgorithms.bits_unused = dinput.integrity_protection_algs_unused;

  ie_index = 2;
  ies_sgnb_addition_req= &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id = ProtocolIE_ID_id_SgNBSecurityKey ;
  ies_sgnb_addition_req->value.present = SgNBAdditionRequest_IEs__value_PR_SgNBSecurityKey;
  SgNBSecurityKey_t * sgnb_sec = &(ies_sgnb_addition_req->value.choice.SgNBSecurityKey);

  sgnb_sec->buf = dinput.sgnb_security_key;
  sgnb_sec->size = dinput.sgnb_security_key_size;
  sgnb_sec->bits_unused = dinput.sgnb_security_key_unused;

  ie_index = 3;
  ies_sgnb_addition_req= &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id = ProtocolIE_ID_id_SgNBUEAggregateMaximumBitRate;
  ies_sgnb_addition_req->value.present = SgNBAdditionRequest_IEs__value_PR_UEAggregateMaximumBitRate;
  UEAggregateMaximumBitRate_t * ue_rates = &(ies_sgnb_addition_req->value.choice.UEAggregateMaximumBitRate);

  // clear buffer if previously allocated
  if (ue_rates->uEaggregateMaximumBitRateDownlink.size > 0){
    ue_rates->uEaggregateMaximumBitRateDownlink.size = 0;
    free(ue_rates->uEaggregateMaximumBitRateDownlink.buf);
  }
  
  if (ue_rates->uEaggregateMaximumBitRateUplink.size > 0){
    ue_rates->uEaggregateMaximumBitRateUplink.size = 0;
    free(ue_rates->uEaggregateMaximumBitRateUplink.buf);
  }
  
  /* To do : Fix malloc done by this operation */
  asn_long2INTEGER(&(ue_rates->uEaggregateMaximumBitRateDownlink), dinput.bit_rate_max_dn);
  asn_long2INTEGER(&(ue_rates->uEaggregateMaximumBitRateUplink),   dinput.bit_rate_max_up);

  
  ie_index = 4;
  ies_sgnb_addition_req= &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id =  ProtocolIE_ID_id_MeNBtoSgNBContainer  ;
  ies_sgnb_addition_req->value.present =  SgNBAdditionRequest_IEs__value_PR_MeNBtoSgNBContainer;
  MeNBtoSgNBContainer_t * metosg_container = &(ies_sgnb_addition_req->value.choice.MeNBtoSgNBContainer);

  metosg_container->buf = dinput.menb_sgnb_container;
  metosg_container->size = dinput.menb_sgnb_container_size;


  ie_index = 5;
  ies_sgnb_addition_req= &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id = ProtocolIE_ID_id_MeNBCell_ID ;
  ies_sgnb_addition_req->value.present = SgNBAdditionRequest_IEs__value_PR_ECGI;
  ECGI_t * menb_ecgi = &(ies_sgnb_addition_req->value.choice.ECGI);
  
  menb_ecgi->pLMN_Identity.buf = dinput.plmn_identity;
  menb_ecgi->pLMN_Identity.size = dinput.plmn_identity_size;
  
  menb_ecgi->eUTRANcellIdentifier.buf = dinput.eutran_identifier;
  menb_ecgi->eUTRANcellIdentifier.size = dinput.eutran_identifier_size;
  menb_ecgi->eUTRANcellIdentifier.bits_unused = 4;

  ie_index = 6;
  ies_sgnb_addition_req = &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id =  ProtocolIE_ID_id_SubscriberProfileIDforRFP;
  ies_sgnb_addition_req->value.present = SgNBAdditionRequest_IEs__value_PR_SubscriberProfileIDforRFP;
  ies_sgnb_addition_req->value.choice.SubscriberProfileIDforRFP = dinput.subscriber_profile_id;
  
  // add erab-tobe-added list
  ie_index = 7;
  ies_sgnb_addition_req= &IE_array[ie_index];
  ies_sgnb_addition_req->criticality = Criticality_reject;
  ies_sgnb_addition_req->id =  ProtocolIE_ID_id_E_RABs_ToBeAdded_SgNBAddReqList;
  ies_sgnb_addition_req->value.present = SgNBAdditionRequest_IEs__value_PR_E_RABs_ToBeAdded_SgNBAddReqList;
  E_RABs_ToBeAdded_SgNBAddReqList_t  * erabs_toadd_list = &(ies_sgnb_addition_req->value.choice.E_RABs_ToBeAdded_SgNBAddReqList);
  
  std::vector<struct erab_item> * ref_erab_input  = &(dinput.erab_list);
  erabs_toadd_list->list.count = 0;
  
  // resize ?
  if (ref_erab_input->size() >= erab_array_size){
    erab_array_size = 2 * ref_erab_input->size();

    E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t * new_ref = 0;
    new_ref = (E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t *)realloc(erab_array, erab_array_size * sizeof(E_RABs_ToBeAdded_SgNBAddReq_Item_t));
    assert(new_ref != 0);
    erab_array = new_ref;
    
    erab_sgnb_present_array_size = 2 * ref_erab_input->size();
    E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent_t  * new_present = 0;
    new_present = (E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent_t  *)realloc(erab_sgnb_present_array, erab_sgnb_present_array_size * sizeof(E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent_t ));
    assert(new_present != 0);
    erab_sgnb_present_array = new_present;

    erab_sgnb_notpresent_array_size = 2 * ref_erab_input->size();
    E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent_t  * new_notpresent = 0;
    new_notpresent = (E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent_t  *)realloc(erab_sgnb_notpresent_array, erab_sgnb_notpresent_array_size * sizeof(E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent_t ));
    assert(new_notpresent != 0);
    erab_sgnb_notpresent_array = new_notpresent;

    
  }
  
  int sgnb_present_index = 0;
  int sgnb_notpresent_index = 0;
  
  for(unsigned int i = 0; i < ref_erab_input->size(); i++){


    erab_array[i].id = ProtocolIE_ID_id_E_RABs_ToBeAdded_SgNBAddReq_Item ;
    erab_array[i].criticality = Criticality_reject;
    erab_array[i].value.present =  E_RABs_ToBeAdded_SgNBAddReq_ItemIEs__value_PR_E_RABs_ToBeAdded_SgNBAddReq_Item;
    E_RABs_ToBeAdded_SgNBAddReq_Item_t * erab_item = &(erab_array[i].value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item);
    
    erab_item->e_RAB_ID = (*ref_erab_input)[i].erab_id;
    erab_item->drb_ID = (*ref_erab_input)[i].drb_id;

    erab_item->en_DC_ResourceConfiguration.pDCPatSgNB =  (*ref_erab_input)[i].pdcp_at_sgnb;
    erab_item->en_DC_ResourceConfiguration.mCGresources =  (*ref_erab_input)[i].mcg_resources;
    erab_item->en_DC_ResourceConfiguration.sCGresources =  (*ref_erab_input)[i].scg_resources;

    erab_item->resource_configuration.present = (E_RABs_ToBeAdded_SgNBAddReq_Item__resource_configuration_PR)(*ref_erab_input)[i].sgnb_pdcp_present;
    
    
    if( erab_item->resource_configuration.present == E_RABs_ToBeAdded_SgNBAddReq_Item__resource_configuration_PR_sgNBPDCPpresent){
      erab_sgnb_present_array[sgnb_present_index].full_E_RAB_Level_QoS_Parameters.qCI = (*ref_erab_input)[i].sgnb_item.qci;
      erab_sgnb_present_array[sgnb_present_index].full_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.priorityLevel = (*ref_erab_input)[i].sgnb_item.priority_level;
      erab_sgnb_present_array[sgnb_present_index].full_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionCapability = (*ref_erab_input)[i].sgnb_item.pre_emption_capability;
      erab_sgnb_present_array[sgnb_present_index].full_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionVulnerability = (*ref_erab_input)[i].sgnb_item.pre_emption_vulnerability;

      erab_sgnb_present_array[sgnb_present_index].s1_UL_GTPtunnelEndpoint.transportLayerAddress.buf = (*ref_erab_input)[i].sgnb_item.transport_layer_addr;
      erab_sgnb_present_array[sgnb_present_index].s1_UL_GTPtunnelEndpoint.transportLayerAddress.size = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_size;
      erab_sgnb_present_array[sgnb_present_index].s1_UL_GTPtunnelEndpoint.transportLayerAddress.bits_unused  = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_unused;

      erab_sgnb_present_array[sgnb_present_index].s1_UL_GTPtunnelEndpoint.gTP_TEID.buf = (*ref_erab_input)[i].sgnb_item.gtp_tei;
      erab_sgnb_present_array[sgnb_present_index].s1_UL_GTPtunnelEndpoint.gTP_TEID.size = (*ref_erab_input)[i].sgnb_item.gtp_tei_size;

      /* Force all optional parameters that are not set to NULL */
      erab_sgnb_present_array[sgnb_present_index].max_MCG_admit_E_RAB_Level_QoS_Parameters = NULL;
      erab_sgnb_present_array[sgnb_present_index].dL_Forwarding = NULL;
      erab_sgnb_present_array[sgnb_present_index].meNB_DL_GTP_TEIDatMCG = NULL;


      /* validate constraints ..*/      
      int ret_constr;
      
      ret_constr = asn_check_constraints(&asn_DEF_AllocationAndRetentionPriority, &erab_sgnb_present_array[sgnb_present_index].full_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for Allocation Priority.. Reason = " + error_string;
	return false;
      }

      ret_constr = asn_check_constraints(&asn_DEF_E_RAB_Level_QoS_Parameters, &erab_sgnb_present_array[sgnb_present_index].full_E_RAB_Level_QoS_Parameters, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for ERAB QoS Parameters.. Reason = " + error_string;
	return false;
      }

      
      ret_constr = asn_check_constraints(&asn_DEF_GTP_TEI, &erab_sgnb_present_array[sgnb_present_index].s1_UL_GTPtunnelEndpoint.gTP_TEID, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for GTP TEID.. Reason = " + error_string;
	return false;
      }

      ret_constr = asn_check_constraints(&asn_DEF_GTPtunnelEndpoint, &erab_sgnb_present_array[sgnb_present_index].s1_UL_GTPtunnelEndpoint, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for GTP tunnel endpoint .. Reason = " + error_string;
	return false;
      }

      
      ret_constr = asn_check_constraints(&asn_DEF_E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent, &erab_sgnb_present_array[sgnb_present_index], errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for E_RABS_ToBeAdded_SgNBAddReq_SgNBPDCPpresent.. Reason = " + error_string;
	return false;
      }

      
      erab_item->resource_configuration.choice.sgNBPDCPpresent = &erab_sgnb_present_array[sgnb_present_index];

      sgnb_present_index ++;
      
    }
    else if (erab_item->resource_configuration.present ==  E_RABs_ToBeAdded_SgNBAddReq_Item__resource_configuration_PR_sgNBPDCPnotpresent){

      
      erab_sgnb_notpresent_array[sgnb_notpresent_index].requested_SCG_E_RAB_Level_QoS_Parameters.qCI = (*ref_erab_input)[i].sgnb_item.qci;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].requested_SCG_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.priorityLevel = (*ref_erab_input)[i].sgnb_item.priority_level;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].requested_SCG_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionCapability = (*ref_erab_input)[i].sgnb_item.pre_emption_capability;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].requested_SCG_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionVulnerability = (*ref_erab_input)[i].sgnb_item.pre_emption_vulnerability;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].requested_SCG_E_RAB_Level_QoS_Parameters.gbrQosInformation = 0;
      
      erab_sgnb_notpresent_array[sgnb_notpresent_index].meNB_UL_GTP_TEIDatPDCP.transportLayerAddress.buf = (*ref_erab_input)[i].sgnb_item.transport_layer_addr;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].meNB_UL_GTP_TEIDatPDCP.transportLayerAddress.size = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_size;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].meNB_UL_GTP_TEIDatPDCP.transportLayerAddress.bits_unused  = (*ref_erab_input)[i].sgnb_item.transport_layer_addr_unused;
      
      erab_sgnb_notpresent_array[sgnb_notpresent_index].meNB_UL_GTP_TEIDatPDCP.gTP_TEID.buf = (unsigned char *)(*ref_erab_input)[i].sgnb_item.gtp_tei;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].meNB_UL_GTP_TEIDatPDCP.gTP_TEID.size = (*ref_erab_input)[i].sgnb_item.gtp_tei_size;

      erab_sgnb_notpresent_array[sgnb_notpresent_index].rlc_Mode = (*ref_erab_input)[i].sgnb_item.rlc_mode;

      /* validate constraints ..*/      
      int ret_constr;
      
      ret_constr = asn_check_constraints(&asn_DEF_AllocationAndRetentionPriority, &erab_sgnb_notpresent_array[sgnb_notpresent_index].requested_SCG_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for Allocation Priority.. Reason = " + error_string;
	return false;
      }

      ret_constr = asn_check_constraints(&asn_DEF_E_RAB_Level_QoS_Parameters, &erab_sgnb_notpresent_array[sgnb_notpresent_index].requested_SCG_E_RAB_Level_QoS_Parameters, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for ERAB QoS Parameters.. Reason = " + error_string;
	return false;
      }

      
      ret_constr = asn_check_constraints(&asn_DEF_GTP_TEI, &erab_sgnb_notpresent_array[sgnb_notpresent_index].meNB_UL_GTP_TEIDatPDCP.gTP_TEID, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for GTP TEID.. Reason = " + error_string;
	return false;
      }

      ret_constr = asn_check_constraints(&asn_DEF_GTPtunnelEndpoint, &erab_sgnb_notpresent_array[sgnb_notpresent_index].meNB_UL_GTP_TEIDatPDCP, errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for GTP tunnel endpoint .. Reason = " + error_string;
	return false;
      }

      
      ret_constr = asn_check_constraints(&asn_DEF_E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent, &erab_sgnb_notpresent_array[sgnb_notpresent_index], errbuf, &errbuf_len);
      if(ret_constr){
	error_string.assign(&errbuf[0], errbuf_len);
	error_string = "Error in constraints for E_RABS_ToBeAdded_SgNBAddReq_SgNBPDCPnotpresent.. Reason = " + error_string;
	return false;
      }


      /* Force all optional parameters that are not set to NULL */
      erab_sgnb_notpresent_array[sgnb_notpresent_index].uL_Configuration = 0;
      erab_sgnb_notpresent_array[sgnb_notpresent_index].secondary_meNB_UL_GTP_TEIDatPDCP = 0;


      erab_item->resource_configuration.choice.sgNBPDCPnotpresent = &erab_sgnb_notpresent_array[sgnb_notpresent_index]; ;
      sgnb_notpresent_index ++;
    }
    else{
      continue;
    }
    
    // check constraint
    int ret_constr = asn_check_constraints(&asn_DEF_E_RABs_ToBeAdded_SgNBAddReq_ItemIEs, &erab_array[i], errbuf, &errbuf_len);
    if(ret_constr){
      error_string.assign(&errbuf[0], errbuf_len);
      error_string = "Error in constraints for E_RABS_ToBeAdded_SgNBAddReq_ItemIEs.. Reason = " + error_string;
      return false;
    }
    
    ASN_SEQUENCE_ADD(erabs_toadd_list, &erab_array[i]);
    
  }
  
  return true;

}


bool sgnb_addition_request::get_fields(X2InitiatingMessage_t * init_msg, sgnb_addition_helper & dout){
    
  if (init_msg == 0){
    error_string = "Error ! Invalid reference for SgNB Addition Request get_fields";
    return false;
  }
  
  dout.clear();
  int res = 0;
  
  struct erab_item * eitem;
  std::vector<struct erab_item> *erab_list;
  
  SgNBAdditionRequest_IEs_t *memb_ptr;
  for(int edx = 0; edx < init_msg->value.choice.SgNBAdditionRequest.protocolIEs.list.count; edx++){
    memb_ptr = init_msg->value.choice.SgNBAdditionRequest.protocolIEs.list.array[edx];
    
    switch(memb_ptr->id){

    case (ProtocolIE_ID_id_MeNB_UE_X2AP_ID):
      dout.menb_ue_x2ap_id = memb_ptr->value.choice.UE_X2AP_ID;
      break;

    case (ProtocolIE_ID_id_SelectedPLMN):
      dout.selected_plmn = memb_ptr->value.choice.PLMN_Identity.buf;
      dout.selected_plmn_size = memb_ptr->value.choice.PLMN_Identity.size;
      break;
      
    case (ProtocolIE_ID_id_NRUESecurityCapabilities):
      dout.encryption_algs = memb_ptr->value.choice.NRUESecurityCapabilities.nRencryptionAlgorithms.buf;
      dout.encryption_algs_size = memb_ptr->value.choice.NRUESecurityCapabilities.nRencryptionAlgorithms.size;
      dout.encryption_algs_unused = memb_ptr->value.choice.NRUESecurityCapabilities.nRencryptionAlgorithms.bits_unused;

      dout.integrity_protection_algs= memb_ptr->value.choice.NRUESecurityCapabilities.nRintegrityProtectionAlgorithms.buf;
      dout.integrity_protection_algs_size = memb_ptr->value.choice.NRUESecurityCapabilities.nRintegrityProtectionAlgorithms.size;
      dout.integrity_protection_algs_unused = memb_ptr->value.choice.NRUESecurityCapabilities.nRintegrityProtectionAlgorithms.bits_unused;

      break;

    case (ProtocolIE_ID_id_SgNBSecurityKey):
      dout.sgnb_security_key = memb_ptr->value.choice.SgNBSecurityKey.buf;
      dout.sgnb_security_key_size = memb_ptr->value.choice.SgNBSecurityKey.size;
      dout.sgnb_security_key_unused = memb_ptr->value.choice.SgNBSecurityKey.bits_unused;

      break;
      
    case (ProtocolIE_ID_id_SgNBUEAggregateMaximumBitRate):
      res = asn_INTEGER2long(&(memb_ptr->value.choice.UEAggregateMaximumBitRate.uEaggregateMaximumBitRateDownlink), &dout.bit_rate_max_dn );
      if (res == -1){
	error_string = "Error converting uEaggregateMaximumBitRateDownlink ";
	return false;
      }
      
      res = asn_INTEGER2long(&(memb_ptr->value.choice.UEAggregateMaximumBitRate.uEaggregateMaximumBitRateUplink), &dout.bit_rate_max_up );
      if (res == -1){
	error_string = "Error converting uEaggregateMaximumBitRateUplink ";
	return false;
      }

      
      break;
      
    case (ProtocolIE_ID_id_MeNBtoSgNBContainer):
      dout.menb_sgnb_container = memb_ptr->value.choice.MeNBtoSgNBContainer.buf;
      dout.menb_sgnb_container_size = memb_ptr->value.choice.MeNBtoSgNBContainer.size;
      break;
      
    case (ProtocolIE_ID_id_SgNB_UE_X2AP_ID):
      dout.sgnb_ue_x2ap_id = memb_ptr->value.choice.SgNB_UE_X2AP_ID;
      break;
      
    case (ProtocolIE_ID_id_SubscriberProfileIDforRFP):
      //sgnb->subscriberprofileidforrfp = memb_ptr->value.choice.SubscriberProfileIDforRFP;
      break;
      
    case (ProtocolIE_ID_id_MeNBCell_ID):

      dout.eutran_identifier = memb_ptr->value.choice.ECGI.eUTRANcellIdentifier.buf;
      dout.eutran_identifier_size = memb_ptr->value.choice.ECGI.eUTRANcellIdentifier.size;
      dout.eutran_identifier_unused = memb_ptr->value.choice.ECGI.eUTRANcellIdentifier.bits_unused;

      dout.plmn_identity = memb_ptr->value.choice.ECGI.pLMN_Identity.buf;
      dout.plmn_identity_size = memb_ptr->value.choice.ECGI.pLMN_Identity.size;
      break;
      
    case (ProtocolIE_ID_id_E_RABs_ToBeAdded_SgNBAddReqList):

      erab_list = & dout.erab_list;
      
      for(int erabs=0; erabs < memb_ptr->value.choice.E_RABs_ToBeAdded_SgNBAddReqList.list.count;erabs++){
	E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t * erab_item_ie = (E_RABs_ToBeAdded_SgNBAddReq_ItemIEs_t *)memb_ptr->value.choice.E_RABs_ToBeAdded_SgNBAddReqList.list.array[erabs];

	erab_list->emplace_back();
	eitem = &(*erab_list)[erabs];
	
	//erab_item *eitem = (erab_item*)calloc(1, sizeof(erab_item));
	
	eitem->drb_id = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.drb_ID;
	eitem->erab_id = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.e_RAB_ID;

	eitem->mcg_resources = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.en_DC_ResourceConfiguration.mCGresources;
	eitem->scg_resources = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.en_DC_ResourceConfiguration.sCGresources;
	eitem->pdcp_at_sgnb = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.en_DC_ResourceConfiguration.pDCPatSgNB;
	eitem->sgnb_pdcp_present = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.resource_configuration.present;
	
	if(erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.resource_configuration.present == E_RABs_ToBeAdded_SgNBAddReq_Item__resource_configuration_PR_sgNBPDCPpresent){

	  E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPpresent_t *erab_sgnb_present_ie = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.resource_configuration.choice.sgNBPDCPpresent;

	  eitem->sgnb_item.qci = erab_sgnb_present_ie->full_E_RAB_Level_QoS_Parameters.qCI;
	  eitem->sgnb_item.priority_level = erab_sgnb_present_ie->full_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.priorityLevel;
	  eitem->sgnb_item.pre_emption_capability = erab_sgnb_present_ie->full_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionCapability;
	  eitem->sgnb_item.pre_emption_vulnerability = erab_sgnb_present_ie->full_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionVulnerability;
	  
	  
	  eitem->sgnb_item.gtp_tei =  erab_sgnb_present_ie->s1_UL_GTPtunnelEndpoint.gTP_TEID.buf;
	  eitem->sgnb_item.gtp_tei_size = erab_sgnb_present_ie->s1_UL_GTPtunnelEndpoint.gTP_TEID.size;
	  
	  eitem->sgnb_item.transport_layer_addr = erab_sgnb_present_ie->s1_UL_GTPtunnelEndpoint.transportLayerAddress.buf;
	  eitem->sgnb_item.transport_layer_addr_size = erab_sgnb_present_ie->s1_UL_GTPtunnelEndpoint.transportLayerAddress.size;
	  eitem->sgnb_item.transport_layer_addr_unused = erab_sgnb_present_ie->s1_UL_GTPtunnelEndpoint.transportLayerAddress.bits_unused;
	  
	  
	}
	else if(erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.resource_configuration.present == E_RABs_ToBeAdded_SgNBAddReq_Item__resource_configuration_PR_sgNBPDCPnotpresent){
	  E_RABs_ToBeAdded_SgNBAddReq_Item_SgNBPDCPnotpresent_t *erab_sgnb_notpresent_ie = erab_item_ie->value.choice.E_RABs_ToBeAdded_SgNBAddReq_Item.resource_configuration.choice.sgNBPDCPnotpresent;
	  eitem->sgnb_item.rlc_mode = erab_sgnb_notpresent_ie->rlc_Mode;
	  eitem->sgnb_item.gtp_tei = erab_sgnb_notpresent_ie->meNB_UL_GTP_TEIDatPDCP.gTP_TEID.buf;
	  eitem->sgnb_item.gtp_tei_size = erab_sgnb_notpresent_ie->meNB_UL_GTP_TEIDatPDCP.gTP_TEID.size;
	  eitem->sgnb_item.transport_layer_addr = erab_sgnb_notpresent_ie->meNB_UL_GTP_TEIDatPDCP.transportLayerAddress.buf;
	  eitem->sgnb_item.transport_layer_addr_size = erab_sgnb_notpresent_ie->meNB_UL_GTP_TEIDatPDCP.transportLayerAddress.size;
	  eitem->sgnb_item.transport_layer_addr_unused = erab_sgnb_notpresent_ie->meNB_UL_GTP_TEIDatPDCP.transportLayerAddress.bits_unused;

	  eitem->sgnb_item.qci = erab_sgnb_notpresent_ie->requested_SCG_E_RAB_Level_QoS_Parameters.qCI;
	  eitem->sgnb_item.pre_emption_capability = erab_sgnb_notpresent_ie->requested_SCG_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionCapability;
	  eitem->sgnb_item.pre_emption_vulnerability = erab_sgnb_notpresent_ie->requested_SCG_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.pre_emptionVulnerability;
	  eitem->sgnb_item.priority_level = erab_sgnb_notpresent_ie->requested_SCG_E_RAB_Level_QoS_Parameters.allocationAndRetentionPriority.priorityLevel;

	}
	else{
	  mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: E-RAB does not have sgNBPDCP present/not present set flag", __FILE__, __LINE__);
	  return false;
	}
	
      }
      
      break;
      
    default:
      break;

    }

  }
  return true;
}
