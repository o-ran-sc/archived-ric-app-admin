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

#include "sliding_window.hpp"


sliding_window::sliding_window(unsigned int window_size){

  if (window_size < MIN_WINDOW_SIZE){
    std::stringstream ss;
    ss << "Error ::" << __FILE__ << ","<< __LINE__ << " window size must be >= " << MIN_WINDOW_SIZE << " Specified = " << window_size << std::endl;
    error_string = ss.str();
    throw std::logic_error(error_string);
  }

  if (window_size > MAX_WINDOW_SIZE){
    std::stringstream ss;
    ss << "Error ::" << __FILE__ << ","<< __LINE__ << " window size must be <= " << MAX_WINDOW_SIZE << " Specified = " << window_size << std::endl;
    std::string error_string = ss.str();
    throw std::logic_error(error_string);
  }

  window_size_ = window_size;
  sliding_window_.resize(window_size_);
  
  head = window_size_ -1;
  tail = 0;
  net_events = 0;
  leading_edge_ = std::chrono::steady_clock::now();
}

// expect to be called very frequently ....
bool  sliding_window::update_window(unsigned int events){
  // get current time
  auto current_edge = std::chrono::steady_clock::now();
  
  int shift = std::chrono::duration_cast<std::chrono::seconds>(current_edge - leading_edge_).count();

  if (shift < 0){
    // we do not update any events if they happened in the past
    // by more than unit of window .. (1 second currently)
    return false;
  }

  // Advance window if shift >= 1
  if (shift >= 1){
    leading_edge_ = current_edge;
  }
  
  if (shift >= window_size_){
    // gap between current time and leading edge
    // exceeds window size. Clear everything ...
    tail = 0;
    head = window_size_ -1;
    std::fill(sliding_window_.begin(), sliding_window_.end(), 0);
    sliding_window_[head] = events ;
    net_events = events;
    return true;
  }
  
  for(int i = 0; i < shift; i++){
    // Advance tail & head by requisite amount, reducing net event rate ..
    net_events -= sliding_window_[tail];
    sliding_window_[tail] = 0;
    tail += 1;
    if (tail >= window_size_){
      tail = 0;
    }
    
    head += 1;
    if (head >= window_size_){
      head = 0;
    }
  }

  sliding_window_[head] += events;
  net_events += events;

  //std::cout <<net_events << "," << window_size_ << "," << shift << std::endl;  
  return true;
}

// modifies window size. resets all parameters 
bool sliding_window::resize_window(unsigned int window_size){
  if (window_size < MIN_WINDOW_SIZE || window_size > MAX_WINDOW_SIZE){
    std::stringstream ss;
    ss << "Error ::" << __FILE__ << ","<< __LINE__ << " window size must be in [ " << MIN_WINDOW_SIZE << "," << MAX_WINDOW_SIZE << "]"  << std::endl;
    error_string = ss.str();
    return false;
  }
  
  window_size_ = window_size;
  sliding_window_.clear();
  sliding_window_.resize(window_size_);
  net_events = 0;
  head = window_size_ -1;
  tail = 0;
  leading_edge_ = std::chrono::steady_clock::now();
  return true;
}

std::string  sliding_window::display_window(void){
  std::stringstream ss;
  for(unsigned int i = 0; i < sliding_window_.size(); i++){
    if (i == head){
      ss <<"[H] ";
    }
    if (i == tail){
      ss <<"[T] ";
    }
    
    ss <<sliding_window_[i] <<",";
  }
  ss <<" [Events] = " << net_events << std::endl;

  return ss.str();
}

void sliding_window::clear(void){
  
  resize_window(window_size_);
}

  
  
