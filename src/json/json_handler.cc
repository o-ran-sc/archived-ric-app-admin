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
/*
 Author : Ashwin Sridharan

  
*/

#include <json_handler.hpp>
#include <cstdio>

TrieNode::TrieNode(int val): val_type(-1){
  _id.tag = DataContainer::Types::integer;
  _id.value.i = val;
}

TrieNode::TrieNode( std::string val) : val_type(-1){
  _id.tag = DataContainer::Types::str;
  _id.value.s.assign(val);
}

void TrieNode::set_value(const char * val){
  _val.tag = DataContainer::Types::str;
  _val.value.s.assign(val);  
}


void TrieNode::set_value(bool val){
  _val.tag = DataContainer::Types::boolean;
  _val.value.b = val;
  
}

void TrieNode::set_value(int val){
  _val.tag = DataContainer::Types::integer;
  _val.value.i = val;
  //std::cout <<"Assigned integer " << val << std::endl;

    
}

void TrieNode::set_value(unsigned int val){
  _val.tag = DataContainer::Types::uinteger;
  _val.value.ui = val;
  
}

void TrieNode::set_value( long int val){
  _val.tag = DataContainer::Types::big_integer;
  _val.value.l = val;
}

void TrieNode::set_value(unsigned  long int val){
  _val.tag = DataContainer::Types::ubig_integer;
  _val.value.ul = val;
}

void TrieNode::set_value(double val){
  _val.tag = DataContainer::Types::real;
  _val.value.f = val;
    
}


void TrieNode::set_value(std::string val){
  _val.tag = DataContainer::Types::str;
  _val.value.s.assign(val);
}

void TrieNode::set_value(const char * c, size_t len){
  _val.tag = DataContainer::Types::str;
  _val.value.s.assign(c, len);
}

void TrieNode::add_child(TrieNode * node){
  _children.push_back(node);
};



void TrieNode::print_id(void){
  switch(_id.tag){

  case DataContainer::Types::integer :
    std::cout <<"Type = " << _id.tag << " Value = " << _id.value.i << std::endl;
    break;
  case DataContainer::Types::str :
    std::cout <<"Type = " << _id.tag << " Value = " << _id.value.s << std::endl;
    break;
  default:
    std::cerr<< "Error ! ID  not set or unknown type " << _id.tag;
  }
};

void TrieNode::print_value(void){
  switch(_val.tag){

  case DataContainer::Types::boolean :
    std::cout <<"Type = " << _val.tag << " Value = " << _val.value.b << std::endl;
    break;
  case DataContainer::Types::integer :
    std::cout <<"Type = " << _val.tag << " Value = " << _val.value.i << std::endl;
    break;
  case DataContainer::Types::uinteger :
    std::cout <<"Type = " << _val.tag << " Value = " << _val.value.ui << std::endl;
    break;
  case DataContainer::Types::big_integer :
    std::cout <<"Type = " << _val.tag << " Value = " << _val.value.l << std::endl;
    break;
  case DataContainer::Types::ubig_integer :
    std::cout <<"Type = " << _val.tag << " Value = " << _val.value.ul << std::endl;
    break;
  case DataContainer::Types::real :
    std::cout <<"Type = " << _val.tag << " Value = " << _val.value.f << std::endl;
    break;
  case DataContainer::Types::str :
    std::cout <<"Type = " << _val.tag << " Value = " << _val.value.s << std::endl;
    break;
  default:
    std::cerr<< "Error ! Value not set or unknown type " << _val.tag;
  }
};
    

jsonHandler::jsonHandler(void):_is_root(false), _is_schema(false), _is_buffer(false){
 
};




void jsonHandler::load_file(std::string input_file, std::string &  contents ){

  std::FILE *fp ;
  try{
    fp = std::fopen(input_file.c_str(), "rb");
  }
  catch(std::exception &e){
    std::string error_string = "Error opening input schema file " + input_file;
    throw std::runtime_error(error_string);
  } 
  
  if (fp){
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
  }
  
  else{
    std::string error_string = "Error opening input  file " + input_file;
    throw std::runtime_error(error_string);
  }
  
}



void jsonHandler::load_schema(std::string input_file){

  load_file(input_file, _contents);
  Document _doc;
  if (_doc.Parse(_contents.c_str()).HasParseError()){
    std::string error_string = input_file + " is invalid JSON" ;
    throw std::runtime_error(error_string);
  }
  
  _ref_schema_doc= std::make_unique<SchemaDocument>(_doc);
  _is_schema = true;
  
  
}

