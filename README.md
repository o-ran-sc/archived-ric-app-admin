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

This repo contains code related to the admission control xapp.
Main source code for the xapp is under src/

Look at Dockerfile for building instructions 


NOTE : In order to compile code as a stand alone process, following pre-requisites are required 
-- For compilation, the nanomsg, nng and ric messaging libraries, as well as ric messaging header must be installed in standard paths (or if non-default, then visible in search paths)

-- mdclog library and headers must be installed and available in search path

NOTE: 
In order to test the code, 

-- An E2 termination must be available

-- A ves collector which collects metrics must be available

The run_xapp.sh script provides a convenient way to set the various parameters and invoke the process (if not running in a docker container)
