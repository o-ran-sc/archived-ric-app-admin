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


#include <message_processor_class.hpp>
#include <admission_policy.hpp>

#define ASN_DEBUG 1
#define SCHEMA_FILE "../schemas/adm-ctrl-xapp-policy-schema.json"
#define SAMPLE_FILE "../schemas/samples.json"
#define VES_FILE  "../schemas/ves_schema.json"

#define BUFFER_SIZE 2<<15

bool report_mode_only = false;

void policy_stub(int mtype, const char *message, int msg_len,  std::string & response, bool atype){
  // do nothing;
}

TEST_CASE("message processor functionality", "[acxapp]"){

  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST MESSAGE PROCESSOR ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_DEBUG);
  mdclog_attr_destroy(attr);

  bool res;
  e2sm_indication e2sm_indication_obj;
  FILE *pfile;
  
  //==== e2sm indication header : just some
  // random fill for now 
  e2sm_header_helper indication_header;
  indication_header.egNB_id="hello";
  indication_header.plmn_id="there";
  indication_header.interface_direction = 1;
  indication_header.egNB_id_type = 2;
  
   
   unsigned char buf_header[32];
   size_t buf_header_size = 32;
   
   // Encode the indication header
   res = e2sm_indication_obj.encode_indication_header(&buf_header[0], &buf_header_size, indication_header);
   REQUIRE(res == true);

   
   //====== x2ap sgnb addition reuqest
   unsigned char x2ap_buf[256];
   size_t x2ap_buf_size = 256;
   pfile = fopen("test-data/X2AP-PDU-SgNBAdditionRequest.per", "r");
   REQUIRE(pfile != NULL);
   x2ap_buf_size = fread((char *)x2ap_buf, sizeof(char), 256, pfile);
   
   //==== e2ap indication
   ric_indication_helper dinput ;   
   dinput.action_id = 100;
   dinput.func_id = 10;
   dinput.indication_sn = 100;
   dinput.req_id = 6;
   dinput.req_seq_no = 11;   

   //==== filler for incorrect x2ap
   unsigned char * incorrect_x2ap;
   size_t incorrect_x2ap_size;

   // ====== mock incorrect e2sm header ...
   unsigned char incorrect_e2sm_header[32];
   size_t incorrect_e2sm_header_size = 32;
   for(int i = 0; i  < 32; i++){
     incorrect_e2sm_header[i] = 'q';
   }
   
   
   ric_indication indication_pdu;

   // rmr buffer
   rmr_mbuf_t rmr_message;
   rmr_message.mtype = RIC_INDICATION;



   /* encoding pdu put here */
   size_t data_size = BUFFER_SIZE;
   unsigned char * data = new unsigned char[data_size];
   assert(data != NULL);

   SECTION("Null Test cases when no handlers registered"){

     
     // message processor
     message_processor my_processor;


     rmr_message.mtype = RIC_INDICATION;	 
     rmr_message.payload = &data[0];
     rmr_message.len = data_size;
     my_processor(&rmr_message);
     REQUIRE(my_processor.get_state() == MISSING_HANDLER_ERROR);


     rmr_message.mtype = RIC_SUB_RESP;	 
     rmr_message.payload = &data[0];
     rmr_message.len = data_size;
     res = my_processor(&rmr_message);
     REQUIRE(my_processor.get_state() == MISSING_HANDLER_ERROR);


     rmr_message.mtype = DC_ADM_INT_CONTROL;	 
     rmr_message.payload = &data[0];
     rmr_message.len = data_size;
     res = my_processor(&rmr_message);
     REQUIRE(my_processor.get_state() == MISSING_HANDLER_ERROR);

     rmr_message.mtype = DC_ADM_GET_POLICY;	 
     rmr_message.payload = &data[0];
     rmr_message.len = data_size;
     res = my_processor(&rmr_message);
     REQUIRE(my_processor.get_state() == MISSING_HANDLER_ERROR);

     
   }
   
   
   SECTION("Test various message types for all handlers registered in ALL mode"){

     // message processor
     message_processor my_processor(ALL, false);

     // admission control policy wrapper
     admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, false);
     
     // register the protector plugin with message processor 
     my_processor.register_protector(adm_plugin.get_protector_instance(0));


     srand(112309);
     
     int num_cycles = 1;     
     for(int i = 0; i < num_cycles;i ++){

       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;// action_type;
       dinput.indication_header = buf_header;
       dinput.indication_header_size = buf_header_size;
       

       //random injection of incorrect e2ap pdu
       for(unsigned int i = 0; i < data_size; i++){
       	 data[i] = rand()% 256 ;
       }
       
       rmr_message.mtype = RIC_INDICATION;	 
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == E2AP_INDICATION_ERROR);
       
       //random injection of erroneous x2ap pdu
       for(int k = 8; k <= 14; k++){
       	 incorrect_x2ap_size = 2 << k;
       	 incorrect_x2ap =  new unsigned char[incorrect_x2ap_size];
       	 assert(incorrect_x2ap != NULL);
       	 for(unsigned int i = 0; i < incorrect_x2ap_size; i++){
       	   incorrect_x2ap[i] = rand()%256;
       	 }
	 
   	 data_size = BUFFER_SIZE;
       	 dinput.indication_type = 1;
       	 dinput.indication_msg = incorrect_x2ap;
       	 dinput.indication_msg_size = incorrect_x2ap_size;
       	 res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
	 REQUIRE(res == true);
	 
       	 rmr_message.mtype = RIC_INDICATION;
       	 rmr_message.payload = &data[0];
       	 rmr_message.len = data_size;
       	 res = my_processor(&rmr_message);
       	 REQUIRE(my_processor.get_state() == PLUGIN_ERROR);

       	 delete[] incorrect_x2ap;
       }

       
       //all correct e2ap and x2ap 
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;
       dinput.indication_msg = &x2ap_buf[0];
       dinput.indication_msg_size = x2ap_buf_size; 
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == NO_ERROR);
       
       
       //incorrect e2sm header
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;    
       dinput.indication_header = incorrect_e2sm_header;
       dinput.indication_header_size = incorrect_e2sm_header_size;
       
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == E2SM_INDICATION_HEADER_ERROR);



       // // fake buffer x2 size 
       std::cout <<"Trying corrupt message field size" << std::endl;
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;
       dinput.indication_header = buf_header;
       dinput.indication_header_size = buf_header_size;

       dinput.indication_msg = &x2ap_buf[0];
       dinput.indication_msg_size = rand()%128;
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = 2<<20; // put a fake buffer size
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == RMR_ERROR);
			    
     }

     

   }
   
   SECTION("Test various messages in E2AP_PROC_ONLY mode"){


     // message processor
     message_processor my_processor(E2AP_PROC_ONLY);

     // admission control policy wrapper
     admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, false);

     
     // register the protector plugin with message processor 
     my_processor.register_protector(adm_plugin.get_protector_instance(0));


     srand(112309);
     
     int num_cycles = 1;     
     for(int i = 0; i < num_cycles;i ++){

       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;// action_type;
       dinput.indication_header = buf_header;
       dinput.indication_header_size = buf_header_size;
       

       //random injection of incorrect e2ap pdu
              //random injection of incorrect e2ap pdu
       for(unsigned int i = 0; i < data_size; i++){
       	 data[i] = rand()% 256 ;
       }
       
       rmr_message.mtype = RIC_INDICATION;	 
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == E2AP_INDICATION_ERROR);
       
       //random injection of erroneous x2ap pdu
       for(int k = 8; k <= 14; k++){
       	 incorrect_x2ap_size = 2 << k;
       	 incorrect_x2ap =  new unsigned char[incorrect_x2ap_size];
       	 assert(incorrect_x2ap != NULL);
       	 for(unsigned int i = 0; i < incorrect_x2ap_size; i++){
       	   incorrect_x2ap[i] = rand()%256;
       	 }
	 
   	 data_size = BUFFER_SIZE;
       	 dinput.indication_type = 1;
       	 dinput.indication_msg = incorrect_x2ap;
       	 dinput.indication_msg_size = incorrect_x2ap_size;
       	 res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
	 REQUIRE(res == true);
	 
       	 rmr_message.mtype = RIC_INDICATION;
       	 rmr_message.payload = &data[0];
       	 rmr_message.len = data_size;
       	 res = my_processor(&rmr_message);
       	 REQUIRE(my_processor.get_state() == NO_ERROR); // since we don't process X2ap

       	 delete[] incorrect_x2ap;
       }

       
       //all correct e2ap and x2ap 
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;
       dinput.indication_msg = &x2ap_buf[0];
       dinput.indication_msg_size = x2ap_buf_size; 
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == NO_ERROR);
       
       
       //incorrect e2sm header
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;    
       dinput.indication_header = incorrect_e2sm_header;
       dinput.indication_header_size = incorrect_e2sm_header_size;
       
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == NO_ERROR); // since we don't process e2sm

     }

     REQUIRE(adm_plugin.get_protector_instance(0)->get_requests() == 0);
   }


   SECTION("Test various messages in E2SM_PROC_ONLY mode"){


     // message processor
     message_processor my_processor(E2SM_PROC_ONLY);

     // admission control policy wrapper
     admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, false);
     
     // register the protector plugin with message processor 
     my_processor.register_protector(adm_plugin.get_protector_instance(0));


     srand(112309);
     
     int num_cycles = 1;     
     for(int i = 0; i < num_cycles;i ++){

       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;// action_type;
       dinput.indication_header = buf_header;
       dinput.indication_header_size = buf_header_size;
       

       //random injection of incorrect e2ap pdu
              //random injection of incorrect e2ap pdu
       for(unsigned int i = 0; i < data_size; i++){
       	 data[i] = rand()% 256 ;
       }
       
       rmr_message.mtype = RIC_INDICATION;	 
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == E2AP_INDICATION_ERROR);
       
       //random injection of erroneous x2ap pdu
       for(int k = 8; k <= 14; k++){
       	 incorrect_x2ap_size = 2 << k;
       	 incorrect_x2ap =  new unsigned char[incorrect_x2ap_size];
       	 assert(incorrect_x2ap != NULL);
       	 for(unsigned int i = 0; i < incorrect_x2ap_size; i++){
       	   incorrect_x2ap[i] = rand()%256;
       	 }
	 
   	 data_size = BUFFER_SIZE;
       	 dinput.indication_type = 1;
       	 dinput.indication_msg = incorrect_x2ap;
       	 dinput.indication_msg_size = incorrect_x2ap_size;
       	 res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
	 REQUIRE(res == true);
	 
       	 rmr_message.mtype = RIC_INDICATION;
       	 rmr_message.payload = &data[0];
       	 rmr_message.len = data_size;
       	 res = my_processor(&rmr_message);
       	 REQUIRE(my_processor.get_state() == NO_ERROR); // since we don't process X2ap

       	 delete[] incorrect_x2ap;
       }

       
       //all correct e2ap and x2ap 
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;
       dinput.indication_msg = &x2ap_buf[0];
       dinput.indication_msg_size = x2ap_buf_size; 
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == NO_ERROR);
       
       
       //incorrect e2sm header
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;    
       dinput.indication_header = incorrect_e2sm_header;
       dinput.indication_header_size = incorrect_e2sm_header_size;
       
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == E2SM_INDICATION_HEADER_ERROR); 

     }

     REQUIRE(adm_plugin.get_protector_instance(0)->get_requests() == 0);
   }


   SECTION("Test various message types for all handlers registered in ALL mode with response"){

     // message processor
     message_processor my_processor(ALL, false);

     // admission control policy wrapper
     admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, false);

     // subscription handler
     subscription_handler sub_handler;
     
     // register the protector plugin with message processor 
     my_processor.register_protector(adm_plugin.get_protector_instance(0));
     my_processor.register_policy_handler(policy_stub);
     my_processor.register_subscription_handler(&sub_handler);

     srand(112309);
     
     int num_cycles = 1;     
     for(int i = 0; i < num_cycles;i ++){

       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;// action_type;
       dinput.indication_header = buf_header;
       dinput.indication_header_size = buf_header_size;
       

       //random injection of incorrect e2ap pdu
       for(unsigned int i = 0; i < data_size; i++){
       	 data[i] = rand()% 256 ;
       }
       
       rmr_message.mtype = RIC_INDICATION;	 
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == E2AP_INDICATION_ERROR);
       
       //random injection of erroneous x2ap pdu
       for(int k = 8; k <= 14; k++){
       	 incorrect_x2ap_size = 2 << k;
       	 incorrect_x2ap =  new unsigned char[incorrect_x2ap_size];
       	 assert(incorrect_x2ap != NULL);
       	 for(unsigned int i = 0; i < incorrect_x2ap_size; i++){
       	   incorrect_x2ap[i] = rand()%256;
       	 }
	 
   	 data_size = BUFFER_SIZE;
       	 dinput.indication_type = 1;
       	 dinput.indication_msg = incorrect_x2ap;
       	 dinput.indication_msg_size = incorrect_x2ap_size;
       	 res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
	 REQUIRE(res == true);
	 
       	 rmr_message.mtype = RIC_INDICATION;
       	 rmr_message.payload = &data[0];
       	 rmr_message.len = data_size;
       	 res = my_processor(&rmr_message);
       	 REQUIRE(my_processor.get_state() == PLUGIN_ERROR);

       	 delete[] incorrect_x2ap;
       }

       
       //all correct e2ap and x2ap 
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;
       dinput.indication_msg = &x2ap_buf[0];
       dinput.indication_msg_size = x2ap_buf_size; 
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == NO_ERROR);
       
       
       //incorrect e2sm header
       data_size = BUFFER_SIZE;
       dinput.indication_type = 1;    
       dinput.indication_header = incorrect_e2sm_header;
       dinput.indication_header_size = incorrect_e2sm_header_size;
       
       res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
       REQUIRE(res == true);
       
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size;
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == E2SM_INDICATION_HEADER_ERROR);



       // // rmr buffer size
       rmr_message.mtype = RIC_INDICATION;
       rmr_message.payload = &data[0];
       rmr_message.len = 2 << 20; // put a fake buffer size
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == RMR_ERROR);


       // subscription response       
       rmr_message.mtype = RIC_SUB_RESP;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size; 
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == NO_ERROR);

       // policy get
       rmr_message.mtype = DC_ADM_GET_POLICY;
       rmr_message.payload = &data[0];
       rmr_message.len = data_size; 
       res = my_processor(&rmr_message);
       REQUIRE(my_processor.get_state() == NO_ERROR);


       // policy set
     }

     
     REQUIRE(adm_plugin.get_protector_instance(0)->get_requests() == num_cycles);
   }


   SECTION("Trigger control low buffer related errors"){

     data_size = BUFFER_SIZE;
     dinput.indication_type = 1;// action_type;
     dinput.indication_header = buf_header;
     dinput.indication_header_size = buf_header_size;
       
     //all correct e2ap and x2ap 
     data_size = BUFFER_SIZE;
     dinput.indication_type = 1;
     dinput.indication_msg = &x2ap_buf[0];
     dinput.indication_msg_size = x2ap_buf_size; 
     res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
     REQUIRE(res == true);

     rmr_message.mtype = RIC_INDICATION;
     rmr_message.payload = &data[0];
     rmr_message.len = data_size;
     
     // message processor
     message_processor my_processor(ALL, false, 128);

     // admission control policy wrapper
     admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, false);
     
     // register the protector plugin with message processor 
     my_processor.register_protector(adm_plugin.get_protector_instance(0));

       
     res = my_processor(&rmr_message);
     REQUIRE(my_processor.get_state() == E2AP_CONTROL_ERROR);
   }
   
   SECTION("Trigger buffer related errors"){
       
     data_size = BUFFER_SIZE;
     dinput.indication_type = 1;// action_type;
     dinput.indication_header = buf_header;
     dinput.indication_header_size = buf_header_size;
       
     //all correct e2ap and x2ap 
     data_size = BUFFER_SIZE;
     dinput.indication_type = 1;
     dinput.indication_msg = &x2ap_buf[0];
     dinput.indication_msg_size = x2ap_buf_size; 
     res = indication_pdu.encode_e2ap_indication(&data[0], &data_size, dinput);
     REQUIRE(res == true);

     rmr_message.mtype = RIC_INDICATION;
     rmr_message.payload = &data[0];
     rmr_message.len = data_size;
  
     // message processor
     message_processor my_processor(ALL, false, 106);

     // admission control policy wrapper
     admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, false);
     
     // register the protector plugin with message processor 
     my_processor.register_protector(adm_plugin.get_protector_instance(0));

       
     res = my_processor(&rmr_message);
     REQUIRE(my_processor.get_state() == BUFFER_ERROR);


   }
   
   delete[] data;

}