void  jsonHandler::load_schema(std::string input_file, TrieNode * root){

  load_file(input_file, _contents);
  std::string response;
  Document _doc;
  if (_doc.Parse(_contents.c_str()).HasParseError()){
    std::string error_string = input_file + " is invalid JSON" ;
    throw std::runtime_error(error_string);
  }
  
  // Get message schema
  bool res;
  Value  _schema_root;
  Value &_root = _doc;
  res = find_member(_root,  response, root, _schema_root);
  if (res == false){
    throw std::runtime_error(response);
  }

  _ref_schema_doc= std::make_unique<SchemaDocument>(_schema_root);
  _is_schema = true;
  
}

void jsonHandler::load_buffer(std::string input_file){
  
  load_file(input_file, _buffer);
  Document _doc;
  if (_doc.Parse(_buffer.c_str()).HasParseError()){
    std::string error_string = input_file + " is invalid JSON" ;
    throw std::runtime_error(error_string);
  }   
  _is_buffer = true;
  
}

void jsonHandler::load_buffer(std::string input_file, TrieNode * root){
  
  load_file(input_file, _buffer);
  Document _doc;
  std::string response;
  
  if (_doc.Parse(_buffer.c_str()).HasParseError()){
    std::string error_string = input_file + " is invalid JSON" ;
    throw std::runtime_error(error_string);
  }   

  bool res;
  Value _buffer_root;
  res = find_member(_doc, response, root, _buffer_root);
  if(res == false){
    throw std::runtime_error(response);
  }

  StringBuffer out_buffer;
  Writer<StringBuffer> writer(out_buffer);
  _buffer_root.Accept(writer);
  _buffer.assign(out_buffer.GetString(), out_buffer.GetLength());
  _is_buffer = true;
}


std::string jsonHandler::get_buffer(void){
  std::string response;  
  if (_is_buffer){
    response.assign(_buffer);
  }
  else{
    response = "";
  }
  
  return response;
}


  



bool jsonHandler::find_member(const std::string schema, std::string & response, TrieNode * root, Value & TargetVal){

  Document doc;
  std::string contents(schema);
  
  if(doc.Parse(contents.c_str()).HasParseError()){
    response.assign("Error Parsing JSON File");
    return false;
  }

  return find_member(doc, response, root, TargetVal);
  return true;
};


bool jsonHandler::find_member(Value & doc_root, std::string & response, TrieNode * root, Value & TargetVal){

  if (!root){
    response.assign("Null Trie root node");
    return false;
  }
  //std::cout <<"LOoking for schema root" << std::endl;

  Value & json_node = doc_root;
  TrieNode * trie_node = root;  
  Value::MemberIterator itr;
  
  while(1){

    DataContainer const * d  = trie_node->get_id();
    if (! d){
      response.assign("Error could not find any id for trie node ");
      return false;
    }

    if (d->tag == DataContainer::Types::integer && json_node.IsArray()){
      if (json_node.Size() < d->value.i){
  	response.assign("Error json array size ");
  	response +=  std::to_string(json_node.Size())  +  " is smaller than trie node index " + std::to_string( d->value.i);
  	return false;
      }
      
      if (trie_node->is_child()){
  	response.assign("Error child trie points to an array ? ");
  	return false;
      }

      trie_node = trie_node->get_children()[0];
      json_node = json_node[d->value.i];
    }
    else if (d->tag == DataContainer::Types::str && json_node.IsObject()){
      
      itr = json_node.FindMember(d->value.s.c_str());
      if (itr == json_node.MemberEnd()){
  	response.assign("Error ! Could not find key = ");
  	response +=  d->value.s;
  	return false;
      }
      if (trie_node->is_child()){
  	// reached end of trie 
  	if (itr->value.IsObject()){
  	  TargetVal = itr->value.GetObject();
	  //std::cout <<"Reached root = " << itr->name.GetString() << std::endl;
  	}
  	else if (itr->value.IsArray()){
  	  TargetVal = itr->value.GetArray();
  	}
  	else{
  	  response.assign("Error ! JSON node selected  must be object or array in current version");
  	  std::cerr << response << std::endl;
  	  return false;
  	}
  	break;
      }
      else{
  	trie_node = trie_node->get_children()[0];
  	trie_node->print_id();
	
  	if (itr->value.IsObject()){
  	  json_node = itr->value.GetObject();
  	}
  	else if (itr->value.IsArray()){
  	  json_node = itr->value.GetArray();
  	}
  	else{
  	  std::string error_string= " Path must be an object or array";
  	  response.assign(error_string);
  	  return false;
  	}
      }
    }
    else{
      std::string error_string = "Mismatch when setting root  : Trie node is of type = " + std::to_string (d->tag) + " and json node is of type = " + std::to_string(json_node.GetType());
      response.assign(error_string);
      return false;
    }

  }

  return true;

}

