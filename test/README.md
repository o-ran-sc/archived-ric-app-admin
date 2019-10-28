#==================================================================================
#        Copyright (c) 2018-2019 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#==================================================================================

This directory contains various unit tests as well as mock-ups for integration testing.

Several unit test cases are provided to test and illustrate usage of various components of the AC xAPP. The unit tests use the CATCH framework(https://github.com/catchorg/Catch2).  It is a header only framework which can be installed simply by retreiving the header and placing it in the search path. To install : 
 wget -nv  --directory-prefix=/usr/local/include/Catch2 https://github.com/catchorg/Catch2/releases/download/v2.9.1/catch.hpp

In addition all the other requirements to compile the AC xAPP (as listed under src/README.md) must be already in place

The unit tests can be run by first compiling them (make all_tests)  and then executing them (./run_tests.sh)
The default make does not do any optimizations and instead aims to generate coverage files (this can be changed by specifying MODE=perf when compiling the tests)

If gcovr is installed (https://github.com/gcovr/gcovr) the script  will  also generates a coverage report (as ../coverage_report.html)

Apart from unit test cases, the test directory also includes three mock-up components to test the AC xAPP.
1. A mock E2 Term : This executable can be created as 'make  mock-e2term-server'.  The mock e2 term server, listens for subscription requests/deletes,
responds to them and when a subscription is active, sends X2 SgNB Addition Requests (encapsulated in E2AP Indication) at a specified rate. This
mimics the E2Term + RAN behaviour.  The executable can be invoked as :
./mock-e2term-server -p <e2term port number> -r <rate to send at>

The e2term port number must match the configured port number in the RMR route table. For example, in the provided uta_rtg.rt example file, E2term
port is assumed to be 38000 

2. A mock VES collector : This is basically a cherrypy server configured to expose a JSON end point, collect the metrics and extract some relevant
stats from the provided scripts. Note  that this is NOT the actual VES collector. The web server simply suffices to mock up the interaction between
the xAPP and the VES collector.  By default, the ves collector is configured to start up and listen on 127.0.0.1:6350, but it can be changed.
Whatever URL is configured on the mock ves collector must match the URL supplied to the AC xAPP (see src/README.md -u option under RUNNING ADMISSION CONTROL xAPP)
Pre-requisities for running the mock ves collector are :
1. python 2.7
2. cherrypy module (pip install cherrypy)

The mock ves collector can be simply invoked as python ./mock_ves_collector.py


3. A mock A1 mediator : The mock up a1 mediator executable sends various JSON (valid/invalid) payloads in sequence to illustrate the behaviour and response of the AC xAPP. It can be compiled  as 'make mock-a1-server'
To execute the mock-a1-server,  it requires as argument a port to listen and send on (can be any port that does not conflict with others). The A1 mediator uses rts send/receive and hence bypasses the routing table. 
./mock-a1-server -p <port number> 
