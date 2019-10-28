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
   Date   : Sept 2019
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <NetworkProtector.h>


TEST_CASE("Protector Plugin", "Test Input types"){


  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST MESSAGE PROCESSOR ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);
  FILE *pfile;
 
  unsigned char scratch_buf[512];
  size_t scratch_buf_size = 512;
  
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
    protector n_plugin(1, 20, 5, 0, false);
    bool res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    REQUIRE(n_plugin.get_requests() == 1);

    // todo: need to validate response ...
    // decode x2ap response
    // verify it is a successful message and compare against input x2ap ..

  }

  SECTION("Invalid X2 message"){
    protector n_plugin(1, 20, 5, 0, false);
    bool res = n_plugin(incorrect_x2ap_buf, incorrect_x2ap_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == false);
    REQUIRE(n_plugin.get_requests() == 0);
  }

  SECTION("Valid X2 but not Initating message"){

    protector n_plugin(1, 20, 5, 0, false);
    bool res = n_plugin(x2ap_sgnb_ack, x2ap_sgnb_ack_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == false);
    REQUIRE(n_plugin.get_requests() == 0);
  }
  

  SECTION("Valid X2 Initiating but not SgNB Addition Request"){
    
    protector n_plugin(1, 20, 5, 0, false);
    bool res = n_plugin(x2ap_resource_req_buf, x2ap_resource_req_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == false);
    REQUIRE(n_plugin.get_requests() == 0);
    
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
    
    REQUIRE(num_valid == n_plugin.get_requests());
    REQUIRE(n_plugin.get_rejects() == 0);

  }


  
  SECTION("No enforce"){
    bool res;
    int num_packets = 1000;
    protector n_plugin(0, 20, 5, 100, false);
    for(int i = 0; i < num_packets; i++){
      res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == true);
    }

    REQUIRE(n_plugin.get_requests() == num_packets);
    REQUIRE(n_plugin.get_rejects() == 0);
    
  }

  SECTION("No blocking"){
    bool res;
    int num_packets = 1000;
    protector n_plugin(1, 20, 5, 0, false);
    for(int i = 0; i < num_packets; i++){
      res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
      REQUIRE(res == true);
    }

    REQUIRE(n_plugin.get_requests() == num_packets);
    REQUIRE(n_plugin.get_rejects() == 0);
    
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

    REQUIRE(n_plugin.get_requests() == num_packets);
    REQUIRE(n_plugin.get_rejects() == num_packets);

  }

  SECTION("Configuration"){
    bool res;
    protector n_plugin(1, 20, 0, 100, false);
    res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    REQUIRE(n_plugin.get_requests() == 1);
    REQUIRE(n_plugin.get_rejects() == 1);
    
    n_plugin.clear();
    REQUIRE(n_plugin.get_requests() == 0);
    REQUIRE(n_plugin.get_rejects() == 0);

    n_plugin.configure(0, 20, 0, 100);
    scratch_buf_size = 512;
    res = n_plugin(x2ap_buf, x2ap_buf_size, scratch_buf, &scratch_buf_size);
    REQUIRE(res == true);
    REQUIRE(n_plugin.get_requests() == 1);
    REQUIRE(n_plugin.get_rejects() == 0);

    res = n_plugin.configure(0, -1, 0, 100);
    REQUIRE(res == false);

  }
}
