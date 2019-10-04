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


#include "admission_policy.hpp"
admission::admission (std::string policy_schema_file, std::string samples_file,  std::string metrics_schema_file, unsigned int num_instances){
  bool res;
  
  if (num_instances == 0){
    throw std::runtime_error("Error ! Number of instances of admission_policy protector pluging must be > 0");
  }
  
  std::string response;
  std::string buffer;
  std::string error_string;
  
  std::vector<TrieNode> config_schema_path;
  
  config_schema_path.clear();

  // path to node in schema to process policy request
  config_schema_path.emplace_back("controls");
  config_schema_path.emplace_back((int)0);
  config_schema_path.emplace_back("message_receives_payload_schema");
  config_schema_path[0].add_child(&config_schema_path[1]);
  config_schema_path[1].add_child(&config_schema_path[2]);
  
  set_policy_req_obj.load_schema(policy_schema_file, &config_schema_path[0]);
  // mdclog_write(MDCLOG_INFO, "Loaded schema for set Policy request");
  
  //path to node in schema  to process policy response
  config_schema_path.clear();
  config_schema_path.emplace_back("controls");
  config_schema_path.emplace_back(0);
  config_schema_path.emplace_back("message_sends_payload_schema");
  config_schema_path[0].add_child(&config_schema_path[1]);
  config_schema_path[1].add_child(&config_schema_path[2]);

  set_policy_resp_obj.load_schema(policy_schema_file, &config_schema_path[0]);
  mdclog_write(MDCLOG_INFO, "Loaded schema for set Policy response");
  
  // load sample response 
  config_schema_path.clear();
  config_schema_path.emplace_back("message_sends_example");
  
  set_policy_resp_obj.load_buffer(samples_file, &config_schema_path[0]);
  
  // verify that our sample conforms to the schema ...
  buffer = set_policy_resp_obj.get_buffer();
  if (! set_policy_resp_obj.is_valid(buffer.c_str(), buffer.length(), response)){
    std::stringstream ss;
    ss << "Error ! Sample loaded for SET policy response = " << buffer << " from file " << samples_file << " not valid. Reason = " << response;
    throw std::runtime_error(ss.str());
  }
  mdclog_write(MDCLOG_INFO, "Verified  sample for set Policy response");
  
  // path to node in schema to  respond to get policy (current same as set policy)
  config_schema_path.clear();
  config_schema_path.emplace_back("controls");
  config_schema_path.emplace_back(0);
  config_schema_path.emplace_back("message_receives_payload_schema");
  config_schema_path[0].add_child(&config_schema_path[1]);
  config_schema_path[1].add_child(&config_schema_path[2]);

  get_policy_resp_obj.load_schema(policy_schema_file, &config_schema_path[0]);
  mdclog_write(MDCLOG_INFO, "Loaded schema for get  Policy response");
  

  // sample to respond to get policy
  config_schema_path.clear();
  config_schema_path.emplace_back("message_receives_example");

  get_policy_resp_obj.load_buffer(samples_file, &config_schema_path[0]);
  
  // verify that our sample conforms to schema
  buffer = get_policy_resp_obj.get_buffer();
  if (! get_policy_resp_obj.is_valid(buffer.c_str(), buffer.length(), response)){
    std::stringstream ss;
    ss << "Error ! Sample loaded for GET policy response = " << buffer << " from file " << samples_file << " not valid. Reason = " << response;
    throw std::runtime_error(ss.str());

  }
  
  mdclog_write(MDCLOG_INFO, "Verified sample for get Policy response");
  
  // schema & sample for metrics
  metrics_obj.load_schema(metrics_schema_file); 
  mdclog_write(MDCLOG_INFO, "Loaded schema  for ves metrics");
 
  config_schema_path.clear();
  config_schema_path.emplace_back("metrics");

  metrics_obj.load_buffer(samples_file, &config_schema_path[0]);  
  
  // verify sample conforms to schema
  buffer = metrics_obj.get_buffer();
  if (! metrics_obj.is_valid(buffer.c_str(), buffer.length(), response)){
    std::stringstream ss;
    ss << "Error ! Sample loaded for VES = " << buffer << " from file " << samples_file << " not valid. Reason = " << response;
    throw std::runtime_error(ss.str());
  }
  
  mdclog_write(MDCLOG_INFO, "Verified sample for metrics");
  
  setPolicyVars();
  
  
  //instantiate the core policy object
  for(unsigned int i = 0; i < num_instances; i++){
    instantiate_protector_plugin();
  }
  
};

