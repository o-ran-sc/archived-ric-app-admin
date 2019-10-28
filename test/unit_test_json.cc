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
#include <json_handler.hpp>

void gen_message(std::string & message_string, bool enforce, int blocking_rate, int trigger_threshold, int window_length){
  std::string enforce_string ((enforce? "true":"false"));
  message_string =  "{\"enforce\":" + enforce_string + ",";
  message_string += std::string("\"blocking_rate\":") + std::to_string(blocking_rate) + ",";
  message_string += std::string("\"window_length\":") + std::to_string(window_length) + ",";
  message_string += std::string("\"trigger_threshold\":") + std::to_string(trigger_threshold) + "}";
}


TEST_CASE("JSON-schema", "Schema Check"){
  jsonHandler json_test;
  std::vector<TrieNode> schema_path;
  TrieNode * test_node;
  std::string valid_schema_file = "../schemas/adm-ctrl-xapp-policy-schema.json";
  std::string valid_sample_file = "../schemas/sample.json";
  std::string response;
  
  
  SECTION("Invalid Schema File"){
    REQUIRE_THROWS( json_test.load_schema("hello there"));
    REQUIRE_THROWS( json_test.load_schema("hello there", NULL));
    REQUIRE_THROWS( json_test.load_schema("hello there", test_node));
  }

  
  SECTION("Invalid JSON"){
    REQUIRE_THROWS( json_test.load_schema("invalid_json.txt"));
    REQUIRE_THROWS( json_test.load_schema("invalid_json.txt", NULL));
    REQUIRE_THROWS( json_test.load_schema("invalid_json.txt", test_node));
    
    
  }

  // SECTION("Valid File and JSON"){
  //   bool res = json_test.load_schema(valid_schema_file);
  //   REQUIRE(res == TRUE);
  // }

  SECTION("Valid File and JSON, invalid schema root "){
    schema_path.clear();
    schema_path.emplace_back("controlS");
    schema_path.emplace_back((int)0);
    schema_path.emplace_back("message_receives_payload_schema");
    schema_path[0].add_child(&schema_path[1]);
    schema_path[1].add_child(&schema_path[2]);
    
    REQUIRE_THROWS(json_test.load_schema(valid_schema_file, &schema_path[0]));
  }

  SECTION("Valid File and JSON & root"){
    schema_path.clear();
    schema_path.emplace_back("controls");
    schema_path.emplace_back((int)0);
    schema_path.emplace_back("message_receives_payload_schema");
    schema_path[0].add_child(&schema_path[1]);
    schema_path[1].add_child(&schema_path[2]);
    std::string message_string;
    bool res;
    
    json_test.load_schema(valid_schema_file, &schema_path[0]);

    gen_message(message_string, 0, 10, 20, 30);
    res = json_test.is_valid(message_string.c_str(), message_string.length(), response);
    REQUIRE(res == true);      

    gen_message(message_string, 0, 10, -5, 30);
    res = json_test.is_valid(message_string.c_str(), message_string.length(), response);
    REQUIRE(res == false);      

    gen_message(message_string, 0, 120, 10, 5);
    res = json_test.is_valid(message_string.c_str(), message_string.length(), response);
    REQUIRE(res == false);      

    gen_message(message_string, 0, 20, 10, 5);
    message_string = message_string + "}";
    res = json_test.is_valid(message_string.c_str(), message_string.length(), response);
    REQUIRE(res == false);      
    
  }


}

