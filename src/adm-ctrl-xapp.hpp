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

#pragma once
#ifndef ADM_CTRL_XAPP_
#define ADM_CTRL_XAPP_

#include <limits>
#include <map>
#include <getopt.h>
#include <csignal>
#include <cstdlib>
#include <time.h>
#include <errno.h>
#include <xapp_utils.hpp>
#include <rmr/RIC_message_types.h>
#include <curl_interface.hpp>
#include <e2sm.hpp>
#include "admission_policy.hpp"
#include "message_processor_class.hpp"

#define MAX_SLEEP 86400
#define DEFAULT_A1_SCHEMA_FILE "/etc/xapp/a1-schema.json"
#define DEFAULT_VES_SCHEMA_FILE "/etc/xapp/ves-schema.json"
#define DEFAULT_SAMPLE_FILE "/etc/xapp/samples.json"
#define DEFAULT_VES_COLLECTOR_URL "127.0.0.1:6350"
#define DEFAULT_XAPP_ID "ac-xapp-123"
#define DEFAULT_VES_MEASUREMENT_INTERVAL 10
#define MAX_SUBSCRIPTION_ATTEMPTS 10
//================================================

// convenient typedef for the list of plugins to be loaded
// currently only the admission control plugin. any plugin
// should be inheriting from the Policy abstract class
typedef  std::vector<std::unique_ptr<Policy> > plugin_list;


// configuration parameters 
struct configuration {
  
  char name[128] = "xapp_adm_ctrl";
  char port[16] = "tcp:4560";
  
  int num_threads = 1;
  std::unique_ptr<XaPP> my_xapp;
  std::string a1_schema_file ;
  std::string sample_file ;
  std::string ves_schema_file ;
  std::string ves_collector_url;
  unsigned int measurement_interval = 0;
  int log_level = MDCLOG_WARN;
  int test_mode = 0;
  ProcessingLevelTypes processing_level = ProcessingLevelTypes::ALL;
  bool report_mode_only = true;
  std::string operating_mode = "REPORT";
  int max_sub_loops = 2;
  std::string xapp_id;
  void fill_gnodeb_list(char * gNodeB_string){
    gNodeB_list.clear();
    char * gnb = strtok(gNodeB_string, ",");
    while(gnb != NULL){
      gNodeB_list.push_back(gnb);
      gnb = strtok(NULL, ",");
    }
    
  };
  
  std::vector<string> gNodeB_list;

};

// class that handles startup and shutdown operations
class  init {
public:
  init(XaPP & , subscription_handler & , configuration &);
  void startup(void);
  void shutdown(void);
  
private:
  void startup_subscribe_requests(void );
  void shutdown_subscribe_deletes(void);
  void startup_get_policies(void );
  
  subscription_handler * sub_handler_ref;
  XaPP * xapp_ref;
  configuration * config_ref;
};

void usage(char *command);
void get_environment_config(configuration & config_instance);
void get_command_line_config(int argc, char **argv, configuration &config_instance);

extern int run_program;
extern bool report_mode_only;

#endif
