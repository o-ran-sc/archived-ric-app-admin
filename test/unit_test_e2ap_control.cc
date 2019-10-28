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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <e2ap_control.hpp>
#include <e2ap_control_response.hpp>
#include <unistd.h>
#include <inttypes.h>


#define BUFFER_SIZE 512

TEST_CASE("E2AP Control Request", "Encoding/Decoding"){

  ric_control_helper dinput ;
  ric_control_helper  dout;
  ric_control_request control_request_pdu;
  
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST E2AP INDICATION");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_ERR);
  mdclog_attr_destroy(attr);

  unsigned char buf_header[BUFFER_SIZE];
  unsigned char buf_msg[BUFFER_SIZE];
  
  SECTION("Verify E2AP Control Encoding/Decoding Successful"){
    
    dinput.func_id = 10;
    dinput.req_id = 6;
    dinput.req_seq_no = 11;
    dinput.control_ack = 1;
  
    strcpy((char *)buf_header, "hello world");
    strcpy((char *)buf_msg, "something out there");
    unsigned char buf_proc_id[4] = "25";
  
    dinput.control_header = buf_header;
    dinput.control_header_size = strlen((const char *)buf_header);
    
    dinput.control_msg = buf_msg;
    dinput.control_msg_size = strlen((const char *)buf_msg);
    
    dinput.call_process_id = buf_proc_id;
    dinput.call_process_id_size = strlen((const char *)buf_proc_id);
    
    /* encoding */
    size_t data_size = 4096;
    unsigned char	data[data_size];
  
    bool res = control_request_pdu.encode_e2ap_control_request(&data[0], &data_size, dinput);
    REQUIRE(res == true);

    E2N_E2AP_PDU_t * e2ap_recv = 0;
    asn_dec_rval_t dec_res  = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), data, data_size);
    REQUIRE(dec_res.code == RC_OK);
    
    res = control_request_pdu.get_fields(e2ap_recv->choice.initiatingMessage, dout);
    REQUIRE (res == true);

    REQUIRE(dinput.func_id == dout.func_id);
    REQUIRE(dinput.req_id == dout.req_id);
    REQUIRE(dinput.req_seq_no == dout.req_seq_no);
    REQUIRE(dinput.control_ack == dout.control_ack);

    std::string din_string;
    std::string dout_string;

    din_string.assign((char *)dinput.control_header, dout.control_header_size);
    dout_string.assign((char *)dout.control_header, dout.control_header_size);
    REQUIRE(din_string == dout_string);
   
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv); 
    
  }

  SECTION("Negative Cases"){
    bool res = control_request_pdu.get_fields(NULL, dout);
    REQUIRE(res == false);
  }
}


TEST_CASE("E2AP Control Response", "Encoding/Decoding"){

  ric_control_helper dinput ;
  ric_control_helper  dout;
  ric_control_response control_response_pdu;
  
  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST E2AP INDICATION");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);

  ric_control_response test_check;

  SECTION("Verify E2AP Control Response Successful Outcome Encoding/Decoding"){
    
    dinput.func_id = 10;
    dinput.req_id = 6;
    dinput.req_seq_no = 11;
    dinput.control_status = 1;
    dinput.cause = 1;
    dinput.sub_cause = 2;


    ric_control_helper dout;
    unsigned char buf_proc_id[4] = "25";

    dinput.call_process_id = buf_proc_id;
    dinput.call_process_id_size = strlen((const char *)buf_proc_id);
    
    /* encoding */
    size_t data_size = 4096;
    unsigned char data[data_size];
    
    ric_control_response control_response_pdu;

    bool res = control_response_pdu.encode_e2ap_control_response(&data[0], &data_size, dinput, true);

    REQUIRE(res == true);
    
    /* decoding */
    E2N_E2AP_PDU_t * e2ap_recv = 0;
    asn_dec_rval_t dec_res  = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), data, data_size);
    REQUIRE(dec_res.code == RC_OK);
    REQUIRE(e2ap_recv->present ==  E2N_E2AP_PDU_PR_successfulOutcome);
    res =   control_response_pdu.get_fields(e2ap_recv->choice.successfulOutcome, dout);
    REQUIRE(res == true);
    REQUIRE(dout.func_id == dinput.func_id);
    REQUIRE(dout.req_id == dinput.req_id);
    REQUIRE(dout.control_status == dinput.control_status);
    
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
  }

  
  SECTION("Verify E2AP Control Response UnSuccessful Outcome Encoding/Decoding"){
    
    dinput.func_id = 10;
    dinput.req_id = 6;
    dinput.req_seq_no = 11;
    dinput.control_status = 1;
    dinput.cause = 1;
    dinput.sub_cause = 2;


    ric_control_helper dout;
    unsigned char buf_proc_id[4] = "25";

    dinput.call_process_id = buf_proc_id;
    dinput.call_process_id_size = strlen((const char *)buf_proc_id);
    
    /* encoding */
    size_t data_size = 4096;
    unsigned char data[data_size];
    
    ric_control_response control_response_pdu;
    bool res = control_response_pdu.encode_e2ap_control_response(&data[0], &data_size, dinput, false);
    REQUIRE(res == true);
    
    /* decoding */
    E2N_E2AP_PDU_t * e2ap_recv = 0;
    asn_dec_rval_t dec_res  = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), data, data_size);
    REQUIRE(dec_res.code == RC_OK);
    REQUIRE(e2ap_recv->present ==  E2N_E2AP_PDU_PR_unsuccessfulOutcome);
    res =   control_response_pdu.get_fields(e2ap_recv->choice.unsuccessfulOutcome, dout);

    REQUIRE(res == true);
    REQUIRE(dout.func_id == dinput.func_id);
    REQUIRE(dout.req_id == dinput.req_id);
    REQUIRE(dout.control_status == dinput.control_status);
    REQUIRE(dout.cause == dinput.cause);
    REQUIRE(dout.sub_cause == dinput.sub_cause);
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
  }

}
