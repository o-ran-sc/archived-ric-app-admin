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

/* Author : Ashwin Sridharan
   Date   : Sept 2019
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <sliding_window.hpp>

TEST_CASE("Unit tests for sliding window", "sliding window"){

  SECTION("Invalid window"){
    REQUIRE_THROWS(sliding_window(-1));
    REQUIRE_THROWS(sliding_window(MAX_WINDOW_SIZE + 1));
  }

  SECTION("Windowing"){
    int window_size = 4;
    sliding_window my_window(window_size);
    REQUIRE(my_window.net_events == 0);
    int num_events = 20;
    
    for(int i = 0; i < num_events; i++){
      my_window.update_window(1);
    }
    
    REQUIRE(my_window.net_events == num_events);

    // test window shift after delay more than window size
    std::this_thread::sleep_for(std::chrono::seconds(window_size + 3));

    my_window.update_window(1);
    REQUIRE(my_window.net_events == 1);

    std::this_thread::sleep_for(std::chrono::seconds(window_size + 3));


    // update every shift
    for(int i = 0; i < window_size; i++){
      my_window.update_window(1);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    REQUIRE(my_window.net_events == window_size);
    std::stringstream ss;
    ss << "[Events] = " << my_window.net_events << std::endl;
    REQUIRE_THAT(my_window.display_window(), Catch::Matchers::EndsWith(ss.str()));
    
  }

  SECTION("Resizing"){

    bool res;
    int window_size = 4;
    sliding_window my_window(window_size);
    REQUIRE(my_window.net_events == 0);
    int num_events = 20;
    
    for(int i = 0; i < num_events; i++){
      my_window.update_window(1);
    }
    
    REQUIRE(my_window.net_events == num_events);

    int new_window_size = 2;
    res = my_window.resize_window(2);
    REQUIRE(res == true);
    REQUIRE(my_window.net_events == 0);

    my_window.update_window(100);
    REQUIRE(my_window.net_events == 100);

    std::this_thread::sleep_for(std::chrono::seconds(new_window_size + 1));
    my_window.update_window(1);
    REQUIRE(my_window.net_events == 1);

    res = my_window.resize_window(-1);
    REQUIRE(res == false);

    res = my_window.resize_window(MAX_WINDOW_SIZE + 1);
    REQUIRE(res == false);


    my_window.clear();
    REQUIRE(my_window.net_events == 0);
  }

}
