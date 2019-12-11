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
 * unit_test_sgnb_add_request.cc
 *
 *  Created on: Sep 9, 2019
 *      Author: sjana
 */

#define CATCH_CONFIG_MAIN
#include <sgnb_addition_request.hpp>
#include <sgnb_addition_response.hpp>
#include <mdclog/mdclog.h>
#include <catch2/catch.hpp>

#define BUFFER_SIZE 2048
TEST_CASE("X2AP PDUs", "[X2AP SgNB Addition Request]"){
  
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST XAPP FRAMEWORK");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_DEBUG);
  mdclog_attr_destroy(attr);
  
  bool res;
	
  sgnb_addition_helper encode_data;
  sgnb_addition_helper decode_data;

  sgnb_addition_request memcheck_req;
  sgnb_addition_request encode_test;
  sgnb_addition_request decode_test;
  unsigned char buf[BUFFER_SIZE];
  size_t buf_size = BUFFER_SIZE;

  sgnb_addition_response memcheck_resp;	
  sgnb_addition_response resp_out;
  
  unsigned char data_buf[BUFFER_SIZE];
  size_t data_size = BUFFER_SIZE;

  // sgnb data creation static parameters
  unsigned char enc_alg[] = "Bl";
  unsigned char integrity_alg[] = "md";
  unsigned char  sgnb_security_key[] = "12345678912345678912345678912345";
  unsigned char  mesgnb_container[] = "Information";
  unsigned char transport_layer_addr[] = "AABBCCDDEE";
  unsigned char gtp_tei[] = "ZZYX";
  unsigned char plmn[] = "321";
  unsigned char eutran[] = "4567";

  SECTION("Verify encoding/decoding of valid data with variable number of erabs"){

    encode_data.menb_ue_x2ap_id = 12;
    encode_data.sgnb_ue_x2ap_id = 10;

    encode_data.bit_rate_max_up = 1000;
    encode_data.bit_rate_max_dn = 2000;

    encode_data.encryption_algs = enc_alg;
    encode_data.encryption_algs_size = strlen((char *)enc_alg);
    encode_data.encryption_algs_unused = 0;

    encode_data.integrity_protection_algs = integrity_alg;
    encode_data.integrity_protection_algs_size = strlen((char *)integrity_alg);
    encode_data.integrity_protection_algs_unused = encode_data.integrity_protection_algs_size/8;
    REQUIRE(encode_data.integrity_protection_algs_size - encode_data.integrity_protection_algs_unused == 2);


    encode_data.sgnb_security_key = sgnb_security_key;
    encode_data.sgnb_security_key_size = strlen((char *)sgnb_security_key);
    encode_data.sgnb_security_key_unused = 0;
    REQUIRE(encode_data.sgnb_security_key_size - encode_data.sgnb_security_key_unused == 32);


    encode_data.menb_sgnb_container = mesgnb_container;
    encode_data.menb_sgnb_container_size = strlen((char *)mesgnb_container);

    encode_data.plmn_identity = plmn;
    encode_data.plmn_identity_size = strlen((char *)plmn);

    encode_data.eutran_identifier = eutran;
    encode_data.eutran_identifier_size = strlen((char *)eutran);
    encode_data.eutran_identifier_unused = 4;

    REQUIRE(encode_data.eutran_identifier_size*8 - encode_data.eutran_identifier_unused == 28);
    encode_data.subscriber_profile_id = 34;

    std::vector<erab_item> * ref_array = encode_data.get_list();
    int max_erabs = 30;
    srand(12093021);

    // pdpc present
    for(int num_erabs = 1; num_erabs < max_erabs; num_erabs++){

      ref_array->clear();
    
    
      for(int i = 0; i < num_erabs; i++){
      
	ref_array->emplace_back();
	(*ref_array)[i].erab_id = rand() %15;
	(*ref_array)[i].drb_id = i%32 + 1;
	(*ref_array)[i].pdcp_at_sgnb = 1;
	(*ref_array)[i].mcg_resources = 1;
	(*ref_array)[i].scg_resources = 1;
	(*ref_array)[i].sgnb_pdcp_present = 1;
	
	// element sgnb present
	(*ref_array)[i].sgnb_item.qci = 8;
	(*ref_array)[i].sgnb_item.priority_level = 8;
	(*ref_array)[i].sgnb_item.pre_emption_capability = 0;
	(*ref_array)[i].sgnb_item.pre_emption_vulnerability = 1;
	(*ref_array)[i].sgnb_item.transport_layer_addr = transport_layer_addr;
	(*ref_array)[i].sgnb_item.transport_layer_addr_size = strlen((char *)transport_layer_addr);
	(*ref_array)[i].sgnb_item.gtp_tei = gtp_tei;
	(*ref_array)[i].sgnb_item.gtp_tei_size = strlen((char *)gtp_tei);
	(*ref_array)[i].sgnb_item.rlc_mode = 1;
	
      }
      buf_size = BUFFER_SIZE;
      res = encode_test.encode_sgnb_addition_request(buf, &buf_size, encode_data);
      REQUIRE(res == true);

      // std::string out_file = std::string("X2AP-PDU-SgNBAdditionRequest-NumErabs-") + std::to_string(num_erabs) + std::string(".per");
      // FILE *pfile = fopen(out_file.c_str(), "w");
      // fwrite(buf, sizeof(char), buf_size, pfile);
      // fclose(pfile);

      // How many times we test decoding the same data
      int num_tests = 10;
      for(int i = 0; i < num_tests; i++){
	decode_data.clear();
	
	X2N_X2AP_PDU_t *x2ap_pdu_obj = 0;
	asn_dec_rval_t rval = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_X2N_X2AP_PDU, (void**)&x2ap_pdu_obj, buf, buf_size);
	REQUIRE(rval.code == RC_OK);
	
	res = decode_test.get_fields(x2ap_pdu_obj->choice.initiatingMessage, decode_data);
	REQUIRE(res==true);
	
	REQUIRE(decode_data.sgnb_security_key_size - decode_data.sgnb_security_key_unused == 32);
	REQUIRE(decode_data.eutran_identifier_size*8 - decode_data.eutran_identifier_unused == 28);
	REQUIRE(encode_data.get_list()->size() == decode_data.get_list()->size());
	REQUIRE(encode_data.erab_list[0].erab_id == decode_data.erab_list[0].erab_id);
	REQUIRE(encode_data.erab_list[0].drb_id == decode_data.erab_list[0].drb_id);
	REQUIRE(encode_data.erab_list[0].pdcp_at_sgnb == decode_data.erab_list[0].pdcp_at_sgnb);
	
	REQUIRE(encode_data.menb_ue_x2ap_id == decode_data.menb_ue_x2ap_id);
	REQUIRE(encode_data.bit_rate_max_up == decode_data.bit_rate_max_up);
	REQUIRE(encode_data.bit_rate_max_dn == decode_data.bit_rate_max_dn);
	REQUIRE(encode_data.subscriber_profile_id == decode_data.subscriber_profile_id);
	ASN_STRUCT_FREE(asn_DEF_X2N_X2AP_PDU, x2ap_pdu_obj);
      }
      
    }


    // pdcp not present
    for(int num_erabs = 1; num_erabs < max_erabs; num_erabs++){

      ref_array->clear();
      
      for(int i = 0; i < num_erabs; i++){
      
	ref_array->emplace_back();
	(*ref_array)[i].erab_id = rand() %15;
	(*ref_array)[i].drb_id = i%32 + 1;
	(*ref_array)[i].pdcp_at_sgnb = 1;
	(*ref_array)[i].mcg_resources = 1;
	(*ref_array)[i].scg_resources = 1;
	(*ref_array)[i].sgnb_pdcp_present = 2;
	
	// element sgnb present
	(*ref_array)[i].sgnb_item.qci = 8;
	(*ref_array)[i].sgnb_item.priority_level = 8;
	(*ref_array)[i].sgnb_item.pre_emption_capability = 0;
	(*ref_array)[i].sgnb_item.pre_emption_vulnerability = 1;
	(*ref_array)[i].sgnb_item.transport_layer_addr = transport_layer_addr;
	(*ref_array)[i].sgnb_item.transport_layer_addr_size = strlen((char *)transport_layer_addr);
	(*ref_array)[i].sgnb_item.gtp_tei = gtp_tei;
	(*ref_array)[i].sgnb_item.gtp_tei_size = strlen((char *)gtp_tei);
	(*ref_array)[i].sgnb_item.rlc_mode = 1;
	
      }
      
      buf_size = BUFFER_SIZE;
      res = encode_test.encode_sgnb_addition_request(buf, &buf_size, encode_data);
      REQUIRE(res == true);
      
      // std::string out_file = std::string("X2AP-PDU-SgNBAdditionRequest-NoPDCP-NumErabs-") + std::to_string(num_erabs) + std::string(".per");
      // FILE *pfile = fopen(out_file.c_str(), "w");
      // fwrite(buf, sizeof(char), buf_size, pfile);
      // fclose(pfile);
      
      
      int num_tests = 10;
      for(int i = 0; i < num_tests; i++){
	decode_data.clear();
	
	X2N_X2AP_PDU_t *x2ap_pdu_obj = 0;
	asn_dec_rval_t rval = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_X2N_X2AP_PDU, (void**)&x2ap_pdu_obj, buf, buf_size);
	REQUIRE(rval.code == RC_OK);
	
	res = decode_test.get_fields(x2ap_pdu_obj->choice.initiatingMessage, decode_data);
	REQUIRE(res==true);
	
	REQUIRE(decode_data.sgnb_security_key_size - decode_data.sgnb_security_key_unused == 32);
	REQUIRE(decode_data.eutran_identifier_size*8 - decode_data.eutran_identifier_unused == 28);
	REQUIRE(encode_data.get_list()->size() == decode_data.get_list()->size());
	REQUIRE(encode_data.erab_list[0].erab_id == decode_data.erab_list[0].erab_id);
	REQUIRE(encode_data.erab_list[0].drb_id == decode_data.erab_list[0].drb_id);
	REQUIRE(encode_data.erab_list[0].pdcp_at_sgnb == decode_data.erab_list[0].pdcp_at_sgnb);
	
	REQUIRE(encode_data.menb_ue_x2ap_id == decode_data.menb_ue_x2ap_id);
	REQUIRE(encode_data.bit_rate_max_up == decode_data.bit_rate_max_up);
	REQUIRE(encode_data.bit_rate_max_dn == decode_data.bit_rate_max_dn);
	REQUIRE(encode_data.subscriber_profile_id == decode_data.subscriber_profile_id);

	ASN_STRUCT_FREE(asn_DEF_X2N_X2AP_PDU, x2ap_pdu_obj);
      }
      
    }
    
  }
  
  SECTION("Encoding, invalid erab"){

    encode_data.menb_ue_x2ap_id = 12;
    encode_data.sgnb_ue_x2ap_id = 120;

    encode_data.bit_rate_max_up = 1000;
    encode_data.bit_rate_max_dn = 2000;

    encode_data.encryption_algs = enc_alg;
    encode_data.encryption_algs_size = strlen((char *)enc_alg);
    encode_data.encryption_algs_unused = 0;

    encode_data.integrity_protection_algs = integrity_alg;
    encode_data.integrity_protection_algs_size = strlen((char *)integrity_alg);
    encode_data.integrity_protection_algs_unused = encode_data.integrity_protection_algs_size/8;
    REQUIRE(encode_data.integrity_protection_algs_size - encode_data.integrity_protection_algs_unused == 2);


    encode_data.sgnb_security_key = sgnb_security_key;
    encode_data.sgnb_security_key_size = strlen((char *)sgnb_security_key);
    encode_data.sgnb_security_key_unused = 0;
    REQUIRE(encode_data.sgnb_security_key_size - encode_data.sgnb_security_key_unused == 32);


    encode_data.menb_sgnb_container = mesgnb_container;
    encode_data.menb_sgnb_container_size = strlen((char *)mesgnb_container);

    encode_data.plmn_identity = plmn;
    encode_data.plmn_identity_size = strlen((char *)plmn);

    encode_data.eutran_identifier = eutran;
    encode_data.eutran_identifier_size = strlen((char *)eutran);
    encode_data.eutran_identifier_unused = 4;

    REQUIRE(encode_data.eutran_identifier_size*8 - encode_data.eutran_identifier_unused == 28);
    encode_data.subscriber_profile_id = 34;

    std::vector<erab_item> * ref_array = encode_data.get_list();
    ref_array->clear();
    
    ref_array->emplace_back();
    (*ref_array)[0].erab_id = rand() %15;
    (*ref_array)[0].drb_id = 150;
    (*ref_array)[0].pdcp_at_sgnb = 1;
    (*ref_array)[0].mcg_resources = 1;
    (*ref_array)[0].scg_resources = 1;
    (*ref_array)[0].sgnb_pdcp_present = 1;
    
    // element sgnb present
    (*ref_array)[0].sgnb_item.qci = 8;
    (*ref_array)[0].sgnb_item.priority_level = 8;
    (*ref_array)[0].sgnb_item.pre_emption_capability = 0;
    (*ref_array)[0].sgnb_item.pre_emption_vulnerability = 1;
    (*ref_array)[0].sgnb_item.transport_layer_addr = transport_layer_addr;
    (*ref_array)[0].sgnb_item.transport_layer_addr_size = strlen((char *)transport_layer_addr);
    (*ref_array)[0].sgnb_item.gtp_tei = gtp_tei;
    (*ref_array)[0].sgnb_item.gtp_tei_size = strlen((char *)gtp_tei);
    (*ref_array)[0].sgnb_item.rlc_mode = 1;
    
    res = encode_test.encode_sgnb_addition_request(buf, &buf_size, encode_data);
    REQUIRE(res == false);
  }

  SECTION("Invalid Encoding, low buffer"){

    encode_data.menb_ue_x2ap_id = 12;
    encode_data.sgnb_ue_x2ap_id = 120;

    encode_data.bit_rate_max_up = 1000;
    encode_data.bit_rate_max_dn = 2000;

    encode_data.encryption_algs = enc_alg;
    encode_data.encryption_algs_size = strlen((char *)enc_alg);
    encode_data.encryption_algs_unused = 0;

    encode_data.integrity_protection_algs = integrity_alg;
    encode_data.integrity_protection_algs_size = strlen((char *)integrity_alg);
    encode_data.integrity_protection_algs_unused = encode_data.integrity_protection_algs_size/8;
    REQUIRE(encode_data.integrity_protection_algs_size - encode_data.integrity_protection_algs_unused == 2);


    encode_data.sgnb_security_key = sgnb_security_key;
    encode_data.sgnb_security_key_size = strlen((char *)sgnb_security_key);
    encode_data.sgnb_security_key_unused = 0;
    REQUIRE(encode_data.sgnb_security_key_size - encode_data.sgnb_security_key_unused == 32);


    encode_data.menb_sgnb_container = mesgnb_container;
    encode_data.menb_sgnb_container_size = strlen((char *)mesgnb_container);

    encode_data.plmn_identity = plmn;
    encode_data.plmn_identity_size = strlen((char *)plmn);

    encode_data.eutran_identifier = eutran;
    encode_data.eutran_identifier_size = strlen((char *)eutran);
    encode_data.eutran_identifier_unused = 4;

    REQUIRE(encode_data.eutran_identifier_size*8 - encode_data.eutran_identifier_unused == 28);
    encode_data.subscriber_profile_id = 34;

    std::vector<erab_item> * ref_array = encode_data.get_list();
    ref_array->clear();
    
    ref_array->emplace_back();
    (*ref_array)[0].erab_id = rand() %15;
    (*ref_array)[0].drb_id = 150;
    (*ref_array)[0].pdcp_at_sgnb = 1;
    (*ref_array)[0].mcg_resources = 1;
    (*ref_array)[0].scg_resources = 1;
    (*ref_array)[0].sgnb_pdcp_present = 1;
    
    // element sgnb present
    (*ref_array)[0].sgnb_item.qci = 8;
    (*ref_array)[0].sgnb_item.priority_level = 8;
    (*ref_array)[0].sgnb_item.pre_emption_capability = 0;
    (*ref_array)[0].sgnb_item.pre_emption_vulnerability = 1;
    (*ref_array)[0].sgnb_item.transport_layer_addr = transport_layer_addr;
    (*ref_array)[0].sgnb_item.transport_layer_addr_size = strlen((char *)transport_layer_addr);
    (*ref_array)[0].sgnb_item.gtp_tei = gtp_tei;
    (*ref_array)[0].sgnb_item.gtp_tei_size = strlen((char *)gtp_tei);
    (*ref_array)[0].sgnb_item.rlc_mode = 1;

    buf_size = 10;
    res = encode_test.encode_sgnb_addition_request(buf, &buf_size, encode_data);
    REQUIRE(res == false);
  }
  
  SECTION("Encoding, missing erab"){

    encode_data.menb_ue_x2ap_id = 12;
    encode_data.sgnb_ue_x2ap_id = 10;

    encode_data.bit_rate_max_up = 1000;
    encode_data.bit_rate_max_dn = 2000;

    encode_data.encryption_algs = enc_alg;
    encode_data.encryption_algs_size = strlen((char *)enc_alg);
    encode_data.encryption_algs_unused = 0;

    encode_data.integrity_protection_algs = integrity_alg;
    encode_data.integrity_protection_algs_size = strlen((char *)integrity_alg);
    encode_data.integrity_protection_algs_unused = encode_data.integrity_protection_algs_size/8;
    REQUIRE(encode_data.integrity_protection_algs_size - encode_data.integrity_protection_algs_unused == 2);


    encode_data.sgnb_security_key = sgnb_security_key;
    encode_data.sgnb_security_key_size = strlen((char *)sgnb_security_key);
    encode_data.sgnb_security_key_unused = 0;
    REQUIRE(encode_data.sgnb_security_key_size - encode_data.sgnb_security_key_unused == 32);


    encode_data.menb_sgnb_container = mesgnb_container;
    encode_data.menb_sgnb_container_size = strlen((char *)mesgnb_container);

    encode_data.plmn_identity = plmn;
    encode_data.plmn_identity_size = strlen((char *)plmn);

    encode_data.eutran_identifier = eutran;
    encode_data.eutran_identifier_size = strlen((char *)eutran);
    encode_data.eutran_identifier_unused = 4;

    encode_data.subscriber_profile_id = 34;
    res = encode_test.encode_sgnb_addition_request(buf, &buf_size, encode_data);
    REQUIRE(res == false);
  }

  
  
  SECTION("SgnB Addition Response ACK Encoding/decoding with valid data"){

    encode_data.menb_ue_x2ap_id = 12;
    encode_data.sgnb_ue_x2ap_id = 10;

    encode_data.bit_rate_max_up = 1000;
    encode_data.bit_rate_max_dn = 2000;

    encode_data.encryption_algs = enc_alg;
    encode_data.encryption_algs_size = strlen((char *)enc_alg);
    encode_data.encryption_algs_unused = 0;

    encode_data.integrity_protection_algs = integrity_alg;
    encode_data.integrity_protection_algs_size = strlen((char *)integrity_alg);
    encode_data.integrity_protection_algs_unused = encode_data.integrity_protection_algs_size/8;
    REQUIRE(encode_data.integrity_protection_algs_size - encode_data.integrity_protection_algs_unused == 2);


    encode_data.sgnb_security_key = sgnb_security_key;
    encode_data.sgnb_security_key_size = strlen((char *)sgnb_security_key);
    encode_data.sgnb_security_key_unused = 0;
    REQUIRE(encode_data.sgnb_security_key_size - encode_data.sgnb_security_key_unused == 32);


    encode_data.menb_sgnb_container = mesgnb_container;
    encode_data.menb_sgnb_container_size = strlen((char *)mesgnb_container);

    encode_data.plmn_identity = plmn;
    encode_data.plmn_identity_size = strlen((char *)plmn);

    encode_data.eutran_identifier = eutran;
    encode_data.eutran_identifier_size = strlen((char *)eutran);
    encode_data.eutran_identifier_unused = 4;

    REQUIRE(encode_data.eutran_identifier_size*8 - encode_data.eutran_identifier_unused == 28);
    encode_data.subscriber_profile_id = 34;

    std::vector<erab_item> * ref_array = encode_data.get_list();
    ref_array->clear();
    
    int num_erabs = 25;
    srand(12093021);

    for(int i = 0; i < num_erabs; i++){
      
      ref_array->emplace_back();
      (*ref_array)[i].erab_id = rand() %15;
      (*ref_array)[i].drb_id = i%32 + 1;
      (*ref_array)[i].pdcp_at_sgnb = 1;
      (*ref_array)[i].mcg_resources = 1;
      (*ref_array)[i].scg_resources = 1;
      (*ref_array)[i].sgnb_pdcp_present = 1;

      // element sgnb present
      (*ref_array)[i].sgnb_item.qci = 8;
      (*ref_array)[i].sgnb_item.priority_level = 8;
      (*ref_array)[i].sgnb_item.pre_emption_capability = 0;
      (*ref_array)[i].sgnb_item.pre_emption_vulnerability = 1;
      (*ref_array)[i].sgnb_item.transport_layer_addr = transport_layer_addr;
      (*ref_array)[i].sgnb_item.transport_layer_addr_size = strlen((char *)transport_layer_addr);
      (*ref_array)[i].sgnb_item.gtp_tei = gtp_tei;
      (*ref_array)[i].sgnb_item.gtp_tei_size = strlen((char *)gtp_tei);
      (*ref_array)[i].sgnb_item.rlc_mode = 1;
      
    }

    res = resp_out.encode_sgnb_addition_response(buf, &buf_size, encode_data, true);
    REQUIRE(res == true);


    // Haven't written get fields for sgnb response yet
    // int num_tests = 10;
    // for(int i = 0; i < num_tests; i++){
    //   decode_data.clear();
      
    //   X2_X2AP_PDU_t *x2ap_pdu_obj = 0;
    //   asn_dec_rval_t rval = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_X2N_X2AP_PDU, (void**)&x2ap_pdu_obj, buf, buf_size);
    //   REQUIRE(rval.code == RC_OK);
      
    //   res = resp_out.get_fields(x2ap_pdu_obj->choice.successfulOutcome, decode_data);
    //   REQUIRE(res==true);
      
    //   REQUIRE(decode_data.sgnb_security_key_size - decode_data.sgnb_security_key_unused == 32);
    //   REQUIRE(decode_data.eutran_identifier_size*8 - decode_data.eutran_identifier_unused == 28);
    //   REQUIRE(encode_data.get_list()->size() == decode_data.get_list()->size());
    //   REQUIRE(encode_data.erab_list[0].erab_id == decode_data.erab_list[0].erab_id);
    //   REQUIRE(encode_data.erab_list[0].drb_id == decode_data.erab_list[0].drb_id);
    //   REQUIRE(encode_data.erab_list[0].pdcp_at_sgnb == decode_data.erab_list[0].pdcp_at_sgnb);
      
    //   REQUIRE(encode_data.menb_ue_x2ap_id == decode_data.menb_ue_x2ap_id);
    //   ASN_STRUCT_FREE(asn_DEF_X2N_X2AP_PDU, x2ap_pdu_obj);
    // }
    
  }

  SECTION("Encoding response, missing erab"){

    encode_data.menb_ue_x2ap_id = 12;
    encode_data.sgnb_ue_x2ap_id = 10;

    encode_data.bit_rate_max_up = 1000;
    encode_data.bit_rate_max_dn = 2000;

    encode_data.encryption_algs = enc_alg;
    encode_data.encryption_algs_size = strlen((char *)enc_alg);
    encode_data.encryption_algs_unused = 0;

    encode_data.integrity_protection_algs = integrity_alg;
    encode_data.integrity_protection_algs_size = strlen((char *)integrity_alg);
    encode_data.integrity_protection_algs_unused = encode_data.integrity_protection_algs_size/8;
    REQUIRE(encode_data.integrity_protection_algs_size - encode_data.integrity_protection_algs_unused == 2);


    encode_data.sgnb_security_key = sgnb_security_key;
    encode_data.sgnb_security_key_size = strlen((char *)sgnb_security_key);
    encode_data.sgnb_security_key_unused = 0;
    REQUIRE(encode_data.sgnb_security_key_size - encode_data.sgnb_security_key_unused == 32);


    encode_data.menb_sgnb_container = mesgnb_container;
    encode_data.menb_sgnb_container_size = strlen((char *)mesgnb_container);

    encode_data.plmn_identity = plmn;
    encode_data.plmn_identity_size = strlen((char *)plmn);

    encode_data.eutran_identifier = eutran;
    encode_data.eutran_identifier_size = strlen((char *)eutran);
    encode_data.eutran_identifier_unused = 4;

    encode_data.subscriber_profile_id = 34;
    res = resp_out.encode_sgnb_addition_response(buf, &buf_size, encode_data, true);
    REQUIRE(res == false);
  }


  //   // Checking Sgnb ACK Encoding
  //   REQUIRE(response == true);
  //   res = resp_out.encode_sgnb_addition_response(data_buf, &data_size,decode_data,response);
  //   REQUIRE(res == true);
	  
  //   // FILE *pfile;
  //   // pfile = fopen("sgnb_addition_ack.per", "w");
  //   // fwrite(data_buf, sizeof(char), data_size, pfile);
  //   // fclose(pfile);
  // }

  SECTION("Response Reject Encoding, valid"){
    //checking Sgnb Reject Encoding
    encode_data.menb_ue_x2ap_id = 12;
    encode_data.sgnb_ue_x2ap_id = 10;
    encode_data.cause = 1;
    encode_data.cause_desc = 1;	  
    res = resp_out.encode_sgnb_addition_response(data_buf, &data_size, encode_data, false);
    std::cout <<"Reason = " << resp_out.get_error() << std::endl;
    REQUIRE(res==true);
  }
  
  SECTION("Response Reject Encoding, invalid data"){
    //checking Sgnb Reject Encoding
    encode_data.menb_ue_x2ap_id = 1200;
    encode_data.sgnb_ue_x2ap_id = 10;
    encode_data.cause = 1;
    encode_data.cause_desc = 1;	  
    res = resp_out.encode_sgnb_addition_response(data_buf, &data_size,decode_data, false);
    REQUIRE(res==false);
  }

	


}



