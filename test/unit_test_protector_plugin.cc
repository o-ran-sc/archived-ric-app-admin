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
   Date   : Nov 2019
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <NetworkProtector.h>


TEST_CASE("Protector Plugin", "Test Input types"){


  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST PROTECTOR PLUGIN ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_ERR);
  mdclog_attr_destroy(attr);
  FILE *pfile;
 
  unsigned char scratch_buf[512];
  size_t scratch_buf_size = 512;

  // Load buffers with different types of X2AP PDUs to test
  // behaviour if valid/invalid
  //========================================================
  // valid x2 sgnb addition request  
  unsigned char x2ap_buf[256];
  size_t x2ap_buf_size ;
  pfile = fopen("test-data/X2AP-PDU-SgNBAdditionRequest.per", "r");
  REQUIRE(pfile != NULL);
  x2ap_buf_size = fread((char *)x2ap_buf, sizeof(char), 256, pfile);
  fclose(pfile);


  // incorrect x2ap pdu 
  srand(10902390);
  unsigned char incorrect_x2ap_buf[8192];
  size_t incorrect_x2ap_buf_size = 8192;
  for(int i = 0; i < 8192; i++){
    incorrect_x2ap_buf[i] = rand()%256;
  }

  // valid x2 but not an initiating message
  unsigned char x2ap_sgnb_ack[512];
  size_t x2ap_sgnb_ack_size;
  pfile = fopen("test-data/X2AP-PDU-SgNBAdditionAck.per", "r");
  REQUIRE(pfile != NULL);
  x2ap_sgnb_ack_size = fread((char *)x2ap_sgnb_ack, sizeof(char), 512, pfile);
  fclose(pfile);

  //valid x2 initiating message, but not sgnb addition request
  unsigned char x2ap_resource_req_buf[512];
  size_t x2ap_resource_req_buf_size;
  pfile = fopen("test-data/X2AP-PDU-ResourceStatusRequest.per", "r");
  REQUIRE(pfile != NULL);
  x2ap_resource_req_buf_size = fread((char *)x2ap_resource_req_buf, sizeof(char), 512, pfile);
  fclose(pfile);


  SECTION("Valid X2 SgnB Addition Request"){
    protector n_plugin(false);
    bool res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    REQUIRE(n_plugin.get_requests(0) == 1);

    // todo: need to validate response ...
    // decode x2ap response
    // verify it is a successful message and compare against input x2ap ..

  }

  SECTION("Invalid X2 message"){
    protector n_plugin(1, 20, 5, 0, false);
    bool res = n_plugin(incorrect_x2ap_buf, incorrect_x2ap_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == false);
    REQUIRE(n_plugin.get_requests(0) == 0);
  }

  SECTION("Valid X2 but not Initating message"){

    protector n_plugin(1, 20, 5, 0, false);
    bool res = n_plugin(x2ap_sgnb_ack, x2ap_sgnb_ack_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == false);
    REQUIRE(n_plugin.get_requests(0) == 0);
  }
  

  SECTION("Valid X2 Initiating but not SgNB Addition Request"){
    
    protector n_plugin(1, 20, 5, 0, false);
    bool res = n_plugin(x2ap_resource_req_buf, x2ap_resource_req_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == false);
    REQUIRE(n_plugin.get_requests(0) == 0);
    
  }

  SECTION("Cycle through various messages"){
    int num_cycles = 1000;
    int num_valid = 0;
    protector n_plugin(1, 20, 5, 0, false);
    bool res;
    
    for(int i = 0; i < num_cycles;i++){
      scratch_buf_size = 512;
      res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == true);
      num_valid ++;
      
      res = n_plugin(x2ap_sgnb_ack, x2ap_sgnb_ack_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == false);
      
      res = n_plugin(x2ap_resource_req_buf, x2ap_resource_req_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == false);
  
      res = n_plugin(incorrect_x2ap_buf, incorrect_x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == false);
      
    }
    
    REQUIRE(num_valid == n_plugin.get_requests(0));
    REQUIRE(num_valid == n_plugin.get_requests(-1));
    REQUIRE(n_plugin.get_rejects(0) == 0);

  }


  

  SECTION("No blocking"){
    bool res;
    int num_packets = 1000;
    protector n_plugin(1, 20, 5, 0, false);
    for(int i = 0; i < num_packets; i++){
      res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == true);
    }

    REQUIRE(n_plugin.get_requests(0) == num_packets);
    REQUIRE(n_plugin.get_rejects(0) == 0);
    
  }
    
  SECTION("All blocking"){
    bool res;
    int num_packets = 1000;
    protector n_plugin(1, 20, 0, 100, false);
    for(int i = 0; i < num_packets; i++){
      scratch_buf_size = 512;
      res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == true);
    }

    REQUIRE(n_plugin.get_requests(0) == num_packets);
    REQUIRE(n_plugin.get_rejects(0) == num_packets);

    REQUIRE(n_plugin.get_requests(-1) == num_packets);
    REQUIRE(n_plugin.get_rejects(-1) == num_packets);
  }

  SECTION("Add/delete/configure/query policies"){
    bool res;
    int policy_id = 5;
    std::vector<double> info;
    std::vector<int> active_list;

    protector n_plugin(1, 20, 0, 100, false);

    // metrics query default policy always returns
    REQUIRE(n_plugin.get_requests(0) == 0);
    REQUIRE(n_plugin.get_rejects(0) == 0);
    
    //metrics invalid policy returns -1;
    REQUIRE(n_plugin.get_requests(100) == -1);
    REQUIRE(n_plugin.get_rejects(100) == -1);

    // 1 active policy
    n_plugin.get_active_policies(active_list);
    REQUIRE(active_list.size() == 1);

    // verify returned policy
    REQUIRE(n_plugin.is_active(active_list[0]) == true);
    
    // add a policy
    res = n_plugin.add_policy(1, 35, 10, 10, policy_id);
    REQUIRE(res == true);

    active_list.clear();
    n_plugin.get_active_policies(active_list);
    REQUIRE(active_list.size() == 2);
    
    // query default policy
    res = n_plugin.query_policy(0, info);
    REQUIRE(res == true);
    REQUIRE(info.size() == 4);
    REQUIRE(info[0] == 1);
    REQUIRE(info[1] == 20);
    REQUIRE(info[2] == 0);
    REQUIRE(info[3] == 100);

    // query new policy
    info.clear();
    res = n_plugin.query_policy(policy_id, info);
    REQUIRE(res == true);
    REQUIRE(info.size() == 4);
    REQUIRE(info[0] == 1);
    REQUIRE(info[1] == 35);
    REQUIRE(info[2] == 10);
    REQUIRE(info[3] == 10);
    
    // try adding same policy
    res = n_plugin.add_policy(1, 200, 10, 10, policy_id);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("already exists"));
    
    // delete the policy
    res = n_plugin.delete_policy(policy_id);
    REQUIRE(res == true);

    // delete non-existent policy
    res = n_plugin.delete_policy(policy_id);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("No policy with id"));

    // configure non-existent policy
    res = n_plugin.configure(1, 20, 0, 100, policy_id);
    REQUIRE(res == false);
    
    // invalid window size to configure policy
    res = n_plugin.configure(0, -1, 0, 100, 0);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal value for window"));

     // invalid trigger  in configure policy
    res = n_plugin.configure(0, 12, -1, 100, 0);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal"));

    // invalid class in configure  policy
    res = n_plugin.configure(0, 20, 1, 100, -25);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal"));

    // invalid blocking rate in configure policy
    res = n_plugin.configure(0, 21, 0, 105, 0);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal"));


    // invalid window size in add policy
    res = n_plugin.add_policy(0, -1, 0, 100, 25);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal"));

    // invalid trigger  in add policy
    res = n_plugin.add_policy(0, 12, -1, 100, 25);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal"));

    // invalid class in add policy
    res = n_plugin.add_policy(0, 20, 1, 100, -25);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal"));

    // invalid blocking rate in add policy
    res = n_plugin.add_policy(0, 21, 0, 105, 25);
    REQUIRE(res == false);
    REQUIRE_THAT(n_plugin.get_error(), Catch::Matchers::Contains("Illegal"));

    // query a non-existant policy
    res = n_plugin.query_policy(200, info);
    REQUIRE(res == false);

    
  }
  
  SECTION("Test turning enforcement on off "){
    bool res;
    protector n_plugin(1, 20, 0, 100, false);
    int num_packets = 10;

    for(int i = 0; i < num_packets; i++){
      scratch_buf_size = 512;
      res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == true);
    }

    // enforcement with 100% blocking. all should be rejected
    REQUIRE(n_plugin.get_requests(0) == num_packets);
    REQUIRE(n_plugin.get_rejects(0) == num_packets);
    
    n_plugin.clear();
    REQUIRE(n_plugin.get_requests(0) == 0);
    REQUIRE(n_plugin.get_rejects(0) == 0);

    // no enforcement even if blocking set to 100%
    // all should be accepted
    n_plugin.configure(0, 20, 0, 100, 0);
    for(int i = 0; i < num_packets; i++){
      scratch_buf_size = 512;
      res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == true);
    }
    
    REQUIRE(n_plugin.get_requests(0) == num_packets);
    REQUIRE(n_plugin.get_rejects(0) == 0);

  }

  SECTION("Test using various policies against pdus with  different subscriber profile ids"){

    // use pre-generated X2AP PDUs with two different subscriber profile ids 
    unsigned char x2ap1_buf[512];
    size_t x2ap1_size;
    unsigned char x2ap2_buf[512];
    size_t x2ap2_size;
    
    pfile = fopen("test-data/X2AP-PDU-SgNBAdditionRequest_SubId_29.per", "r");
    REQUIRE(pfile != NULL);
    x2ap1_size = fread((char *)x2ap1_buf, sizeof(char), 512, pfile);
    fclose(pfile);

    pfile = fopen("test-data/X2AP-PDU-SgNBAdditionRequest_SubId_34.per", "r");
    REQUIRE(pfile != NULL);
    x2ap2_size = fread((char *)x2ap2_buf, sizeof(char), 512, pfile);
    fclose(pfile);
    
    
    bool res;
    protector n_plugin(1, 20, 0, 100, false);
    // test with just default policy
    scratch_buf_size = 512;
    res = n_plugin(x2ap1_buf, x2ap1_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    scratch_buf_size = 512;
    res = n_plugin(x2ap2_buf, x2ap2_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    REQUIRE(n_plugin.get_requests(0) == 2);

    n_plugin.clear();

    // add a policy and test
    n_plugin.add_policy(1, 20, 0, 100, 29);
    scratch_buf_size = 512;
    res = n_plugin(x2ap1_buf, x2ap1_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    scratch_buf_size = 512;
    res = n_plugin(x2ap2_buf, x2ap2_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    REQUIRE(n_plugin.get_requests(0) == 1);
    REQUIRE(n_plugin.get_requests(29) == 1);

    // add another policy and test
    n_plugin.add_policy(1, 20, 0, 100, 34);
    scratch_buf_size = 512;
    res = n_plugin(x2ap2_buf, x2ap2_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    scratch_buf_size = 512;
    res = n_plugin(x2ap1_buf, x2ap1_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);

    REQUIRE(n_plugin.get_requests(34) == 1);
    REQUIRE(n_plugin.get_requests(29) == 2);
    REQUIRE(n_plugin.get_requests(-1) == 4); // 1 for 0, 2 for 29 and 1 for 34
    
  }
}
