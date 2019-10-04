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

#ifndef SLIDING_WINDOW_
#define SLIDING_WINDOW_

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>

#define MIN_WINDOW_SIZE 1  // in seconds
#define MAX_WINDOW_SIZE 604800 // in seconds 1 week

class sliding_window {

public:
  sliding_window(unsigned int);
  bool update_window (unsigned int);
  std::string display_window(void);
  bool resize_window(unsigned int);
  std::string get_error(void) { return error_string ;};
  unsigned int net_events;
  void clear(void);
  
private:  
  std::vector<unsigned int> sliding_window_;
  unsigned int head, tail,  window_size_;
  std::chrono::time_point<std::chrono::steady_clock>  leading_edge_;
  std::string error_string;
};


#endif