void admission::instantiate_protector_plugin(void){
  _plugin_instances.emplace_back(bool(current_config["enforce"]), current_config["window_length"], current_config["blocking_rate"], current_config["trigger_threshold"]);
}

admission::~admission(void){
  prev_values.clear();
  curr_values.clear();
  policy_vars.clear();
  set_policy_response.clear();
  get_policy_response.clear();
  metric_responses.clear();
  policy_pointer.clear();
}


std::string admission::getName(void){
  return std::string("admission control policy");
}

void admission::setPolicyVars(void){


  // keys in request to set policy
  policy_vars.emplace_back("enforce");
  policy_vars.emplace_back("window_length");
  policy_vars.emplace_back("blocking_rate");
  policy_vars.emplace_back("trigger_threshold");

  
  // keys in response to set policy
  set_policy_response.emplace_back("status");
  set_policy_response.emplace_back("message");

  
  policy_pointer.push_back(&policy_vars[0]);
  policy_pointer.push_back(&policy_vars[1]);
  policy_pointer.push_back(&policy_vars[2]);
  policy_pointer.push_back(&policy_vars[3]);


  // keys in metric response
  metric_responses.emplace_back("event");  // 0
  metric_responses.emplace_back("commonEventHeader");  // 1
  metric_responses.emplace_back("measurementFields"); // 2
  metric_responses.emplace_back("additionalFields");  // 3
  metric_responses.emplace_back("SgNB Request Rate");  // 4
  metric_responses.emplace_back("SgNB Accept Rate");  // 5
  metric_responses.emplace_back("startEpochMicrosec"); // 6
  metric_responses.emplace_back("measurementInterval"); // 7
  metric_responses.emplace_back("lastEpochMicrosec");//8
  //metric_responses.emplace_back("TS"); //9
  
  metric_responses[0].add_child(&metric_responses[1]);
  metric_responses[0].add_child(&metric_responses[2]);

  metric_responses[1].add_child(&metric_responses[6]);  
  metric_responses[1].add_child(&metric_responses[8]);

  metric_responses[2].add_child(&metric_responses[3]);
  metric_responses[2].add_child(&metric_responses[7]);

  metric_responses[3].add_child(&metric_responses[4]);
  metric_responses[3].add_child(&metric_responses[5]);
  //metric_responses[3].add_child(&metric_responses[9]);

  // default config map for the policy object
  current_config.insert(std::pair<std::string, int>("enforce", 1));
  current_config.insert(std::pair<std::string, int>("window_length", 60));
  current_config.insert(std::pair<std::string, int>("blocking_rate", 1));
  current_config.insert(std::pair<std::string, int>("trigger_threshold", 1));
  
  prev_config = current_config;

  auto ts = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
  auto epoch = ts.time_since_epoch();
  auto val = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
  prev_time_stamp = val.count();
  prev_values.push_back(0);
  prev_values.push_back(0);
  curr_values = prev_values;
}

protector * admission::get_protector_instance(unsigned int index){
  if (index > _plugin_instances.size() -1){
    mdclog_write(MDCLOG_ERR, "%s, %d: Error . Requested index %u exceeds number of plugin instances %u", __FILE__, __LINE__, index, _plugin_instances.size());
    return NULL;
  }
  else{
    return &_plugin_instances[index];
  }
};