bool jsonHandler::is_valid(const char *message, int message_length,  std::string & response){

  Document doc;
  if (! _is_schema){
    return false;
  }

  SchemaValidator validator(*(_ref_schema_doc.get()));
  
  // ensure message has terminator by translating to string ?
  std::string message_s(message, message_length);

  
  // validate json 
  if (doc.Parse(message_s.c_str()).HasParseError()){
    
    // return error message
    std::string failed_message = "\"message\": \"Invalid JSON\"";
    response.assign( failed_message );
    return false;
  }

  
  // Validate against our JSON input schema
  if (!doc.Accept(validator)){
    
    StringBuffer sb;
    validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
    std::string failed_message = std::string("\"message\": \"Schema Violation:") + std::string(sb.GetString());
    failed_message += std::string(" Invalid keyword :") + std::string(validator.GetInvalidSchemaKeyword()) + " \"";
    response.assign(failed_message);
    return false;
    
  }
  response.assign("SUCCESS");
  return true;
  
  
}



// should be thread safe since it can be expected to be called from multiple threads
// only static external variable referenced is the schema (which should be read-only and hence ok ?)

// Returns 0 if success
// -1 if invalid json
// -2 if invalid schema (assuming schema provided)
// -3 unknown key
// -4 no buffer available

int jsonHandler::get_values(const  char *message, int message_length, std::string & response, TrieNode * root, std::vector<TrieNode *> & response_list){
  
  Document doc;

  // ensure message has terminator by translating to string ?
  std::string message_s(message, message_length);
  
  // validate json 
  if (doc.Parse(message_s.c_str()).HasParseError()){
    
    // return error message
    response.assign("Invalid JSON");
    return -1;
  }
  
  // Validate against our JSON input schema
  if ( _is_schema){
    SchemaValidator validator(*(_ref_schema_doc.get()));
    
    if (!doc.Accept(validator)){
    
      StringBuffer sb;
      validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
      response = std::string("Schema Violation ") + std::string(sb.GetString()) ;
      response  += std::string(" Invalid keyword = ") + std::string(validator.GetInvalidSchemaKeyword()) + " \"";
      return -2;
    }

  }
  
  Value & doc_root = doc;
  bool res = traverse_doc(doc_root, root, true, response, response_list);
  if (!res){
    return -3;
  }
    
  response.assign("SUCCESS");
  return 0;
}


int jsonHandler::get_values( std::string & response, TrieNode * root, std::vector<TrieNode *> & response_list){
  int res;
  if (_is_buffer){
    Document _doc;
    _doc.Parse(_buffer.c_str());
    Value & _buffer_root = _doc;
    
    res = traverse_doc(_buffer_root, root, true, response,  response_list);
    return res;
  }
  else{
    response = "Error !  No buffer loaded in json object for get";
    return -4;
  }
}


