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

#include "adm-ctrl-xapp.hpp"

void get_environment_config(configuration & config_instance){

  // Order of priority for setting variables
  // So we start in reverse order
  //  -- command line 
  // -- environment variable
  // -- default path 

  if (const char *env_schema = std::getenv("A1_SCHEMA_FILE")){
    config_instance.a1_schema_file.assign(env_schema);
    mdclog_write(MDCLOG_INFO, "Schema file set to %s from environment variable",     config_instance.a1_schema_file.c_str());
  }
  else{
    config_instance.a1_schema_file.assign(DEFAULT_A1_SCHEMA_FILE);
    mdclog_write(MDCLOG_INFO, "Using default schema file %s\n", config_instance.a1_schema_file.c_str());
  }
 
  if (const char *env_schema = std::getenv("VES_SCHEMA_FILE")){
    config_instance.ves_schema_file.assign(env_schema);
    mdclog_write(MDCLOG_INFO, "VES Schema file set to %s from environment variable",     config_instance.ves_schema_file.c_str());
  }
  else{
    config_instance.ves_schema_file.assign(DEFAULT_VES_SCHEMA_FILE);
    mdclog_write(MDCLOG_INFO, "Using default ves schema file %s\n",     config_instance.ves_schema_file.c_str());
  }
   
   
  if (const char *env_schema = std::getenv("SAMPLES_FILE")){
    config_instance.sample_file.assign(env_schema);
    mdclog_write(MDCLOG_INFO, "JSON Sample file set to %s from environment variable",     config_instance.sample_file.c_str());
  }
  else{
    config_instance.sample_file.assign(DEFAULT_SAMPLE_FILE);
    mdclog_write(MDCLOG_INFO, "Using default sample file %s\n",     config_instance.sample_file.c_str());
  }

      
  if (const char *env_schema = std::getenv("VES_COLLECTOR_URL")){
    config_instance.ves_collector_url.assign(env_schema);
    mdclog_write(MDCLOG_INFO, "VES Collector URL  set to %s from environment variable",     config_instance.ves_collector_url.c_str());
  }
  else{
    config_instance.ves_collector_url.assign(DEFAULT_VES_COLLECTOR_URL);
    mdclog_write(MDCLOG_INFO, "Using default ves collector url  %s\n",     config_instance.ves_collector_url.c_str());
  }

   
  if (const char *env_schema = std::getenv("VES_MEASUREMENT_INTERVAL")){
    config_instance.measurement_interval = atoi(env_schema);
    if (    config_instance.measurement_interval < 1 ||     config_instance.measurement_interval > MAX_SLEEP){
      throw std::runtime_error("Invalid measurmeent interval provided. Must between [1 and " + std::to_string(MAX_SLEEP) + "] seconds");
       
    }
    mdclog_write(MDCLOG_INFO, "Interval set to %d from environment variable",     config_instance.measurement_interval);
  }
  else{
    config_instance.measurement_interval = DEFAULT_VES_MEASUREMENT_INTERVAL;
    mdclog_write(MDCLOG_INFO, "Using default measurement interval %d\n",     config_instance.measurement_interval);
  }


   
  if (char *env_gnodeb = std::getenv("GNODEB")){
    config_instance.fill_gnodeb_list(env_gnodeb);
    mdclog_write(MDCLOG_INFO, "gNodeB List set to %s from environment variable", env_gnodeb);
  }


  if (const char *env_opmode = std::getenv("OPERATING_MODE")){
    config_instance.operating_mode.assign(env_opmode);
    if (    config_instance.operating_mode != "REPORT" &&     config_instance.operating_mode != "CONTROL"){
      throw std::runtime_error("Invalid operating mode: " +     config_instance.operating_mode + " Must be REPORT or CONTROL");
    }
    mdclog_write(MDCLOG_INFO, "Operating mode set from environment variable to %s\n",     config_instance.operating_mode.c_str());
  }

   
  if (const char *threads = std::getenv("THREADS")){
    config_instance.num_threads = atoi(threads);
    if (    config_instance.num_threads <= 0 or     config_instance.num_threads  > MAX_THREADS){
      mdclog_write(MDCLOG_ERR, "Error :: %s, %d :: Must specify numnber of threads between [1 and %d]. Specified = %d\n", __FILE__, __LINE__, MAX_THREADS,     config_instance.num_threads);
      exit(-1);
    }
    else{
      mdclog_write(MDCLOG_INFO, "Number of threads set to %d from environment variable\n",     config_instance.num_threads);
    }
  }

  if (const char *test= std::getenv("TEST_MODE")){
    config_instance.test_mode = atoi(test);
    mdclog_write(MDCLOG_INFO, "xAPP set to Test Mode state %d from Environment Variable",     config_instance.test_mode);
  }

  if (const char *log_env = std::getenv("LOG_LEVEL")){
    if (!strcmp(log_env, "MDCLOG_INFO")){
      config_instance.log_level = MDCLOG_INFO;
    }
    else if (!strcmp(log_env, "MDCLOG_WARN")){
      config_instance.log_level = MDCLOG_WARN;
    }
    else if (!strcmp(log_env, "MDCLOG_ERR")){
      config_instance.log_level = MDCLOG_ERR;
    }
    else if (!strcmp(log_env, "MDCLOG_DEBUG")){
      config_instance.log_level = MDCLOG_DEBUG;
    }
    else{
      config_instance.log_level = MDCLOG_WARN;
      std::cerr <<"Error ! Illegal environment option for log level  ignored. Setting log level to " << config_instance.log_level << std::endl;
    }
  }
  
}

