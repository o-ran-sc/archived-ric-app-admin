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

using namespace rapidjson;

// constructor loads the policy schema, metric schema, and example payload JSON files
// instantiates the required number of protector plugins 
admission::admission (std::string policy_schema_file, std::string samples_file,  std::string metrics_schema_file, unsigned int num_instances, std::string xapp_id, bool report_only){
  if (num_instances < 1 || num_instances > MAX_INSTANCES){
    std::stringstream ss;
    ss << "Error !" << __FILE__ << ", " << __LINE__ << " Number of instances must be between " << 1 << " and " << MAX_INSTANCES << ". Specified = " << num_instances << std::endl;
    error_string = ss.str();
    throw std::runtime_error(error_string);
  }

  _xapp_id.assign(xapp_id);
  std::string response;
  std::string buffer;
  std::stringstream ss;
  Document _doc;
  Value * ref;
  std::string schema_key;

  // Load schema files and extract relevant validators
  // schema for downstream message
  bool res = load_schema(policy_schema_file, "/downstream_schema", downstream_schema_ref_, downstream_validator_);
  if(res == false){
    throw std::runtime_error(error_string);
  }
  // schema for notification message
  res = load_schema(policy_schema_file, "/notify_schema", notify_schema_ref_, notify_validator_);
  if(res == false){
    throw std::runtime_error(error_string);
  }
  // schema for metrics message
  res = load_schema(metrics_schema_file, "", metrics_schema_ref_, metrics_validator_);
  if(res == false){
    throw std::runtime_error(error_string);
  }

  
  // load samples file and extract sample messages
  buffer.erase();
  load_file(samples_file, buffer);
  if(_doc.Parse(buffer.c_str()).HasParseError()){
    ss << "Error ! " << __FILE__ << "," << __LINE__ << " Invalid JSON in file " << samples_file << std::endl;
    error_string = ss.str();
    throw std::runtime_error(error_string);
  }
  mdclog_write(MDCLOG_DEBUG, "Loaded sample file %s\n", samples_file.c_str());

  StringBuffer s_buffer;
  Writer<StringBuffer> writer(s_buffer);
  
  // extract notify sample message
  ref = NULL;
  schema_key = "/notify_policy_message";
  ref = Pointer(schema_key.c_str()).Get(_doc);
  if(! ref){
    ss << "Error ! " << __FILE__ << "," << __LINE__ << " Could not find key " << schema_key << std::endl;
    error_string = ss.str();
    throw std::runtime_error(error_string);
  }
  (*ref).Accept(writer);
  if(notify_message_.Parse(s_buffer.GetString()).HasParseError()){
    ss << "Error ! " << __FILE__ << "," << __LINE__ << " Invalid JSON snippet at key :  " << schema_key  << std::endl;
    error_string = ss.str();
    throw std::runtime_error(error_string);
  }
      
						    
  mdclog_write(MDCLOG_DEBUG, "Loaded sample message for notification policy");
  
  // extract metrics sample message
  ref = NULL;
  schema_key = "/metrics";
  ref = Pointer(schema_key.c_str()).Get(_doc);
  if(! ref){
    ss << "Error ! " << __FILE__ << "," << __LINE__ << " Could not find key " << schema_key << std::endl;
    error_string = ss.str();
    throw std::runtime_error(error_string);
  }
  s_buffer.Clear();
  writer.Reset(s_buffer);
  (*ref).Accept(writer);
  if(metrics_message_.Parse(s_buffer.GetString()).HasParseError()){
    ss << "Error ! " << __FILE__ << "," << __LINE__ << " Invalid JSON snippet at key :  " << schema_key<< std::endl;
    error_string = ss.str();
    throw std::runtime_error(error_string);
  }

  
  mdclog_write(MDCLOG_DEBUG, "Loaded sample message for metrics");
  
  // set the keys we extract and update 
  setPolicyVars();
  
  //instantiate the core policy plugin
  for(unsigned int i = 0; i < num_instances; i++){
    instantiate_protector_plugin(report_only);
  }
  
};



