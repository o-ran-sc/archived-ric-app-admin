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
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <e2ap_indication.hpp>
#include <unistd.h>
#include <inttypes.h>

#define BUFFER_SIZE 512

TEST_CASE("E2AP Indication", "Encoding/Decoding"){
  
  ric_indication_helper dinput ;
  ric_indication_helper dout;


  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST E2AP INDICATION");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);

  unsigned char buf_header[BUFFER_SIZE];
  unsigned char buf_msg[BUFFER_SIZE];
  unsigned char buf_callproc[BUFFER_SIZE];
  ric_indication test_check;
  
  SECTION("Incorrect E2AP Indication PDU"){
    size_t data_size = 512;
    unsigned char data[data_size];
    for(int i = 0; i < 510; i++){
      data[i] = 'z';
    }
 
    E2N_E2AP_PDU_t * e2ap_recv = 0;
    asn_dec_rval_t dec_res  = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), data, data_size);
    REQUIRE(dec_res.code != RC_OK);
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv); 
  }
  
  SECTION("Verify E2AP Indication Encoding Successful"){
    
    dinput.action_id = 100;
    dinput.func_id = 10;
    dinput.indication_sn = 100;
    dinput.indication_type = 1;
    dinput.req_id = 6;
    dinput.req_seq_no = 11;

    strcpy((char *)buf_header, "X2AP Header");
    strcpy((char *)buf_msg, "X2AP_Message");
    strcpy((char *)buf_callproc, "Call Process ID=10");
    
    dinput.indication_header = buf_header;
    dinput.indication_header_size = strlen((const char *)buf_header);
  
    dinput.indication_msg = buf_msg;
    dinput.indication_msg_size = strlen((const char *)buf_msg);

    dinput.call_process_id = buf_callproc;
    dinput.call_process_id_size = strlen((const char *)buf_callproc);
    
    /* encoding */
    size_t data_size = 512;
    unsigned char data[data_size];
    
    ric_indication indication_pdu;
    bool res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
    REQUIRE(res == true);
  }


  SECTION("Verify constraint checks"){

    dinput.action_id = 100;
    dinput.func_id = 10;
    dinput.indication_sn = 100;
    dinput.indication_type = 100;
    dinput.req_id = 6;
    dinput.req_seq_no = 11;

    strcpy((char *)buf_header, "X2AP Header");
    strcpy((char *)buf_msg, "X2AP_Message");
    
    dinput.indication_header = buf_header;
    dinput.indication_header_size = strlen((const char *)buf_header);
  
    dinput.indication_msg = buf_msg;
    dinput.indication_msg_size = strlen((const char *)buf_msg);
    
    
    /* encoding */
    size_t data_size = 512;
    unsigned char data[data_size];
    
    ric_indication indication_pdu;
    bool res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
    REQUIRE(res == false);


  }
  SECTION("Verify E2AP Decoding Successful"){

    dinput.action_id = 100;
    dinput.func_id = 10;
    dinput.indication_sn = 100;
    dinput.indication_type = 1;
    dinput.req_id = 6;
    dinput.req_seq_no = 11;


    strcpy((char *)buf_header, "X2AP Header");
    strcpy((char *)buf_msg, "X2AP_Message");
    
    dinput.indication_header = buf_header;
    dinput.indication_header_size = strlen((const char *)buf_header);
  
    dinput.indication_msg = buf_msg;
    dinput.indication_msg_size = strlen((const char *)buf_msg);
    
    
    /* encoding */
    size_t data_size = 512;
    unsigned char data[data_size];
    
    ric_indication indication_pdu;
    bool res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
    
    E2N_E2AP_PDU_t * e2ap_recv = 0;
    asn_dec_rval_t dec_res  = asn_decode(0,ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), data, data_size);
    REQUIRE(dec_res.code == RC_OK);
    
    res = indication_pdu.get_fields(e2ap_recv->choice.initiatingMessage, dout);
    REQUIRE(res == true);

    std::string din_string((const char *)dinput.indication_header, dinput.indication_header_size);
    std::string dout_string((const char*)dout.indication_header, dout.indication_header_size);
    REQUIRE(din_string == dout_string);

    din_string.assign((const char *)dinput.indication_msg, dinput.indication_msg_size);
    dout_string.assign((const char*)dout.indication_msg, dout.indication_msg_size);
    REQUIRE(din_string == dout_string);

    din_string.assign((const char *)dinput.call_process_id, dinput.call_process_id_size);
    dout_string.assign((const char*)dout.call_process_id, dout.call_process_id_size);
    REQUIRE(din_string == dout_string);

    res = indication_pdu.get_fields(NULL, dout);
    REQUIRE(res == false);
    
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv); 
  }
    
}