TEST_CASE("DataContainer", "Get/set"){
  std::string test_string = "Hello there";
  int test_int = 101;
  double test_real = 123.8;
  long int test_long = 2839289;
  unsigned int test_uint = 189;
  unsigned long test_ulong = 29392329;
  
  SECTION("Validate data types"){
    TrieNode test("Test Node");
    test.set_value(test_string);
    const DataContainer * val = test.get_value();
    REQUIRE(val->tag == DataContainer::Types::str);
    REQUIRE(val->value.s == test_string);
    
    test.set_value(test_int);
    val = test.get_value();
    REQUIRE(val->tag == DataContainer::Types::integer);
    REQUIRE(val->value.i == test_int);


    test.set_value(test_uint);
    val = test.get_value();
    REQUIRE(val->tag == DataContainer::Types::uinteger);
    REQUIRE(val->value.ui == test_uint);

    test.set_value(test_real);
    val = test.get_value();
    REQUIRE(val->tag == DataContainer::Types::real);
    REQUIRE(val->value.f == test_real);
    
    test.set_value(test_long);
    val = test.get_value();
    REQUIRE(val->tag == DataContainer::Types::big_integer);
    REQUIRE(val->value.l == test_long);

    
    test.set_value(test_ulong);
    val = test.get_value();
    REQUIRE(val->tag == DataContainer::Types::ubig_integer);
    REQUIRE(val->value.ul == test_ulong);

    test.print_id();
    test.print_value();

    REQUIRE(test.is_child() == true);

    TrieNode test_child("child");
    test.add_child(&test_child);
    REQUIRE(test.is_child() == false);
   
  }

}
TEST_CASE("JSON-getset", "Get/Set values"){

    jsonHandler json_test;
    std::vector<TrieNode> schema_path;
    std::vector<TrieNode> key_path;
    TrieNode  *test_node;
    std::string valid_schema_file = "../schemas/adm-ctrl-xapp-policy-schema.json";
    std::string valid_sample_file = "../schemas/samples.json";
    std::string response;
    bool res;
    
    SECTION("Invalid buffer file"){
      REQUIRE_THROWS( json_test.load_buffer("hello there"));
      REQUIRE_THROWS( json_test.load_buffer("hello there", NULL));
      REQUIRE_THROWS( json_test.load_buffer("hello there", test_node));
    }
    
    SECTION("Validate buffer"){
      schema_path.clear();
      schema_path.emplace_back("controls");
      schema_path.emplace_back((int)0);
      schema_path.emplace_back("message_receives_payload_schema");
      schema_path[0].add_child(&schema_path[1]);
      schema_path[1].add_child(&schema_path[2]);
      json_test.load_schema(valid_schema_file, &schema_path[0]);
      
      schema_path.clear();
      schema_path.emplace_back("message_receives_example");
      json_test.load_buffer(valid_sample_file, &schema_path[0]);

      std::string buffer = json_test.get_buffer();
      res = json_test.is_valid(buffer.c_str(), buffer.length(), response);
      REQUIRE(res == true);
    }

    SECTION("Get correct value from internal buffer"){
      int res;
      std::string message_string;
      std::vector<TrieNode *> available_keys;
      TrieNode test("window_length");
      TrieNode invalid_test("Window_length");
      
      // Try with no buffer loaded 
      res = json_test.get_values(response, &test, available_keys);
      REQUIRE(res == -4);

      // Try with buffer loaded
      schema_path.clear();
      schema_path.emplace_back("message_receives_example");
      json_test.load_buffer(valid_sample_file, &schema_path[0]);

      available_keys.clear();
      res = json_test.get_values(response, &test, available_keys);
      REQUIRE (res == 1);
      REQUIRE(available_keys.size() == 1);
      REQUIRE(available_keys[0]->get_value()->value.i == 10);
      
      available_keys.clear();
      res = json_test.get_values(response, &invalid_test, available_keys);
      REQUIRE (res == 0);
      REQUIRE(available_keys.size() == 0);

      
    }
    SECTION("Get values from supplied buffer"){
      int res;
      std::string message_string;
      std::vector<TrieNode *> available_keys;
      

      bool enforce = true;
      int blocking_rate = 10;
      int trigger_threshold = 5000;
      int window_length = 25;

      TrieNode test("window_length");
      available_keys.clear();
      gen_message(message_string, enforce, blocking_rate, trigger_threshold, window_length);
      res = json_test.get_values(message_string.c_str(), message_string.length(), response, &test, available_keys);
      REQUIRE(res == 0);
      REQUIRE(available_keys.size() == 1);
      REQUIRE(available_keys[0]->get_value()->value.i == 25);
 
      TrieNode invalid_test("Window_length");
      available_keys.clear();
      gen_message(message_string, enforce, blocking_rate, trigger_threshold, window_length);
      res = json_test.get_values(message_string.c_str(), message_string.length(), response, &invalid_test, available_keys);
      REQUIRE(res == -3);
      REQUIRE(available_keys.size() == 0);

      
    }
    

    SECTION("Get/Set  from memory"){
      int res;
      std::vector<TrieNode *> keys;

      TrieNode test1("blocking_rate");
      int blocking_rate = 23;
      test1.set_value(blocking_rate);

      TrieNode test2("window_length");
      int window_length = 45;
      test2.set_value(window_length);
      
      schema_path.clear();
      schema_path.emplace_back("message_receives_example");
      json_test.load_buffer(valid_sample_file, &schema_path[0]);

      keys.clear();
      keys.push_back(&test1);
      keys.push_back(&test2);
      res = json_test.set_values(response,  keys);
      REQUIRE(res == 0);

      keys.clear();
      std::string message_string(response);
      res = json_test.get_values(message_string.c_str(), message_string.length(), response, &test1, keys);
      REQUIRE(res == 0);
      REQUIRE(keys.size() == 1);
      REQUIRE(keys[0]->get_value()->value.i == blocking_rate);

      keys.clear();
      res = json_test.get_values(message_string.c_str(), message_string.length(), response, &test2, keys);
      REQUIRE(res == 0);
      REQUIRE(keys.size() == 1);
      REQUIRE(keys[0]->get_value()->value.i == window_length);

    }


    SECTION("Get/Set from buffer "){
      int res;
      std::vector<TrieNode *> keys;
      std::string message_string;
      
      bool enforce = true;
      int blocking_rate = 10;
      int trigger_threshold = 5000;
      int window_length = 25;

      gen_message(message_string, enforce, blocking_rate, trigger_threshold, window_length);

      TrieNode test_node("blocking_rate");
      blocking_rate = 33;
      test_node.set_value(blocking_rate);
      

      keys.clear();
      keys.push_back(&test_node);
      res = json_test.set_values(message_string.c_str(), message_string.length(), response,  keys);
      REQUIRE(res == 0);

      keys.clear();
      message_string = response;
      res = json_test.get_values(message_string.c_str(), message_string.length(), response, &test_node, keys);
      REQUIRE(res == 0);
      REQUIRE(keys.size() == 1);
      REQUIRE(keys[0]->get_value()->value.i == blocking_rate);
      
    }


    SECTION("multi-level get/set"){
      std::vector<TrieNode *> keys;
      schema_path.clear();
      schema_path.emplace_back("test1");
      schema_path.emplace_back(1);
      schema_path[0].add_child(&schema_path[1]);
      
      std::string sample_file = "test-data/test_sample.json";
      REQUIRE_THROWS(json_test.load_buffer(sample_file, &schema_path[0]));

      schema_path.clear();
      schema_path.emplace_back("test1");
      schema_path.emplace_back(1);
      schema_path.emplace_back("test4");
      schema_path[0].add_child(&schema_path[1]);
      schema_path[1].add_child(&schema_path[2]);
      
      json_test.load_buffer(sample_file, &schema_path[0]);
      std::string buffer = json_test.get_buffer();
      
      TrieNode test("test5");
      keys.clear();
      res = json_test.get_values(response, &test, keys);
      REQUIRE(res == 1);
      REQUIRE(keys.size() == 1);
      REQUIRE(keys[0]->get_value()->value.s == "new target");
    }
      
}



      
