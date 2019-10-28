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

#include <cstring>
#include <assert.h>

#include "e2sm.hpp"

TEST_CASE("E2SM Event Trigger Test Cases", "Encoding/decoding"){
  

  unsigned char buf[128];
  size_t buf_size = 128;
  
  bool res;
  asn_dec_rval_t retval;

  e2sm_event_trigger_helper  trigger_data;
  e2sm_event_trigger_helper trigger_recv;

  e2sm_event_trigger mem_check; // just null declaration to test memory
  
  e2sm_event_trigger e2sm_event_trigger;
  E2N_E2SM_gNB_X2_eventTriggerDefinition_t *event; // used for decoding


  SECTION("Encoding event trigger, positive and negative cases"){

    // fill in data
    trigger_data.egNB_id = "hello world";
    trigger_data.plmn_id = "something new";
    trigger_data.egNB_id_type = 2;
    trigger_data.interface_direction = 1;
    
    trigger_data.procedure_code = 27;
    trigger_data.message_type = 0;
    
    //Add multiple interface protocol ie items to cover all test cases ..

    trigger_data.add_protocol_ie_item(1, 1, 1, 1);
    trigger_data.add_protocol_ie_item(2, 2, 2, 2);
    trigger_data.add_protocol_ie_item(3, 3, 3, 0);
    trigger_data.add_protocol_ie_item(4, 3, 4, std::string("Something new"));
    trigger_data.add_protocol_ie_item(4, 3, 5, std::string("Something old"));
  
    res = e2sm_event_trigger.encode_event_trigger(&buf[0], &buf_size, trigger_data);
    REQUIRE(res == true);
    
    event = 0;
    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2SM_gNB_X2_eventTriggerDefinition, (void**)&(event), &buf[0], buf_size);
    
    REQUIRE(retval.code == RC_OK);
    res =  e2sm_event_trigger.get_fields(event, trigger_recv);
    REQUIRE(res == true);
    REQUIRE(trigger_recv.interface_direction == trigger_data.interface_direction);
    REQUIRE(trigger_recv.procedure_code == trigger_data.procedure_code);
    REQUIRE(trigger_recv.message_type == trigger_data.message_type);
    REQUIRE(trigger_recv.plmn_id == "som");
    REQUIRE(trigger_recv.get_list()->size() == trigger_data.get_list()->size());
    ASN_STRUCT_FREE(asn_DEF_E2N_E2SM_gNB_X2_eventTriggerDefinition, event);

    // buffer size too low
    buf_size = 2;
    res = e2sm_event_trigger.encode_event_trigger(&buf[0], &buf_size, trigger_data);
    REQUIRE(res == false);

    // invalid constraints ...
    trigger_data.interface_direction = 10;
    buf_size = 128;
    res = e2sm_event_trigger.encode_event_trigger(&buf[0], &buf_size, trigger_data);
    REQUIRE(res == false);

  }
  

}


