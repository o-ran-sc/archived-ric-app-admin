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
#include <iostream>
#include <subscription_request.hpp>
#include <subscription_response.hpp>
#include <subscription_delete_request.hpp>
#include <subscription_delete_response.hpp>
#include <chrono>

#define BUFFER_SIZE 1024


TEST_CASE("E2AP PDU Subscription", "RIC Susbcription/Deletion Requests"){

  subscription_helper  din;
  subscription_helper  dout;
  
  subscription_request sub_req;
  subscription_request sub_recv;
  
  subscription_delete sub_del_req;
  subscription_delete sub_del_recv;
  
  unsigned char buf[BUFFER_SIZE];
  size_t buf_size = BUFFER_SIZE;
  bool res;

  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST SUBSCRIPTION");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);


  //=========================================================
  SECTION ("Verifiy request PDU generation"){

    din.clear();
    dout.clear();

    
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;
    std::string event_def = "This is a test";

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    din.set_event_def(event_def.c_str(), event_def.length());

    // fail if no actions specified
    res = sub_req.encode_e2ap_subscription(&buf[0], &buf_size, din);
    REQUIRE(res == false);
    
    // variable number of actions
    int num_actions = 17;
    srand(9384938);

    for(int num_act = 1; num_act < num_actions; num_act ++){
      
      din.clear();
      for(int i = 0; i < num_act; i++){
	int type = rand() % 3;
	din.add_action(i, type);
      }

      buf_size = BUFFER_SIZE;
      res = sub_req.encode_e2ap_subscription(&buf[0], &buf_size, din);
      if(!res)
	std::cerr <<"Error = " << sub_req.get_error() << " with actions = " << din.get_list()->size() << std::endl;
      REQUIRE(res == true);
    }

    // Adding another action should result in
    // encoding error since according to specs v-0.31
    // maxRICactionID == 16
    din.add_action(26, 0);
    buf_size = BUFFER_SIZE;
    res = sub_req.encode_e2ap_subscription(&buf[0], &buf_size, din);
    REQUIRE(res == false);
    
  //=========================================================
    
  }

  SECTION ("Verify request PDU decoding"){

    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;
    std::string event_def = "This is a test";
    

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    din.set_event_def(event_def.c_str(), event_def.length());

    srand(9384938);

    int num_actions = 17;
    for(int num_act = 1; num_act < num_actions; num_act ++){

      din.clear();
      dout.clear();
      
      for(int i = 0; i < num_act; i++){
	int type = rand() % 3;
	din.add_action(i, type);
      }

      buf_size = BUFFER_SIZE;      
      res = sub_req.encode_e2ap_subscription(&buf[0], &buf_size,  din);    
      REQUIRE(res == true);

      E2N_E2AP_PDU_t *e2ap_recv = 0;
      asn_dec_rval_t retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), &buf[0], buf_size);
      REQUIRE(retval.code == RC_OK);

      sub_recv.get_fields(e2ap_recv->choice.initiatingMessage, dout);
      REQUIRE(dout.get_request_id() == din.get_request_id());
      REQUIRE(dout.get_req_seq() == din.get_req_seq());
      REQUIRE(dout.get_function_id() == din.get_function_id());
      REQUIRE(dout.get_list()->size() == din.get_list()->size());
      std::string dout_string((const char *)dout.get_event_def(), dout.get_event_def_size());
      std::string din_string((const char *)din.get_event_def(), din.get_event_def_size());
      REQUIRE(dout_string == din_string);
      
      ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
    }

  }

  SECTION ("Verify Error on illegal values in request"){
    din.clear();
    dout.clear();

     //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 6000;
    std::string event_def = "This is a test";
    

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    din.set_event_def(event_def.c_str(), event_def.length());
    
    din.add_action(0, 0);
    din.add_action(11, 1);
    res = sub_req.encode_e2ap_subscription(&buf[0], &buf_size,  din);
    REQUIRE(res == false);
  }

  SECTION("Verify Error on illegal input in request"){
    res = sub_req.get_fields(NULL, dout);
    REQUIRE(res == false);
  }

  

  SECTION ("Verifiy delete request PDU generation"){
    
    //Random Data  for request
    din.clear();
    dout.clear();    
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;
    
    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    
    res = sub_del_req.encode_e2ap_subscription(&buf[0], &buf_size, din);
    REQUIRE(res == true);
  
  }

  SECTION ("Verify delete request PDU decoding"){
    din.clear();
    dout.clear();
    
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    
    res = sub_del_req.encode_e2ap_subscription(&buf[0], &buf_size, din);
    
    REQUIRE(res == true);

    E2N_E2AP_PDU_t *e2ap_recv = 0;
    asn_dec_rval_t retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), &buf[0], buf_size);
    REQUIRE(retval.code == RC_OK);

    sub_del_recv.get_fields(e2ap_recv->choice.initiatingMessage, dout);
    REQUIRE(dout.get_request_id() == din.get_request_id());
    REQUIRE(dout.get_req_seq() == din.get_req_seq());
    REQUIRE(dout.get_function_id() == din.get_function_id());

    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
  }

  SECTION ("Verify Error on illegal values in delete request"){
    din.clear();
    dout.clear();

     //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 6000;
  
    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    
    res = sub_del_req.encode_e2ap_subscription(&buf[0], &buf_size, din);
    
    REQUIRE(res == false);
  }
  
  SECTION("Verify Error on illegal input in delete request"){
    res = sub_del_req.get_fields(NULL, dout);
    REQUIRE(res == false);
  }

}

