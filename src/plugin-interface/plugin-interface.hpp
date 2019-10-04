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

#pragma once
#ifndef POLICY_BASE
#define POLICY_BASE
#include <json_handler.hpp>
// Base abstract Class that provides interface to  manage plugins.
//  Interface for following actions
// -- configure policy
// -- get policy
// -- get metrics.

// All plugins should be instantiated within derivatives of this class

// Direct execution of the plugin on data is done by the message
// operator class.

class Policy
{
public:
  virtual bool setPolicy(const char *, int, std::string &) = 0;
  virtual bool getPolicy(const char *, int , std::string & ) = 0;
  virtual int getMetrics(std::string  & ) = 0;
  virtual std::string getName(void) = 0;
  virtual ~Policy(void) = 0;
  
  std::string  getError(void) const;
  void  setError(std::string &);
  
  
  
private:

  std::string _error;
  
  
};

#endif
