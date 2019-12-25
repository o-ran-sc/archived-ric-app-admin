/*
    // basically just set size to be larger than allowed even if actual 
    // message is small
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
   Date   : Feb 2019
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <unistd.h>
#include <xapp_utils.hpp>

#define MESSAGE_SIZE 512

int num_recv_pkts = 0;
int num_tx_pkts = 0;
int num_dropped_pkts = 0;
int failed_tx = 0;
int num_ping_pkts = 0;
int num_pong_pkts = 0;
uint32_t NumPkts=1000;
unsigned char meid[32];

struct Test_message{
  struct timespec ts;
  char payload[MESSAGE_SIZE];
};

bool rcvd_pkts(rmr_mbuf_t *rcv_msg){
  memset(meid, 0,  32);
  rmr_get_meid(rcv_msg, meid);
  num_recv_pkts++;
  return false;
}

bool echo_into_space_pkts(rmr_mbuf_t *rcv_msg){
  rcv_msg->mtype = 100; // some invalid type 
  num_recv_pkts++;
  return true;
}

void dropped_pkts(rmr_mbuf_t *send_msg){
  std::cout <<"Error handler triggered " << std::endl;
  num_dropped_pkts++;
}


bool pong_x(rmr_mbuf_t *rcv_msg){
  rcv_msg->mtype = 102; //ping port
  num_ping_pkts++;
  return true;
}

bool ping_recv(rmr_mbuf_t * rcv_msg){
  num_pong_pkts++;
  return false;
}


TEST_CASE("Test xapp functionality", "[xapp]"){

  // Test parameters
  char app_name[128] = "Test App";
  char port[16] = "tcp:4999";

  init_logger("UNIT_TEST_XAPP", MDCLOG_INFO);
  
  SECTION("Illegal buffer size"){
    REQUIRE_THROWS(XaPP(app_name, port, RMR_BUFFER_SIZE + 1));
  }
  
  SECTION("All good"){
    REQUIRE_NOTHROW(XaPP(app_name, port, sizeof(Test_message)));
  }
   
  SECTION("Simple + memory"){
    XaPP check_xapp = XaPP(app_name, port, sizeof(Test_message));
    REQUIRE(check_xapp.get_name() == std::string(app_name));
  }
  
  SECTION("Configuration test"){  
    XaPP test_xapp = XaPP(app_name, port, sizeof(Test_message));
    REQUIRE(test_xapp.get_status() == true);
    REQUIRE(test_xapp.get_rmr_context() != NULL);
    
  }
  

  SECTION("Transmission test with start"){
    num_recv_pkts = 0;
    num_dropped_pkts = 0;
    failed_tx = 0;
    
    // Instantiate and configure xAPP
    XaPP test_xapp = XaPP(app_name, port, sizeof(Test_message));

    // Start receiver for test
    test_xapp.StartThread(rcvd_pkts);
    sleep(1);
  
    // Test Send  normal message
    Test_message my_message;
    uint32_t i = 0;
    for(i = 0; i < NumPkts; i++){
      clock_gettime(CLOCK_REALTIME, &(my_message.ts));
      snprintf(my_message.payload, MESSAGE_SIZE, "hello world %d", i);
      bool res = test_xapp.Send(102, sizeof(Test_message), (void *) (&my_message));
      if (!res){
  	failed_tx ++;
      }
      usleep(10);
    
    }
    sleep(1);
    REQUIRE(test_xapp.get_num_active_threads() == 1);
    test_xapp.Stop();
    REQUIRE(test_xapp.get_num_active_threads() == 0);
    
    std::cout <<"Num Packets Sent = " << NumPkts << " Received packets = " << num_recv_pkts << " Dropped packets = " << num_dropped_pkts << " Failed sends = "<< failed_tx << std::endl;

    
    REQUIRE(num_recv_pkts > 0);
    REQUIRE(num_dropped_pkts == 0);
    REQUIRE(failed_tx == 0);
    REQUIRE(num_recv_pkts == NumPkts);
  }

    SECTION("Transmission test error handler with start"){

      num_recv_pkts = 0;
      num_dropped_pkts = 0;
      failed_tx = 0;
      
      // Instantiate and configure xAPP
      XaPP test_xapp = XaPP(app_name, port, sizeof(Test_message) );

      
      // Start receiver for test
      test_xapp.StartThread(&echo_into_space_pkts, &dropped_pkts);
      sleep(1);
      
      // Test Send  normal message
      Test_message my_message;
      uint32_t i = 0;
      for(i = 0; i < NumPkts; i++){
  	clock_gettime(CLOCK_REALTIME, &(my_message.ts));
  	snprintf(my_message.payload, MESSAGE_SIZE, "hello world %d", i);
  	bool res = test_xapp.Send(102, sizeof(Test_message), (void *) (&my_message));
  	if (!res){
  	  failed_tx ++;
  	}
  	usleep(10);
	
      }
      sleep(1);
      
      test_xapp.Stop();  
      std::cout <<"Num Packets Sent = " << NumPkts << " Received packets = " << num_recv_pkts << " Dropped packets = " << num_dropped_pkts << " Failed sends = "<< failed_tx << std::endl;

      
      REQUIRE(num_recv_pkts == NumPkts);
      REQUIRE(num_dropped_pkts == NumPkts);
      REQUIRE(failed_tx == 0);
    }
    
    
    SECTION("Transmission test error handler with start thread"){
      
      num_recv_pkts = 0;
      num_dropped_pkts = 0;
      failed_tx = 0;
      
      // Instantiate and configure xAPP
      XaPP test_xapp = XaPP(app_name, port, sizeof(Test_message) );

      
      // Start receiver for test
      test_xapp.StartThread(&echo_into_space_pkts, &dropped_pkts);
      sleep(1);
      
      // Test Send  normal message
      Test_message my_message;
      uint32_t i = 0;
      for(i = 0; i < NumPkts; i++){
  	clock_gettime(CLOCK_REALTIME, &(my_message.ts));
  	snprintf(my_message.payload, MESSAGE_SIZE, "hello world %d", i);
  	bool res = test_xapp.Send(102, sizeof(Test_message), (void *) (&my_message));
  	if (!res){
  	  failed_tx ++;
  	}
  	usleep(10);
	
      }
      sleep(1);
      
      test_xapp.Stop();  
      std::cout <<"Num Packets Sent = " << NumPkts << " Received packets = " << num_recv_pkts << " Dropped packets = " << num_dropped_pkts << " Failed sends = "<< failed_tx << std::endl;

      
      REQUIRE(num_recv_pkts == NumPkts);
      REQUIRE(num_dropped_pkts == NumPkts);
      REQUIRE(failed_tx == 0);


    }

    SECTION("Test ping pong : two xapps send to each other. "){

      char ping_name[] = "ping";
      char pong_name[] = "pong";
      char ping_port[] = "tcp:4999";
      char pong_port[] = "tcp:4998";
      
      // Instantiate  ping xAPP
      XaPP ping_xapp = XaPP(ping_name, ping_port, sizeof(Test_message) );


      // Instantiate pong xapp
      XaPP pong_xapp = XaPP(pong_name, pong_port, sizeof(Test_message) );
 
      // Start receiver on ping
      ping_xapp.StartThread(ping_recv);
      sleep(1);

      // Start receiver on pong
      pong_xapp.StartThread(pong_x);
      
      // send messages from ping to pong
      Test_message my_message;
      uint32_t i = 0;
      for(i = 0; i < NumPkts; i++){
    	clock_gettime(CLOCK_REALTIME, &(my_message.ts));
    	snprintf(my_message.payload, MESSAGE_SIZE, "hello world %d", i);
    	bool res = ping_xapp.Send(101, sizeof(Test_message), (void *) (&my_message));
    	if (!res){
    	  failed_tx ++;
    	}
      }

      sleep(1);
      pong_xapp.Stop();

      REQUIRE(failed_tx == 0);
      REQUIRE(num_ping_pkts == NumPkts);
      REQUIRE(num_pong_pkts == NumPkts);


      ping_xapp.Stop();

    }
}


TEST_CASE(" Test out various  transmission methods ..", "1"){

  // Test parameters
  char app_name[128] = "Test App";
  char port[16] = "tcp:4999";
  
  init_logger("UNIT_TEST_XAPP", MDCLOG_INFO);
  
  bool res;
  unsigned char my_meid[32] = "ABC123";
  Test_message my_message;
  
  SECTION("Test if message larger than allowed"){
    num_recv_pkts = 0;
    num_dropped_pkts = 0;
    failed_tx = 0;
    
    // Instantiate and configure xAPP
    XaPP test_xapp = XaPP(app_name, port, sizeof(Test_message) );
    
    // Start receiver for test
    test_xapp.StartThread(&rcvd_pkts);
    sleep(1);
    
    // Test sending a message of size larger than allowed
    // basically just set size to be larger than allowed even if actual 
    // message is small
    res = test_xapp.Send(102, RMR_BUFFER_SIZE + 100, (void *)(&my_message));
    REQUIRE(res == false);

    res = test_xapp.Send(102, RMR_BUFFER_SIZE + 100, (void *)(&my_message), my_meid);
    REQUIRE(res == false);
    test_xapp.Stop();
  

  }
  
  SECTION("Test with tlv"){
    num_recv_pkts = 0;
    num_dropped_pkts = 0;
    failed_tx = 0;
    
    // Instantiate and configure xAPP
    XaPP test_xapp = XaPP(app_name, port, sizeof(Test_message) );

    // Start receiver for test
    test_xapp.StartThread(&rcvd_pkts);
    sleep(1);
    
    // Test Send  with tlv
    clock_gettime(CLOCK_REALTIME, &(my_message.ts));
    snprintf(my_message.payload, MESSAGE_SIZE, "hello world");
    res = test_xapp.Send(102, sizeof(Test_message), (void *) (&my_message));
    sleep(1);
    
    // Test send with tlv and meid
    res = test_xapp.Send(102, sizeof(Test_message), (void *) (&my_message), my_meid);
    sleep(1);
    
    test_xapp.Stop();
    
    REQUIRE(num_recv_pkts == 2);
    REQUIRE(!strcmp((const char *)meid, (const char *)my_meid));
  }  
}