TEST_CASE("E2AP PDU Subscription Response", "RIC Subscription/Deletion Responses"){
  subscription_response_helper  din;
  subscription_response_helper  dout;
  
  subscription_response sub_resp;
  subscription_response sub_recv;
  
  subscription_delete_response sub_del_resp;
  subscription_delete_response sub_del_recv;

  unsigned char buf[BUFFER_SIZE];
  size_t buf_size = BUFFER_SIZE;
  bool res;
  
  //=========================================================
  SECTION ("Verifiy successful/unsuccessful response PDU generation"){

    din.clear();
    dout.clear();
    
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;
    
    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);

    
    // variable number of actions
    int num_actions = 25;
    din.clear();
    srand(9384938);
    for(int num_success = 0; num_success <= num_actions; num_success ++){
      for(int num_failures = 0; num_failures <= num_actions; num_failures++){
	din.clear();
	for(int i = 0; i < num_success; i++){
	  din.add_action(i);
	}
	for(int j = 0; j < num_failures; j++){
	  int cause = 1;
	  int sub_cause = 1;
	  din.add_action(j, cause, sub_cause);
	}

	//std::cout <<"Successful admitted actions = " << num_success << " Unsuccessful admitted actions = " << num_failures << std::endl;
	
	buf_size = BUFFER_SIZE;
	res = sub_resp.encode_e2ap_subscription_response(&buf[0], &buf_size, din, true);
	if(num_success == 0 || num_success > 16 || num_failures > 16){
	  REQUIRE(res == false);
	}
	else{
	  REQUIRE(res == true);
	}
	
	buf_size = BUFFER_SIZE;
	res = sub_resp.encode_e2ap_subscription_response(&buf[0], &buf_size, din, false);
	if(num_failures == 0 || num_failures > 16){
	  REQUIRE(res == false);
	}
	else{
	  REQUIRE(res == true);
	}
      }
    }
  }
    
  
    
