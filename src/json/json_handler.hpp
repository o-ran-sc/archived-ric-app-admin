
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

#pragma once
#ifndef JSON_HANDLER
#define JSON_HANDLER

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/schema.h>
#include <iostream>
#include <unordered_map>
#include <map>
#include <stdio.h>
#include <memory>
#include <vector>
#include <mdclog/mdclog.h>
#include <queue>


#define MAX_QUEUE_SIZE 100

using namespace rapidjson;


struct DataContainer {  
  enum  Types {boolean, integer, uinteger, big_integer, ubig_integer, real, str} tag ;    
  struct  {
    union  {
      bool b;
      int i;
      unsigned int ui;
      long int l;
      unsigned long int ul;
      double f;
    } ;
    std::string s;
  } value;  
};
  
class TrieNode;

class TrieNode{
public:

  TrieNode (int);
  TrieNode( std::string );

  void set_type(DataContainer::Types);
  int get_type(void) const {return _val.tag;} ;

  void set_value(bool val); 
  void set_value(int val);
  void set_value(unsigned int val);
  
  void set_value(long  int );
  void set_value(unsigned long int );
  void set_value(double val);
  void set_value(const char *);
  void set_value(std::string val);
  void set_value(const char * , size_t );

  void print_id(void);
  void print_value(void);
  void add_child(TrieNode *);
  
  DataContainer const * get_value(void) const  { return & _val; };
  DataContainer const *  get_id(void) const  { return & _id;};

  std::vector<TrieNode *> &   get_children(void){ return _children;} ;
  std::string &  name(void) {return _name; };
  bool is_child(void){  return _children.size() ? false : true; };
  
private:
  std::vector<TrieNode *> _children;
  int val_type;
  std::string _name;
  DataContainer _id;
  DataContainer _val;
};
  
class jsonHandler {

public:
  jsonHandler(void);

  
  void load_schema(std::string);
  void load_schema(std::string, TrieNode *root);
  void load_schema(std::string , std::vector<std::string> & );

  void load_buffer(std::string);
  void  load_buffer(std::string, TrieNode * );
  std::string get_buffer(void);

  int get_values(const  char *, int , std::string & , TrieNode *, std::vector<TrieNode *> & );
  int get_values( std::string & , TrieNode *, std::vector<TrieNode *> & );
  
  int set_values (const char *, int, std::string & , std::vector<TrieNode *>);
  int set_values (std::string & , std::vector<TrieNode *>);
  
  bool is_valid(const char *, int, std::string &);
  
private:

  void load_file(std::string, std::string &);

  bool traverse_doc(Value &, TrieNode *, bool, std::string &, std::vector<TrieNode *> & );

  bool find_member(const std::string, std::string &, std::vector<std::string> &, Value & );

  bool find_member(const std::string, std::string &, TrieNode *, Value &);
  bool find_member(Value &,  std::string &, TrieNode *, Value &);

  bool add_array_objects(std::queue<Value> &, Value &);

  bool _is_root, _is_schema, _is_buffer;

  std::unique_ptr<SchemaDocument> _ref_schema_doc;
  std::map <std::string, DataContainer> _key_value_pairs;
  std::string _contents;
  std::string _buffer;
  
};


#endif
