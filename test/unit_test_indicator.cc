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
/* Author : Ashwin Sridharan
   Date    : Feb 2019
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <Indication_builder.hpp>

TEST_CASE("E2AP PDUs", "RIC Indication PDU"){

  int req_id = 100;
  int req_seq_no = 200;
  int func_id = 999;
  int action_id = 18;
  int sn = 20;
  int type = 1;
  std::string header = "Indication Header";
  std::string message = "addition request";
  std::string call_process_id = "message_23";
  
  
  E2AP_PDU_builder e2ap_pdu_builder(1);
  IndicationBuilder ric_indication(e2ap_pdu_builder.get_e2ap_pdu());
  IndicationHelper he_indication;
  he_indication.set_request(req_id, req_seq_no);
  he_indication.set_function_id(func_id);
  he_indication.set_action_id(action_id);
  he_indication.set_indication_sn(sn);
  he_indication.set_indication_type(type);

  he_indication.set_indication_header(header.c_str(), header.length());
  he_indication.set_indication_message(message.c_str(), message.length());
  he_indication.set_call_process_id(call_process_id.c_str(), call_process_id.length());

  ric_indication.setRICrequestID(he_indication);
  ric_indication.setRANfunctionID(he_indication);
  ric_indication.setRICactionID(he_indication);
  ric_indication.setRICindicationSN(he_indication);
  ric_indication.setRICindicationType(he_indication);
  ric_indication.setRICindicationHeader(he_indication);
  ric_indication.setRICindicationMessage(he_indication);
  ric_indication.setRICcallProcessID(he_indication);
  
  unsigned char buffer[1024];
  unsigned int pdu_length;
  
  bool res = e2ap_pdu_builder.encode_pdu(buffer, 1024, &pdu_length);
  REQUIRE (res == true);

  res = e2ap_pdu_builder.decode_pdu(buffer, pdu_length);
  REQUIRE (res == true);

  IndicationHelper he_indication2;
  ric_indication.reset(e2ap_pdu_builder.get_e2ap_pdu());
  ric_indication.get_fields(he_indication2);
  REQUIRE(he_indication2.get_request_id() == req_id);
  REQUIRE(he_indication2.get_req_seq() == req_seq_no);
  REQUIRE(he_indication2.get_function_id() == func_id);
  REQUIRE(he_indication2.get_indication_sn() == sn);
  REQUIRE(he_indication2.get_action_id() == action_id);
  REQUIRE(he_indication2.get_indication_type() == type);

  std::string header2(static_cast<const char *>(he_indication2.get_indication_header()), he_indication2.get_indication_header_size());
  REQUIRE(header2 == header);
  
}