bool admission::load_schema(const std::string & schema_file, const std::string & schema_key, std::unique_ptr<SchemaDocument> & schema_ref, std::unique_ptr<SchemaValidator> &validator_ref){

  std::string buffer;
  Value *ref;
  Document doc;
  std::stringstream ss;
  bool res;
  
  // load policy schema file
  res = load_file(schema_file, buffer);
  if(!res){
    return false;
  }
  
  if(doc.Parse(buffer.c_str()).HasParseError()){
    ss << "Error ! " << __FILE__ << "," << __LINE__ << " Invalid JSON in file " << schema_file << std::endl;
    error_string = ss.str();
    return false;
  }
  mdclog_write(MDCLOG_DEBUG, "Loaded schema file %s\n", schema_file.c_str());

  ref = NULL;
  Pointer p(schema_key.c_str());
  ref = p.Get(doc);
  if(! ref){
    ss << "Error ! " << __FILE__ << "," << __LINE__ << " Could not find key " << schema_key << " in file " << schema_file <<  std::endl;
    error_string = ss.str();
    return false;
  }

  schema_ref = std::make_unique<SchemaDocument>((*ref));
  validator_ref = std::make_unique<SchemaValidator>((*schema_ref));  
  mdclog_write(MDCLOG_DEBUG, "Loaded schema and validator for %s\n", schema_file.c_str());

  return true;

}

void admission::instantiate_protector_plugin(bool mode){
  _plugin_instances.emplace_back(0, 60, 5000, 20, mode);
}

admission::~admission(void){
  counters.clear();
 
}

bool admission::load_file(std::string input_file, std::string &  contents ){
  
  std::FILE *fp ;
  try{
    fp = std::fopen(input_file.c_str(), "rb");
  }
  catch(std::exception &e){
    error_string = "Error opening input  file " + input_file + " Reason = " + e.what();
    return false;
  } 
  
  if (fp){
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
  }
  
  else{
    error_string = "Error opening input  file " + input_file;
    return false;
  }

  return true;
}


std::string admission::getName(void){
  return std::string("admission control policy");
}

// Function sets path for each key
// in json object for set/get policy JSON
// and metrics payload string. This way, we do not create
// entire JSON from scratch, but rather just update the relevant portions
void admission::setPolicyVars(void){

  // generic variables in the policy that are re-used
  generic_policy_vars.insert(std::pair<std::string, Pointer>("policy_type_id", Pointer("/policy_type_id")));
  generic_policy_vars.insert(std::pair<std::string, Pointer>("policy_instance_id", Pointer("/policy_instance_id")));
  generic_policy_vars.insert(std::pair<std::string, Pointer>("status", Pointer("/status")));
  generic_policy_vars.insert(std::pair<std::string, Pointer>("message", Pointer("/message")));
  generic_policy_vars.insert(std::pair<std::string, Pointer>("operation", Pointer("/operation")));
  generic_policy_vars.insert(std::pair<std::string, Pointer>("xapp_id", Pointer("/handler_id")));
  
  //variables in the sliding window policy
  window_policy_vars.insert(std::pair<std::string, Pointer>("class", Pointer("/payload/class")));
  window_policy_vars.insert(std::pair<std::string, Pointer>("enforce", Pointer("/payload/enforce")));
  window_policy_vars.insert(std::pair<std::string, Pointer>("window_length", Pointer("/payload/window_length")));
  window_policy_vars.insert(std::pair<std::string, Pointer>("trigger_threshold", Pointer("/payload/trigger_threshold")));
  window_policy_vars.insert(std::pair<std::string, Pointer>("blocking_rate", Pointer("/payload/blocking_rate")));


  // variables in each metric message
  metric_vars.insert(std::pair<std::string, Pointer>("class", Pointer("/event/measurementFields/additionalFields/Class Id")));
  metric_vars.insert(std::pair<std::string, Pointer>("request_count", Pointer("/event/measurementFields/additionalFields/SgNB Request Count")));
  metric_vars.insert(std::pair<std::string, Pointer>("accept_count", Pointer("/event/measurementFields/additionalFields/SgNB Accept Count")));
  metric_vars.insert(std::pair<std::string, Pointer>("report_interval", Pointer("/event/measurementFields/measurementInterval")));
  metric_vars.insert(std::pair<std::string, Pointer>("epoch", Pointer("/event/commonHeader/startEpochMicrosec")));


  // set xapp id in return message
  generic_policy_vars["xapp_id"].Set(notify_message_, _xapp_id.c_str());
  
  // Set up the counters for metrics
  auto ts = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
  auto epoch = ts.time_since_epoch();
  auto val = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
  prev_time_stamp = val.count();

}