//   //=========================================================
    


  SECTION ("Verify successful response PDU decoding"){
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    

    // variable number of actions
    int num_actions = 16;
    din.clear();
    srand(9384938);
    for(int num_success = 1; num_success <= num_actions; num_success ++){
      for(int num_failures = 0; num_failures <= num_actions; num_failures++){
	din.clear();
	dout.clear();
	for(int i = 0; i < num_success; i++){
	  din.add_action(i);
	}
	for(int j = 0; j < num_failures; j++){
	  int cause = 1;
	  int sub_cause = 1;
	  din.add_action(j, cause, sub_cause);
	}

	
	buf_size = BUFFER_SIZE;
	res = sub_resp.encode_e2ap_subscription_response(&buf[0], &buf_size, din, true);
	REQUIRE(res == true);

	E2N_E2AP_PDU_t *e2ap_recv = 0;
	asn_dec_rval_t retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), &buf[0], buf_size);
	REQUIRE(retval.code == RC_OK);

	sub_recv.get_fields(e2ap_recv->choice.successfulOutcome, dout);
	REQUIRE(dout.get_request_id() == din.get_request_id());
	REQUIRE(dout.get_req_seq() == din.get_req_seq());
	REQUIRE(dout.get_function_id() == din.get_function_id());
	REQUIRE(dout.get_admitted_list()->size() == din.get_admitted_list()->size());
	REQUIRE(dout.get_not_admitted_list()->size() == din.get_not_admitted_list()->size());

	ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
      }
    }
  }

  SECTION ("Verify unsuccessful response PDU decoding"){
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    

    // variable number of actions
    int num_actions = 16;
    din.clear();
    srand(9384938);
    for(int num_failures = 1; num_failures <= num_actions; num_failures++){
      din.clear();
      dout.clear();

      for(int j = 0; j < num_failures; j++){
	int cause = 1;
	int sub_cause = 1;
	din.add_action(j, cause, sub_cause);
      }
      
	
      buf_size = BUFFER_SIZE;
      res = sub_resp.encode_e2ap_subscription_response(&buf[0], &buf_size, din, false);
      REQUIRE(res == true);
      
      E2N_E2AP_PDU_t *e2ap_recv = 0;
      asn_dec_rval_t retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), &buf[0], buf_size);
      REQUIRE(retval.code == RC_OK);
      
      sub_recv.get_fields(e2ap_recv->choice.unsuccessfulOutcome, dout);
      REQUIRE(dout.get_request_id() == din.get_request_id());
      REQUIRE(dout.get_req_seq() == din.get_req_seq());
      REQUIRE(dout.get_function_id() == din.get_function_id());
      REQUIRE(dout.get_not_admitted_list()->size() == din.get_not_admitted_list()->size());

      ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
    }
    
  }
  
  SECTION ("Verify Error on illegal values in response"){
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 6000;
    
    
    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    
    din.add_action(0);
    din.add_action(11, 2, 1);

    buf_size = BUFFER_SIZE;
    res = sub_resp.encode_e2ap_subscription_response(&buf[0], &buf_size, din, true);
    REQUIRE(res == false);

    buf_size = BUFFER_SIZE;
    res = sub_resp.encode_e2ap_subscription_response(&buf[0], &buf_size, din, false);
    REQUIRE(res == false);

    //SuccessfulOutcome_t * success_msg = NULL;
    //UnsuccessfulOutcome_t * unsuccess_msg = NULL;
    //REQUIRE_THROWS(sub_resp.get_fields(success_msg, din));
    //REQUIRE_THROWS(sub_resp.get_fields(unsuccess_msg, din));
  }


  SECTION ("Verifiy successful/unsuccessful  delete response PDU generation"){
    
    //Random Data  for request
    din.clear();
    dout.clear();
    
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;
    

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
      
    res = sub_del_resp.encode_e2ap_subscription_delete_response(&buf[0], &buf_size, din, true);    
    REQUIRE(res == true);
    
    res = sub_del_resp.encode_e2ap_subscription_delete_response(&buf[0], &buf_size, din, false);    
    REQUIRE(res == true);
    
   }

  SECTION ("Verify successful delete response PDU decoding"){
    din.clear();
    dout.clear();
    
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    res = sub_del_recv.encode_e2ap_subscription_delete_response(&buf[0], &buf_size,  din, true);
    REQUIRE(res == true);

    E2N_E2AP_PDU_t *e2ap_recv = 0;
    asn_dec_rval_t retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), &buf[0], buf_size);
    REQUIRE(retval.code == RC_OK);

    sub_recv.get_fields(e2ap_recv->choice.successfulOutcome, dout);
    REQUIRE(dout.get_request_id() == din.get_request_id());
    REQUIRE(dout.get_req_seq() == din.get_req_seq());
    REQUIRE(dout.get_function_id() == din.get_function_id());

    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
  }
  
  SECTION("Verify unsuccessful delete response PDU decoding"){
    
    din.clear();
    dout.clear();
    
    //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 0;

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
      
    res = sub_del_recv.encode_e2ap_subscription_delete_response(&buf[0], &buf_size, din, false);
    REQUIRE(res == true);

    E2N_E2AP_PDU_t *e2ap_recv = 0;
    asn_dec_rval_t retval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2N_E2AP_PDU, (void**)&(e2ap_recv), &buf[0], buf_size);
    REQUIRE(retval.code == RC_OK);

    sub_recv.get_fields(e2ap_recv->choice.unsuccessfulOutcome, dout);
    REQUIRE(dout.get_request_id() == din.get_request_id());
    REQUIRE(dout.get_req_seq() == din.get_req_seq());
    REQUIRE(dout.get_function_id() == din.get_function_id());
    ASN_STRUCT_FREE(asn_DEF_E2N_E2AP_PDU, e2ap_recv);
  }
  
  SECTION ("Verify Error on illegal values in delete response"){
     //Random Data  for request
    int request_id = 2;
    int req_seq_no = 1;
    int function_id = 6000;
    

    din.set_request(request_id, req_seq_no);
    din.set_function_id(function_id);
    
    din.add_action(0);
    din.add_action(11, 2, 1);
    res = sub_del_resp.encode_e2ap_subscription_delete_response(&buf[0], &buf_size, din, true);
    
    REQUIRE(res == false);
  }


}


