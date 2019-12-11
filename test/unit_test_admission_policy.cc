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


#include <admission_policy.hpp>

#define SCHEMA_FILE "../schemas/rate-control-policy.json"
#define SAMPLE_FILE "../schemas/samples.json"
#define VES_FILE  "../schemas/ves_schema.json"
#define INVALID_JSON "./test-data/invalid.json"
#define VALID_JSON "./test-data/valid.json"

bool report_mode_only = false;

TEST_CASE("Admission Policy Wrapper", "[acxapp]"){

  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST MESSAGE PROCESSOR ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_DEBUG);
  mdclog_attr_destroy(attr);

  std::string xapp_id = "ac-xapp-test123";
  
  SECTION("Invalid number of instances requested"){
    REQUIRE_THROWS( admission(SCHEMA_FILE,  SAMPLE_FILE, VES_FILE, 0, xapp_id));
    REQUIRE_THROWS( admission(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, -1, xapp_id));
    REQUIRE_THROWS( admission(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1000, xapp_id));
  }
  
  SECTION("Invalid AC xAPP  Schema file "){
    REQUIRE_THROWS( admission("hello there", SAMPLE_FILE, VES_FILE, 1, xapp_id));
  }
  
  SECTION("Invalid sample file"){
    REQUIRE_THROWS(admission(SCHEMA_FILE, "hello there", VES_FILE, 1, xapp_id));
  }

  SECTION("Invalid VES schema file "){
    REQUIRE_THROWS(admission(SCHEMA_FILE, SAMPLE_FILE, "hello there", 1, xapp_id));
  }

  SECTION("Invalid JSON in a schema"){
    REQUIRE_THROWS(admission(INVALID_JSON, SAMPLE_FILE, VES_FILE, 1, xapp_id));
  }

  SECTION("Valid JSON, but no valid schema key"){
    REQUIRE_THROWS(admission(VALID_JSON, SAMPLE_FILE, VES_FILE, 1, xapp_id));
    REQUIRE_THROWS(admission(SCHEMA_FILE, VALID_JSON, VES_FILE, 1, xapp_id));

  }
  
  SECTION("Test policy schema validation"){
    bool res;
    std::string response;
    
    std::string valid_policy = "{\"policy_type_id\":21000, \"policy_instance_id\":\"hello-there\", \"operation\":\"CREATE\", \"payload\":{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10, \"class\":14}}";

    std::string invalid_policy1 = "{\"policy_type_id\":21000, \"policy_instance_id\":\"hello-there\", \"operation\":\"CREATE\", \"payload\":{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10, \"class\":5000}}";
    
    std::string invalid_policy2 = "{\"policy_instance_id\":\"hello-there\", \"operation\":\"CREATE\", \"payload\":{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10, \"class\":14}}";

    std::string invalid_policy3 = "{\"policy_type_id\":21000, \"policy_instance_id\":\"hello-there\", \"operation\":\"CREATED\", \"payload\":{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10, \"class\":14}}";

    std::string invalid_policy4 = "{\"policy_type_id\":21000, \"policy_instance_id\":\"hello-there\", \"operation\":\"CREATE\", \"payload\":{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10}}";

    std::string invalid_policy5 = "hello-there";
    
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, xapp_id);

    REQUIRE_THAT(adm_plugin.getName(), Catch::Matchers::Equals("admission control policy"));
    
    res = adm_plugin.setPolicy(invalid_policy1.c_str(), invalid_policy1.length(), response);
    REQUIRE(res == false);

    res = adm_plugin.setPolicy(invalid_policy2.c_str(), invalid_policy2.length(), response);
    REQUIRE(res == false);

    res = adm_plugin.setPolicy(invalid_policy3.c_str(), invalid_policy3.length(), response);
    REQUIRE(res == false);

    res = adm_plugin.setPolicy(invalid_policy4.c_str(), invalid_policy4.length(), response);
    REQUIRE(res == false);

    res = adm_plugin.setPolicy(invalid_policy5.c_str(), invalid_policy5.length(), response);
    REQUIRE(res == false);

    res = adm_plugin.setPolicy(valid_policy.c_str(), valid_policy.length(), response);
    REQUIRE(res == true);

    
  }
  
  SECTION("Test Set/configure/delete policy"){
    std::string policy = "{\"policy_type_id\":21000, \"policy_instance_id\":\"hello-there\", \"operation\":\"CREATE\", \"payload\":{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10, \"class\":14}}";
    std::string delete_policy = "{\"policy_type_id\":21000, \"policy_instance_id\":\"hello-there\", \"operation\":\"DELETE\"}";  

    
    bool res;
    std::string response, test_policy, policy_instance_id;
    rapidjson::Document doc;
    rapidjson::StringBuffer s_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s_buffer);

    int num_policies = 10;
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, xapp_id);
    doc.Parse(policy.c_str());
    for(int i = 1; i <= num_policies; i++){
      policy_instance_id = "rate_control_policy_" + std::to_string(i);
      rapidjson::SetValueByPointer(doc, "/policy_instance_id", policy_instance_id.c_str());
      rapidjson::SetValueByPointer(doc, "/payload/class", i);
      s_buffer.Clear();
      writer.Reset(s_buffer);
      doc.Accept(writer);
      test_policy.assign(s_buffer.GetString(), s_buffer.GetLength());
      
      // valid policy should succeed
      res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
      std::cout <<"Response1 = " << response << std::endl;
      REQUIRE(res == true);
    
      // trying to add policy with same class as before should also return true
      res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
      REQUIRE(res == true);

    }
    
    REQUIRE(adm_plugin.get_num_policies() == num_policies);
    
    // configure existing policy should work
    rapidjson::SetValueByPointer(doc, "/operation", "UPDATE");
    for(int i = 1; i <= num_policies; i++){
      policy_instance_id = "rate_control_policy_" + std::to_string(i);
      rapidjson::SetValueByPointer(doc, "/policy_instance_id", policy_instance_id.c_str());
      rapidjson::SetValueByPointer(doc, "/payload/class", i);
      rapidjson::SetValueByPointer(doc, "/payload/blocking_rate", 55.0);
      s_buffer.Clear();
      writer.Reset(s_buffer);
      doc.Accept(writer);
      test_policy.assign(s_buffer.GetString(), s_buffer.GetLength());
      
      // valid policy should succeed
      res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
      REQUIRE(res == true);
      
    }
    
    // configure non-existing policy should fail
    policy_instance_id = "rate_control_policy_" + std::to_string(1000);
    rapidjson::SetValueByPointer(doc, "/policy_instance_id", policy_instance_id.c_str());
    s_buffer.Clear();
    writer.Reset(s_buffer);
    doc.Accept(writer);
    test_policy.assign(s_buffer.GetString(), s_buffer.GetLength());
    res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
    REQUIRE(res == false);

    // configure exsting policy but with non-existing class should fail
    policy_instance_id = "rate_control_policy_" + std::to_string(1);
    rapidjson::SetValueByPointer(doc, "/policy_instance_id", policy_instance_id.c_str());
    rapidjson::SetValueByPointer(doc, "/payload/class", 200);
    s_buffer.Clear();
    writer.Reset(s_buffer);
    doc.Accept(writer);
    test_policy.assign(s_buffer.GetString(), s_buffer.GetLength());
    res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
    REQUIRE(res == false);

    // delete existing policy should work
    doc.Parse(delete_policy.c_str());
    for(int i = 1; i <= num_policies; i++){
      std::string policy_instance_id = "rate_control_policy_" + std::to_string(i);
      rapidjson::SetValueByPointer(doc, "/policy_instance_id", policy_instance_id.c_str());
      s_buffer.Clear();
      writer.Reset(s_buffer);
      doc.Accept(writer);
      test_policy.assign(s_buffer.GetString(), s_buffer.GetLength());
      
      // delete policy should succeed since these policies were created
      res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
      REQUIRE(res == true);
    }

    REQUIRE(adm_plugin.get_num_policies() == 0);

    //delete non-existing policy should fail
    res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
    REQUIRE(res == false);


    
  }

  // SECTION("Get policy"){

  //   std::string valid_policy = "{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10}";
  //   bool res;
  //   std::string response;

  //   // first apply policy
  //   admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, xapp_id);
  //   res = adm_plugin.setPolicy(valid_policy.c_str(), valid_policy.length(), response);
  //   std::cout <<"Response = " << response << std::endl;
  //   REQUIRE(res == true);
  //   REQUIRE_THAT(response, Catch::Matchers::Contains("SUCCESS"));
 

  //   // now retreive policy and check
  //   res = adm_plugin.getPolicy(valid_policy.c_str(), valid_policy.length(), response);
  //   REQUIRE(res == true);

  //   REQUIRE_THAT(response, Catch::Matchers::Contains("\"trigger_threshold\":10"));

  // }

  SECTION("Metrics"){

    std::string policy = "{\"policy_type_id\":21000, \"policy_instance_id\":\"hello-there\", \"operation\":\"CREATE\", \"payload\":{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10, \"class\":14}}";

    bool res;
    std::string response, test_policy, policy_instance_id;
    rapidjson::Document doc;
    rapidjson::StringBuffer s_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s_buffer);

    int num_policies = 10;
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, xapp_id);
    doc.Parse(policy.c_str());
    // Add some policies first 
    for(int i = 1; i <= num_policies; i++){
      policy_instance_id = "rate_control_policy_" + std::to_string(i);
      rapidjson::SetValueByPointer(doc, "/policy_instance_id", policy_instance_id.c_str());
      rapidjson::SetValueByPointer(doc, "/payload/class", i);
      s_buffer.Clear();
      writer.Reset(s_buffer);
      doc.Accept(writer);
      test_policy.assign(s_buffer.GetString(), s_buffer.GetLength());
      
      // valid policy should succeed
      res = adm_plugin.setPolicy(test_policy.c_str(), test_policy.length(), response);
      REQUIRE(res == true);
      //REQUIRE_THAT(response, Catch::Matchers::Contains("SUCCESS"));    
    }
    

    // now get metrics
    // First time, measurementInterval should be zero
    std::vector<std::string> metrics;
    adm_plugin.getMetrics(metrics);
    REQUIRE(metrics.size() == num_policies + 1);
    for(auto const e : metrics){
      std::cout << e<< std::endl;
      REQUIRE_THAT(e, Catch::Matchers::Contains("\"Class Id\""));
      REQUIRE_THAT(e, Catch::Matchers::Contains("\"measurementInterval\":\"0\""));
    }

    int interval = 5;
    // wait for 'x' seconds and try again
    std::this_thread::sleep_for(std::chrono::seconds(interval));
    rapidjson::Pointer int_ref("/event/measurementFields/measurementInterval");
    
    metrics.clear();
    adm_plugin.getMetrics(metrics);
    REQUIRE(metrics.size() == num_policies + 1);
    for(auto const e : metrics){
      REQUIRE(doc.Parse(e.c_str()).HasParseError() == 0);
      std::cout << e<< std::endl;
      REQUIRE_THAT(e, Catch::Matchers::Contains("\"Class Id\""));
      // extract measurement interval
      rapidjson::Value *interval_val = int_ref.Get(doc);
      REQUIRE(interval_val != NULL);
      double read_interval_approx = atof(interval_val->GetString());
      REQUIRE(read_interval_approx == Approx(interval).margin(1));
    }

    
  }

SECTION("Get plugin"){
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1, xapp_id);
    protector * p = NULL;
    p = adm_plugin.get_protector_instance(100);
    REQUIRE( p == NULL);

    p = adm_plugin.get_protector_instance(0);
    REQUIRE(p != NULL);
    REQUIRE(p->get_requests(0) == 0);
  }
      
      
}