// returns the plugin core 
protector * admission::get_protector_instance(unsigned int index){
  if (index > _plugin_instances.size() -1){
    mdclog_write(MDCLOG_ERR, "%s, %d: Error . Requested index %u exceeds number of plugin instances %lu", __FILE__, __LINE__, index, _plugin_instances.size());
    return NULL;
  }
  else{
    return &_plugin_instances[index];
  }
};


// control plane function to set policy in plugin
// create policy returns successful if no previous policy with same class id exists and values are valid
// delete policy returns successful if policy table contains same policy instance id
// update policy returns successful if policy table contains same policy instance id,  and values are valid
bool admission::setPolicy(const char * message, int message_length, std::string & response){

  bool res = false;
  std::string policy_instance_id;
  std::string status_message, operation;
  std::stringstream ss;
  
  // policy key variables
  bool enforce = false;
  int window_size = -1, trigger_threshold = -1, class_id = -1;
  double blocking_rate = -1;

  // reset the validator
  (*downstream_validator_).Reset();
  
  // step 1: verify JSON
  Document doc;
  if(doc.Parse(message).HasParseError()){
    status_message.assign("Invalid JSON");
  }
  
  // Validate against our JSON input schema
  else if (!doc.Accept((*downstream_validator_))){
    StringBuffer sb;
    (*downstream_validator_).GetInvalidSchemaPointer().StringifyUriFragment(sb);
    std::string failed_message = std::string("\"message\": \"Schema Violation:") + std::string(sb.GetString());
    failed_message += std::string(" Invalid keyword :") + std::string((*downstream_validator_).GetInvalidSchemaKeyword()) + " \"";
    sb.Clear();
    (*downstream_validator_).GetInvalidDocumentPointer().StringifyUriFragment(sb);
    failed_message += std::string(" Invalid document :") + std::string(sb.GetString());
    status_message.assign(failed_message);

  }
  else{

    // step 2: extract the standard keys expected in all downstream
    // messages : policy_instance
    Value * ref;
    
    for(auto const &e : generic_policy_vars){
      ref = NULL;
      ref = e.second.Get(doc);

      // this key can be simultaneously put in notify 
      if (ref != NULL && e.first == "policy_type_id" ){
  	e.second.Set(notify_message_, ref->GetInt());
      }

      // this key can be simultaneously put in notify 
      else if (ref != NULL && e.first == "policy_instance_id" ){
  	e.second.Set(notify_message_, ref->GetString());
	policy_instance_id = ref->GetString();
      }
      else if (ref != NULL && e.first == "operation"){
	operation = ref->GetString();
      }
    }


    // do we have this policy ?
    auto it = policy_table.find(policy_instance_id);
    res = true;

    // if operation is create and policy already present, simply return with OK ?
    // we may get the same create policy multiple times due to race conditions when the
    // xapp starts up
    if(operation == "CREATE" && it != policy_table.end()){
      res = true;
    }
    else{
      
      if (operation == "DELETE" || operation == "UPDATE"){
	// don't proceed if policy not found 
	if(it == policy_table.end()){
	  ss <<" No policy instance = " << policy_instance_id << " found. Cannot perform operation = " << operation << std::endl;
	  status_message = ss.str();
	  res = false;
	}
	else{
	  class_id = it->second; // used if delete 
	}
      }
      
      if (res){
	// perform the operation
	res = false;
	
	if (operation == "DELETE"){
	  res = _plugin_instances[0].delete_policy(class_id);
	  if(res){
	    ss <<"Policy instance id " << policy_instance_id << " successfully deleted" << std::endl;
	    status_message = ss.str();
	    policy_table.erase(policy_instance_id);
	  }
	  else{
	    status_message = _plugin_instances[0].get_error();
	  }
	}
	
	else if (operation == "CREATE" or operation == "UPDATE"){
	  // initialize policy params to invalid
	  window_size = -1;
	  class_id = -1;
	  trigger_threshold = -1;
	  blocking_rate = -1;
	  
	  // get values for policy keys
	  for(auto const& e: window_policy_vars){
	    ref = NULL;
	    ref = e.second.Get(doc);
	    if (ref == NULL){
	      continue;
	    }
	    if(e.first == "enforce"){
	      enforce = ref->GetBool();
	    }
	    else if (e.first == "window_length"){
	      window_size = ref->GetInt();
	    }
	    else if (e.first == "blocking_rate"){
	      blocking_rate = ref->GetDouble();
	    }
	    else if (e.first == "trigger_threshold"){
	      trigger_threshold = ref->GetInt();
	    }
	    else if (e.first == "class"){
	      class_id = ref->GetInt();
	    } 
	  }
	  
	  
	  if(operation == "CREATE"){
	    res = _plugin_instances[0].add_policy(enforce, window_size, trigger_threshold, blocking_rate, class_id);
	    status_message.assign(_plugin_instances[0].get_error());
	    
	    if(res == true){
	      // add to policy list
	      policy_table.insert(std::pair<std::string, int>(policy_instance_id, class_id));
	    }
	  }
	  else if (operation == "UPDATE"){
	    res = _plugin_instances[0].configure(enforce, window_size, trigger_threshold, blocking_rate, class_id);
	    status_message.assign(_plugin_instances[0].get_error()); 
	  }
	  
	}
      }
    }
    
    if(res == true)
      status_message.assign("SUCCESS");
  }
  

  // generate response 
  //generic_policy_vars["message"].Set(notify_message_, status_message.c_str());
  
  if(res == false){
    generic_policy_vars["status"].Set(notify_message_, "ERROR");
  }
  else {
    if(operation == "DELETE"){
      generic_policy_vars["status"].Set(notify_message_, "DELETED");
    }
    else{
      generic_policy_vars["status"].Set(notify_message_, "OK");
    }
  }
  

  StringBuffer s_buffer;
  Writer<StringBuffer> writer(s_buffer);
  notify_message_.Accept(writer);
  response.assign(s_buffer.GetString(), s_buffer.GetLength());
  mdclog_write(MDCLOG_DEBUG, "Set Policy Response = %s\n", response.c_str());
  return res;
  
};