void get_command_line_config(int argc, char **argv, configuration &config_instance){

    // Parse command line options to over ride
  static struct option long_options[] = 
    {
	/* Thse options require arguments */
	{"name", required_argument, 0, 'n'},
        {"port", required_argument, 0, 'p'},
 	{"threads", required_argument,    0, 't'},
	{"a1-schema", required_argument, 0, 'a'},
	{"ves-schema", required_argument, 0, 'v'},
	{"samples", required_argument, 0, 's'},
	{"ves-url", required_argument, 0, 'u'},
	{"interval", required_argument, 0, 'i'},
	{"gNodeB", required_argument, 0, 'g'},
	{"opmode", required_argument, 0, 'c'},
	{"verbose", no_argument, &config_instance.log_level, MDCLOG_INFO},
	{"test", no_argument, &config_instance.test_mode, 1},
	 
    };


   while(1) {

	int option_index = 0;
	char c = getopt_long(argc, argv, "n:p:t:s:g:a:v:u:i:c:", long_options, &option_index);

        if(c == -1){
	    break;
         }

	switch(c)
	  {

	  case 0:
	    /* An option flag was set. 
	       Do nothing for now */
	    break;
	  
	  case 'n':
	    strcpy(config_instance.name, optarg);
	    break;
	  
	  case 'p':
	    strcpy(config_instance.port, optarg);
	    break;
	  
	  case 't':
	    config_instance.num_threads = atoi(optarg);
	    mdclog_write(MDCLOG_INFO, "Number of threads set to %d from command line e\n", config_instance.num_threads);
	    break;

	  case 's':
	    config_instance.sample_file.assign(optarg);
	    mdclog_write(MDCLOG_INFO, "Samples JSON  file set to %s from command line ", config_instance.sample_file.c_str());     
	    break;

	  case 'a':
	    config_instance.a1_schema_file.assign(optarg);
	    mdclog_write(MDCLOG_INFO, "Schema file set to %s from command line ", config_instance.a1_schema_file.c_str());
	    break;
	  
	  case 'v':
	    config_instance.ves_schema_file.assign(optarg);
	    mdclog_write(MDCLOG_INFO, "VES Schema file set to %s from command line ", config_instance.ves_schema_file.c_str());
	    break;

	  case 'c':
	    config_instance.operating_mode.assign(optarg);
	    mdclog_write(MDCLOG_INFO, "Operating mode set from command line to %s\n", config_instance.operating_mode.c_str());
	    break;
	  
	  case 'u':
	    config_instance.ves_collector_url.assign(optarg);
	    mdclog_write(MDCLOG_INFO, "VES collector url set to %s from command line ", config_instance.ves_collector_url.c_str());
	    break;

	  case 'i':
	    config_instance.measurement_interval = atoi(optarg);
	    if (config_instance.measurement_interval < 1 || config_instance.measurement_interval > MAX_SLEEP){
	      throw std::runtime_error("Invalid measurmeent interval provided. Must between [1 and " + std::to_string(MAX_SLEEP) + "] seconds");
	    }
	    mdclog_write(MDCLOG_INFO, "Measurement interval set to %d from command line\n", config_instance.measurement_interval);
	    break;
	    
	  case 'g':
	    config_instance.fill_gnodeb_list(optarg);
	    mdclog_write(MDCLOG_INFO, "gNodeB List set to %s from command line ", optarg);
	    break;
	  
	  case 'h':
	    usage(argv[0]);
	    exit(0);
	  
	  default:
	    usage(argv[0]);
	    exit(1);
	  }
   };

}


void usage(char *command){
  std::cout <<"Usage : " << command << " " << std::endl;
  std::cout <<" --name[-n] xapp_instance_name "<< std::endl;
    std::cout <<" --port[-p] port to listen on e.g tcp:4561  "<< std::endl;
    std::cout << "--threads[-t] number of listener threads "<< std::endl ;
    std::cout << "--a1-schema[-a] a1 schema file location" << std::endl;
    std::cout << "--ves-schema[-v] ves schema file location" << std::endl;
    std::cout << "--samples [-s]  samples file location with samples for all jsons" << std::endl;
    std::cout << "--ves-url [-u] ves collector url" << std::endl;
    std::cout <<"[--gNodeB[][-g] gNodeB" << std::endl;
    std::cout <<"--interval[-i] measurement interval to send to ves collector (in seconds)" << std::endl;
    std::cout <<"--test puts xapp in test mode where it sends subscription, waits for interval and then sends delete subscription " << std::endl;
    std::cout <<"--opmode [-c] type of operatoring mode : either REPORT or INSERT. In REPORT, does not send a control message back to gNodeB" << std::endl;
    std::cout << "--verbose " << std::endl;
}
