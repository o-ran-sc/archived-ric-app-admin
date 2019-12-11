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

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/schema.h>

#include <iostream>
#include <vector>
#include <map>
#include <plugin-interface.hpp>
#include <mdclog/mdclog.h>
#include <NetworkProtector.h>
#include <chrono>
#include <sstream>

#define MAX_INSTANCES 10

class admission: virtual public Policy
{
public:
  admission(std::string, std::string, std::string, unsigned int, std::string, bool report_only=true);
  ~admission(void);
  protector * get_protector_instance(unsigned int);
  bool setPolicy(const char *, int , std::string & );
  bool getPolicy(const char * , int, std::string & );
  std::string getName(void);
  int getMetrics(std::vector<std::string> & ) ;
  std::string get_error(void) const {return error_string;};
  unsigned int get_num_policies(void) {return policy_table.size();};
private:
  void storePolicy(void);
  void init_log(void);
  void setPolicyVars(void);
  void instantiate_protector_plugin(bool);
  bool load_file(std::string, std::string &) ;
  bool load_schema(const std::string & , const std::string & , std::unique_ptr<rapidjson::SchemaDocument> & , std::unique_ptr<rapidjson::SchemaValidator> &);
  void process_counters(int , std::string & );

  std::vector<protector> _plugin_instances;
  std::map<std::string, int> policy_table;
  std::map<std::string, rapidjson::Pointer> window_policy_vars;
  std::map<std::string, rapidjson::Pointer> generic_policy_vars;
  std::map<std::string, rapidjson::Pointer> metric_vars;

 
  std::unique_ptr<rapidjson::SchemaDocument> downstream_schema_ref_, notify_schema_ref_, metrics_schema_ref_;
  std::unique_ptr<rapidjson::SchemaValidator> downstream_validator_, notify_validator_, metrics_validator_;
  rapidjson::Document notify_message_, metrics_message_;

  std::string _xapp_id;
  
  // stores past count of requests, reject and time stamp
  std::map<int, std::vector<long int>> counters;
  std::string error_string;
  unsigned long int prev_time_stamp;
};

#endif