// If in get mode, return all values we can get
// If in set mode, return false if we cannot set a value 
bool jsonHandler::traverse_doc(Value & json_node, TrieNode * trie_node, bool get, std::string & response, std::vector<TrieNode* > & response_list ){

  if (!trie_node){
    response.assign(" Null Trie node ");
    return  false;
  }

  bool res;
  
  DataContainer const * d  = trie_node->get_id();
  if (! d){
    response.assign(" Error could not find any id for trie node");
    return false;
  }
 
  Value::MemberIterator itr;
  

  if (d->tag == DataContainer::Types::integer && json_node.IsArray()){
    if (json_node.Size() < d->value.i){
      response = "Error json array size " + std::to_string( json_node.Size()) +  " is smaller than trie node index " + std::to_string( d->value.i);
      return false;
    }
    
    if (trie_node->is_child()){
      response.assign("Error child trie points to an array ? ");
      return false;
    }
    
    for (auto & e: trie_node->get_children()){
      res = traverse_doc(json_node[d->value.i], e, get, response, response_list);
      if (!res && ! get){				
  	// if not in get mode and we hit a not found
  	// don't go any further, else move to next ...
  	return res;
      }
    }
  }
  
  else if (d->tag == DataContainer::Types::str  && json_node.IsObject()){
    itr = json_node.FindMember(d->value.s.c_str());
    
    if (itr == json_node.MemberEnd()){
      response = "Error ! Could not find key " + d->value.s;
      return false;
    }

    if (trie_node->is_child()){
      // end of the line : do we get or set values  ?
      bool is_set = false;
      if (get){
  	if (itr->value.IsBool()){
  	  trie_node->set_value(itr->value.GetBool());
  	  is_set = true;
  	}
  	else if (itr->value.IsInt()){
  	  trie_node->set_value(itr->value.GetInt());
  	  is_set = true;
  	}
  	else if(itr->value.IsUint()){
  	  trie_node->set_value(static_cast<unsigned int>(itr->value.GetUint()));
  	  is_set = true;
  	}
  	else if(itr->value.IsUint64()){
  	  trie_node->set_value(static_cast<unsigned long int>(itr->value.GetUint64()));
  	  is_set = true;
  	}
  	else if (itr->value.IsInt64()){
  	  trie_node->set_value(static_cast<long int>(itr->value.GetInt64()));
  	  is_set = true;
  	}
  	else if ( itr->value.IsDouble()){
  	  trie_node->set_value(itr->value.GetDouble());
  	  is_set = true;
  	}
  	else if ( itr->value.IsString()){
  	  trie_node->set_value(itr->value.GetString(), itr->value.GetStringLength());
  	  is_set = true;
  	}
  	else{
  	  response =  " json node corresponding to child node key  must of type bool, int or string. Is of type = "  + std::to_string(itr->value.GetType());
  	  return false;
  	}

  	if (is_set){
  	  response_list.push_back(trie_node);
  	}
	
  	//std::cout <<"Set value of child node with key = " << d->value.s.c_str() << " Type = " << trie_node->get_type()  << std::endl;
	
      }
      else{
  	DataContainer const  * d_val = trie_node->get_value();
  	if (d_val->tag == DataContainer::Types::boolean){
  	  itr->value.SetBool(d_val->value.b);
  	}
  	else if (d_val->tag == DataContainer::Types::integer){
  	  itr->value.SetInt(d_val->value.i);

  	}
  	else if (d_val->tag == DataContainer::Types::uinteger){
  	  itr->value.SetUint(d_val->value.ui);

  	}
  	else if (d_val->tag == DataContainer::Types::big_integer){
  	  itr->value.SetInt64(d_val->value.l);
  	}
  	else if (d_val->tag == DataContainer::Types::ubig_integer){
  	  itr->value.SetUint64(d_val->value.ul);
  	}
  	else if (d_val->tag == DataContainer::Types::real){
  	  itr->value.SetDouble(d_val->value.f);
  	}
  	else if (d_val->tag == DataContainer::Types::str){
  	  itr->value.SetString(d_val->value.s.c_str(), d_val->value.s.length());		  
  	}
  	else{
  	  response = " unknown type for child node value = " + std::to_string(d_val->tag) + " cannot set json node key = " +  d->value.s;
  	  return false;
  	}
      }      
      return true;
    }
    else{
      for (auto & e: trie_node->get_children()){
  	res = traverse_doc(itr->value, e, get, response, response_list);
  	if(res == false && ! get){
  	  return false;
  	}
      }
    }
  }
 
  else{
    response = "Mismatch : Trie node is of type = " + std::to_string(d->tag) + " while json node is of type = "  + std::to_string( json_node.GetType());
    return false;
  }

  return true;
  
 
}


int jsonHandler::set_values(const char * buffer, int len, std::string & response, std::vector<TrieNode *> root_nodes){
 
  Document doc;
  std::string message_s(buffer, len);
  
  // validate json 
  if (doc.Parse(message_s.c_str()).HasParseError()){  
    // return error message
    response.assign("Invalid JSON");
    return -1;
  }

  
  Value & doc_root = doc;
  // fake list to maintain signature for re-using traverse_doc
  // since we don't return trie nodes when setting ...
  std::vector<TrieNode *> fake_list;
  for(auto const & e: root_nodes){
    bool res = traverse_doc(doc_root, e, false, response, fake_list);
    if (!res){
      return -3;
    }
  }
  
  StringBuffer out_buffer;
  Writer<StringBuffer> writer(out_buffer);
  doc_root.Accept(writer);
  response.assign(out_buffer.GetString(), out_buffer.GetLength());
   return 0;
 }  


// wrapper if instead of providing buffer, we simply use stored json object and use it
int jsonHandler::set_values(std::string & response, std::vector<TrieNode *> root_nodes){
  if (_is_buffer){
    std::vector<TrieNode *> fake_list;
    Document _doc;
    _doc.Parse(_buffer.c_str());
    Value & _buffer_root = _doc;
    
    for(auto const & e: root_nodes){
      bool res  = traverse_doc(_buffer_root, e,  false, response, fake_list);
      if (!res){
  	return -3;
      }
    }
    
    StringBuffer out_buffer;
    Writer<StringBuffer> writer(out_buffer);
    _buffer_root.Accept(writer);
    response.assign(out_buffer.GetString(), out_buffer.GetLength());
    return 0;
  }
  else{
    response = "Error ! " + std::string( __FILE__) + "," + std::to_string(__LINE__) + " :  No buffer loaded in json object to set";
    return -1;
  }
  
}
