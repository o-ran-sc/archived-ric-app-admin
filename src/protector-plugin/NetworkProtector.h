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
#include <sgnb_addition_request.hpp>  // to decode the X2AP payload
#include <sgnb_addition_response.hpp> // to respond

#ifdef __GNUC__
#define likely(x)  __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

class protector
{
public: 
  
  protector( bool enforce, int windowSize_, int threshold_, double blockRate_);

  bool operator()(unsigned char *, size_t , unsigned char *, size_t *);
  
  bool configure(bool enforce, int windowSize_, int threshold_, double blockRate_);
  void clear();
  bool selectiveBlock();

  unsigned long int get_requests(void) const;
  unsigned long int get_rejects(void) const;
  std::string get_error(void) { return error_string;};
  
private:
  bool m_enforce; // whether to execute logic or not 
  int m_counter;			// count the # of attaching access
  int m_windowSize;			// time in seconds window for the # of counts
  int m_threshold;			// count above which we start enforcing if enforce set
  double m_blockRate;		// % of rejecting rate for counter > threshold
  time_t m_timeWindow;      // time active window started
  unsigned long int m_req; // number of requests
  unsigned long int m_rej; // number of rejects

  std::unique_ptr<sliding_window> m_window_ref;
  std::unique_ptr<std::mutex> m_access;
  sgnb_addition_helper sgnb_data;
  sgnb_addition_request sgnb_req;
  sgnb_addition_response sgnb_resp;
  
  std::string error_string;
};

#endif
