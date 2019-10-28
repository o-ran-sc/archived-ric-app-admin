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

#define SCHEMA_FILE "../schemas/adm-ctrl-xapp-policy-schema.json"
#define SAMPLE_FILE "../schemas/samples.json"
#define VES_FILE  "../schemas/ves_schema.json"


bool report_mode_only = false;

TEST_CASE("Admission Policy Wrapper", "[acxapp]"){

  mdclog_attr_t *attr;
  mdclog_attr_init(&attr);
  mdclog_attr_set_ident(attr, "UNIT TEST MESSAGE PROCESSOR ");
  mdclog_init(attr);
  mdclog_level_set(MDCLOG_INFO);
  mdclog_attr_destroy(attr);


  SECTION("Invalid AC xAPP  Schema"){
    REQUIRE_THROWS( admission("hello there", SAMPLE_FILE, VES_FILE, 1));
  }

  SECTION("Invalid sample file"){
    REQUIRE_THROWS(admission(SCHEMA_FILE, "hello there", VES_FILE, 1));
    REQUIRE_THROWS(admission(SCHEMA_FILE, "random.txt", VES_FILE, 1));
  }

  SECTION("Invalid VES schema"){
    REQUIRE_THROWS(admission(SCHEMA_FILE, SAMPLE_FILE, "hello there", 1));
  }
    
  SECTION("Set policy : invalid"){
    std::string invalid_policy1 = "{\"blocking_rate\":20\"}";
    std::string invalid_policy2 = "{\"blocking_rate\":120, \"enforce\":\"true\", \"window_length\":50, \"trigger_threshold\":10}";
    bool res;
    std::string response;
    
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1);
    res = adm_plugin.setPolicy(invalid_policy1.c_str(), invalid_policy1.length(), response);

    REQUIRE(res == false);
    REQUIRE_THAT(response, Catch::Matchers::Contains("FAIL"));

    res = adm_plugin.setPolicy(invalid_policy2.c_str(), invalid_policy2.length(), response);
    REQUIRE(res == false);
    REQUIRE_THAT(response, Catch::Matchers::Contains("FAIL"));

  }

  SECTION("Set policy : valid"){
    std::string valid_policy = "{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10}";
    bool res;
    std::string response;
    
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1);
    res = adm_plugin.setPolicy(valid_policy.c_str(), valid_policy.length(), response);
    std::cout <<"Response = " << response << " for " << valid_policy << std::endl;
    REQUIRE(res == true);
    REQUIRE_THAT(response, Catch::Matchers::Contains("SUCCESS"));


    
    
  }

  SECTION("Get policy"){

    std::string valid_policy = "{\"blocking_rate\":20, \"enforce\":true, \"window_length\":50, \"trigger_threshold\":10}";
    bool res;
    std::string response;

    // first apply policy
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1);
    res = adm_plugin.setPolicy(valid_policy.c_str(), valid_policy.length(), response);
    std::cout <<"Response = " << response << std::endl;
    REQUIRE(res == true);
    REQUIRE_THAT(response, Catch::Matchers::Contains("SUCCESS"));
 

    // now retreive policy and check
    res = adm_plugin.getPolicy(valid_policy.c_str(), valid_policy.length(), response);
    REQUIRE(res == true);

    REQUIRE_THAT(response, Catch::Matchers::Contains("\"trigger_threshold\":10"));

  }

  SECTION("Metrics"){
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1);
    int res;
    std::string response;
    
    res = adm_plugin.getMetrics(response);
    REQUIRE(res == 0);
    std::cout << "Metrics response = " << response << std::endl;
    REQUIRE_THAT(response, Catch::Matchers::Contains("\"SgNB Request Rate\":\"0\""));
  }

  SECTION("Get plugin"){
    admission adm_plugin(SCHEMA_FILE, SAMPLE_FILE, VES_FILE, 1);;
    protector * p = NULL;
    p = adm_plugin.get_protector_instance(100);
    REQUIRE( p == NULL);

    p = adm_plugin.get_protector_instance(0);
    REQUIRE(p != NULL);
    REQUIRE(p->get_requests() == 0);
  }
      
      
}