// control plane function to retreive policy set in plugin
// This is just a placeholder. Still TBD ..... 
bool admission::getPolicy(const char * message, int message_length, std::string & response){
  return true;
								       
}


// control plane function to retreive metrics from plugin
// crafts into ves schema based JSON payload
int admission::getMetrics(std::vector<std::string> & response_vector){

  int res;
  // the list of active policies on the protector plugin can
  // dynamically change.


  // run through active policies
  for(auto const &e: policy_table){
    int id = e.second;
    std:: string response;
    process_counters(id, response);
    response_vector.emplace_back(response);
  }

  // also account for default policy
  int id = -1;
  std::string response;
  process_counters(id, response);
  response_vector.emplace_back(response);

  
  // clear out counters for expired policies
  for (auto const &e : counters){
    
    if (e.first != -1 && ! _plugin_instances[0].is_active(e.first)){
      counters.erase(e.first);
    }
  }

  return 0;
}


void admission::process_counters(int id, std::string & response){


  long request_count, accept_count, curr_timestamp,  time_interval;
  
  long int requests = _plugin_instances[0].get_requests(id);
  long int rejects = _plugin_instances[0].get_rejects(id);
    
   
  std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> ts;
  ts = std::chrono::time_point_cast<std::chrono::microseconds> (std::chrono::system_clock::now());
  curr_timestamp =ts.time_since_epoch().count();
  
  // do we have counters for this policy ?
  auto prev_it = counters.find(id);
  if(prev_it == counters.end()){

    // new policy seeing for first time 
    request_count = requests;
    accept_count = requests - rejects;
    time_interval = 0;
    
    // store
    counters.insert(std::pair<int, std::vector<long int>>(id, std::vector<long int>({requests, rejects, curr_timestamp})));
  }
  else{
    request_count = requests - prev_it->second[0];
    accept_count = request_count - (rejects - prev_it->second[1]);
    time_interval = ceil((curr_timestamp - prev_it->second[2])/1e+6); // seconds
    
    // update history
    prev_it->second[0] = requests;
    prev_it->second[1] = rejects;
    prev_it->second[2] = curr_timestamp;
  }
  
  // generate string and add to response vector
  metric_vars["class"].Set(metrics_message_, std::to_string(id).c_str());
  metric_vars["request_count"].Set(metrics_message_, std::to_string(request_count).c_str());
  metric_vars["accept_count"].Set(metrics_message_, std::to_string(accept_count).c_str());
  metric_vars["epoch"].Set(metrics_message_, std::to_string(curr_timestamp).c_str());
  metric_vars["report_interval"].Set(metrics_message_, std::to_string(time_interval).c_str());
  
  StringBuffer sb_buffer;
  Writer<StringBuffer> writer(sb_buffer);  
  metrics_message_.Accept(writer);
  response.assign(sb_buffer.GetString(), sb_buffer.GetLength());
  

}
