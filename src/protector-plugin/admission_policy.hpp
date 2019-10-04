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


/* The control  wrapper class around the plugin
   The wrapper class is responsible for control operations of the plugin :
   set policy
   get policy
   report metrics

   the plugin handles X2AP messages 
*/
#pragma once
#ifndef ADMISSION_CTRL
#define ADMISSION_CTRL
#include <map>
#include <plugin-interface.hpp>
#include <mdclog/mdclog.h>
#include <NetworkProtector.h>
#include <chrono>
#include <sstream>

class admission: virtual public Policy
{
public:
  admission(std::string, std::string, std::string, unsigned int);
  ~admission(void);
  protector * get_protector_instance(unsigned int);
  bool setPolicy(const char *, int , std::string & );
  bool getPolicy(const char * , int, std::string & );
  std::string getName(void);
  int getMetrics(std::string & ) ;
  
private:
  void storePolicy(void);
  void init_log(void);
  void setPolicyVars(void);
  void instantiate_protector_plugin(void);

  std::map<std::string, int> current_config;
  std::map<std::string, int> prev_config;
  std::vector<protector> _plugin_instances;

  std::vector<TrieNode> policy_vars;
  std::vector<TrieNode> set_policy_response;
  std::vector<TrieNode> get_policy_response;
  std::vector<TrieNode> metric_responses;
  std::vector<TrieNode *> policy_pointer;
 
  jsonHandler set_policy_req_obj;
  jsonHandler set_policy_resp_obj;
  
  jsonHandler get_policy_req_obj;
  jsonHandler get_policy_resp_obj;
  
  jsonHandler metrics_obj;

  std::vector<int> prev_values;
  std::vector<int> curr_values;

  unsigned long int prev_time_stamp;
};

#endif