TEST_CASE("E2SM Indication Header", "e2sm_header"){
  
  unsigned char buf_header[128];
  unsigned char buf_msg[1024];
  size_t header_size = 128;
  size_t msg_size = 1024;
  
  std::string test_x2ap_message = "This is X2AP !";

  bool res;
  asn_dec_rval_t retval;
  e2sm_indication e2sm_builder;
  e2sm_indication e2sm_receiver;
  
  SECTION("Indication Header encoding/decoding"){
    e2sm_header_helper header_tx;
  
    e2sm_header_helper header_re;  
  
    header_tx.egNB_id="hello";
    header_tx.plmn_id="there";
    header_tx.interface_direction = 1;
    header_tx.egNB_id_type = 2;
    
    // Encode the message
    res = e2sm_builder.encode_indication_header(&buf_header[0], &header_size, header_tx);
    REQUIRE(res == true);
    
  
    E2N_E2SM_gNB_X2_indicationHeader_t *header = 0; // used for decoding
    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2SM_gNB_X2_indicationHeader, (void**)&(header), &buf_header[0], header_size);

    REQUIRE(retval.code == RC_OK);
    
    res = e2sm_receiver.get_header_fields(header, header_re);
    REQUIRE(res == true);
      
    REQUIRE(header_re.plmn_id == "the");
    ASN_STRUCT_FREE(asn_DEF_E2N_E2SM_gNB_X2_indicationHeader, header);

    // buffer size too low
    header_size = 2;
    res = e2sm_builder.encode_indication_header(&buf_header[0], &header_size, header_tx);
    REQUIRE(res == false);

    // invalid constraints ...
    header_tx.interface_direction = 10;
    header_size = 128;
    res = e2sm_builder.encode_indication_header(&buf_header[0], &header_size, header_tx);
    REQUIRE(res == false);
    
    
  }

  SECTION("Indication Message encoding/decoding"){
    
    e2sm_message_helper message_tx;
    e2sm_message_helper message_re;

    message_tx.x2ap_pdu = (unsigned char *)calloc(1024, sizeof(unsigned char));
    memcpy(message_tx.x2ap_pdu, test_x2ap_message.c_str(), test_x2ap_message.length());
    message_tx.x2ap_pdu_size = test_x2ap_message.length();
    

    res = e2sm_builder.encode_indication_message(&buf_msg[0], &msg_size, message_tx);
    REQUIRE(res == true);
    
    E2N_E2SM_gNB_X2_indicationMessage_t *message = 0; // used for decoding

    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2SM_gNB_X2_indicationMessage, (void**)&(message), &buf_msg[0], msg_size);
  
    REQUIRE(retval.code == RC_OK);
    res = e2sm_receiver.get_message_fields(message, message_re);
    REQUIRE(res == true);
    
    REQUIRE(message_re.x2ap_pdu_size == message_tx.x2ap_pdu_size);
    ASN_STRUCT_FREE(asn_DEF_E2N_E2SM_gNB_X2_indicationMessage, message);
    free(message_tx.x2ap_pdu);
  }
  
 
}



TEST_CASE("E2SM Encoding/Decoding", "Control"){
  
  unsigned char buf_header[128];
  unsigned char buf_msg[1024];
  size_t header_size = 128;
  size_t msg_size = 1024;

  e2sm_control e2sm_builder;
  e2sm_header_helper header_tx;
  e2sm_message_helper message_tx;
  
 
  e2sm_control e2sm_receiver;
  e2sm_header_helper header_re;
  e2sm_message_helper message_re;


  bool res;
  asn_dec_rval_t retval;

  E2N_E2SM_gNB_X2_controlHeader_t *header; // used for decoding
  E2N_E2SM_gNB_X2_controlMessage_t *message; // used for decoding

  SECTION("Control encoding/decoding header"){

    header_tx.egNB_id="hello";
    header_tx.plmn_id="there";
    header_tx.interface_direction = 1;
    header_tx.egNB_id_type = 2;

    
    res = e2sm_builder.encode_control_header(&buf_header[0], &header_size, header_tx);
    REQUIRE(res == true);
    
    // =================================
    // Decode the message
    header = 0;
    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2SM_gNB_X2_controlHeader, (void**)&(header), &buf_header[0], header_size);
    
    REQUIRE(retval.code == RC_OK);
    
    res = e2sm_receiver.get_header_fields(header, header_re);
    REQUIRE(res == true);
    REQUIRE(header_re.plmn_id == "the");
    REQUIRE(header_re.interface_direction == header_tx.interface_direction);
    
    // free the struct since we have already extracted all references
    ASN_STRUCT_FREE(asn_DEF_E2N_E2SM_gNB_X2_controlHeader, header);
  }

  SECTION("Control message encoding/decoding"){

    std::string test_x2ap_message = "This is X2AP !";
    
    message_tx.x2ap_pdu = (unsigned char *)calloc(1024, sizeof(unsigned char));
    memcpy(message_tx.x2ap_pdu, test_x2ap_message.c_str(), test_x2ap_message.length());
    message_tx.x2ap_pdu_size = test_x2ap_message.length();

    res = e2sm_builder.encode_control_message(&buf_msg[0], &msg_size, message_tx);
    REQUIRE(res == true);


    message = 0;
    retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2SM_gNB_X2_controlMessage, (void**)&(message), &buf_msg[0], msg_size);
   
    REQUIRE(retval.code == RC_OK);
    res = e2sm_receiver.get_message_fields(message, message_re);
    REQUIRE(res == true);
    REQUIRE(message_re.x2ap_pdu_size == message_tx.x2ap_pdu_size);
    ASN_STRUCT_FREE(asn_DEF_E2N_E2SM_gNB_X2_controlMessage, message);
    free(message_tx.x2ap_pdu);
  }
  
}
