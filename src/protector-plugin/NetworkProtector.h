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


#ifndef NETWORKPROTECTOR_H
#define NETWORKPROTECTOR_H

#include "sliding_window.hpp"
#include <mutex>
#include <map>
#include <sgnb_addition_request.hpp>  // to decode the X2AP payload
#include <sgnb_addition_response.hpp> // to respond

#ifdef __GNUC__
#define likely(x)  __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

// each policy corresponds to a specific X2 subscriber  profile ID
// and applies sliding window logic to UEs in that class (if enforce)
class protector_policy {
public:
  protector_policy(bool enforce=true,  int window_size=60, int threshold=10, double block_rate=10): _enforce(enforce), _window_size(window_size), _threshold(threshold), _block_rate(block_rate){
    _counter = 0;
    _req = 0;
    _rej = 0;
    _window_ref = std::make_unique<sliding_window>(_window_size);
  };

  bool _enforce;                // do we enforce policy ?
  int _counter;			// count the # of attaching access
  int _window_size;			// time in seconds window for the # of counts
  int _threshold;			// count above which we start enforcing if enforce set
  double _block_rate;		// % of rejecting rate for counter > threshold
  std::unique_ptr<sliding_window> _window_ref;
  unsigned long int _req; // number of requests
  unsigned long int _rej; // number of rejects

};

  
class protector
{
public: 
  
  protector( bool rep=true);
  protector(bool enforce, int window_size, int threshold, double block_rate, bool rep=true);
  bool operator()(unsigned char *, size_t , unsigned char *, size_t *);
  
  bool configure(bool enforce, int windowSize_, int threshold_, double blockRate_, int id);
  bool add_policy (bool enforce, int windowSize_, int threshold_, double blockRate_, int id);
  bool delete_policy(int id);
  bool query_policy(int , std::vector<double> &);
  void get_active_policies(std::vector<int> & );
  bool is_active(int id);
  
  void clear();
  bool selectiveBlock(double);

  long int get_requests(int id) const;
  long int get_rejects(int id) const;
  std::string get_error(void) { return error_string;};
  
private:

  std::map<int, protector_policy> policy_list;
  std::unique_ptr<std::mutex> m_access;

  sgnb_addition_helper sgnb_data;
  sgnb_addition_request sgnb_req;
  sgnb_addition_response sgnb_resp;

  unsigned long int net_requests = 0;
  unsigned long int net_rejects = 0;
  
  std::string error_string;
  bool report_mode_only;
};

#endif