bool admission::setPolicy(const char * message, int message_length, std::string & response){

  // Get the configuration
  std::vector<TrieNode*> available_keys;
  std::vector<TrieNode*> roots;
  bool res;
  
  std::vector<TrieNode*>resp_pointer;
  resp_pointer.push_back(&set_policy_response[0]);
  resp_pointer.push_back(&set_policy_response[1]);
  
  std::string local_response;
  for(unsigned int i = 0; i < policy_vars.size(); i++){
    int res = set_policy_req_obj.get_values(message, message_length, local_response, &policy_vars[i],  available_keys);
    if (res != 0 ){
      setError(local_response);
      set_policy_response[0].set_value("FAIL");
      set_policy_response[1].set_value(local_response);
      set_policy_resp_obj.set_values(response, resp_pointer);
   
      return false;
    }
  }
  
  if ( available_keys.size() == 0){
    local_response  = "No Keys were found";
    setError(local_response);
    set_policy_response[0].set_value("FAIL");
    set_policy_response[1].set_value(local_response);
    
    set_policy_resp_obj.set_values(response,resp_pointer);
    return false;
  }
  				     

  // Get new config
  prev_config = current_config;
  
  for(std::vector<TrieNode *>::iterator it = available_keys.begin(); it != available_keys.end(); ++it){
    DataContainer const * id = (*it)->get_id();
    DataContainer const * val = (*it)->get_value();
    auto e = current_config.find(id->value.s.c_str());
    if (e != current_config.end()){
      e->second = val->value.i;
    }

  }

  // Note : xapp schema specifies window be in 'minutes'. Sliding window implementation maintains in seconds, hence multiply by 60
  current_config["window_length"] *= 60;

  // Apply the config
  res= true;
  for(std::vector<protector>::iterator it_p = _plugin_instances.begin(); it_p != _plugin_instances.end(); ++it_p){
    
    res = (*it_p).configure( bool(current_config["enforce"]), current_config["window_length"], current_config["trigger_threshold"],  current_config["blocking_rate"]);
    if (!res){
      mdclog_write(MDCLOG_ERR, "Error ::%s, %d :: Could not configure plugin\n", __FILE__, __LINE__);
      set_policy_response[0].set_value("FAIL");
      set_policy_response[1].set_value("Could not apply configuration");
      break;
    }
  }

  if (res){
    set_policy_response[0].set_value("SUCCESS");
    set_policy_response[1].set_value("configuration applied");
  }
  
  set_policy_resp_obj.set_values(response,resp_pointer);
  return true;
  
};
  
bool admission::getPolicy(const char * message, int message_length, std::string & response){

  // Note : by same token, when returning sliding window length : translate to
  // minutes
  policy_vars[0].set_value(bool(current_config["enforce"]));
  policy_vars[1].set_value((int)( (double)current_config["window_length"]/60.0));
  policy_vars[2].set_value(current_config["blocking_rate"]);
  policy_vars[3].set_value(current_config["trigger_threshold"]);

  int res = get_policy_resp_obj.set_values(response, policy_pointer);
  return true;
								       
}


int admission::getMetrics(std::string & response){
  std::vector<TrieNode *> metric_pointers;
  metric_pointers.push_back(&metric_responses[0]);

  auto ts = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
  auto epoch = ts.time_since_epoch();
  auto val_ms = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
  auto val_s = std::chrono::duration_cast<std::chrono::seconds>(epoch);
  
  long int  current_time_stamp = val_ms.count();
  long int current_time = val_s.count();
  long int interval = current_time_stamp - prev_time_stamp;


  curr_values[0] = _plugin_instances[0].get_requests();
  curr_values[1] =  _plugin_instances[0].get_rejects();

  //curr_values[0] = rand()%100 + prev_values[0];
  //curr_values[1] = rand()%20 + prev_values[1];
 
  //std::cout <<" Accept counter = " << curr_values[0]<< " Reject counter = " << curr_values[1] << " Request Count = " <<  (curr_values[0] - prev_values[0]) << " Reject count = " << curr_values[1] - prev_values[1] << std::endl;

  metric_responses[4].set_value(std::to_string(curr_values[0] - prev_values[0]));
  metric_responses[5].set_value(std::to_string((curr_values[0] - prev_values[0]) - (curr_values[1] - prev_values[1])));
  metric_responses[6].set_value(prev_time_stamp);
  metric_responses[8].set_value(current_time_stamp);
  metric_responses[7].set_value(interval);
  //metric_responses[9].set_value(current_time);
  
  prev_values = curr_values;
  prev_time_stamp = current_time_stamp;
  int res = metrics_obj.set_values(response, metric_pointers);

  return res;
  
}
