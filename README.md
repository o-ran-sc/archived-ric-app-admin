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

This repository contains source code for a prototype open-source admission control xAPP. 
The admission control xAPP subscribes and listens for SgNB Addition Request X2AP messages. Upon
receiving one, it makes a decision on whether to accept or reject the request based on a simple
sliding window. If the decision is to accept the request, it sends an SgNB Addition Acknowledge, else
it sends an SgNB Addition Reject message.

1. Building docker image of the Admission Control xAPP
------------------------------------------------------
  Simply run : docker build -t <image-name> ./
  IMPORTANT : docker user must have credentials to access to the LF O-RAN docker repository : nexus3.o-ran-sc.org:10004,  which can be done with docker login prior to running the build


2. Compiling and invoking the AC xAPP directly
-----------------------------------------------
The Admission Control xAPP may be compiled and invoked directly. Required components for compilation are documeted in the Dockerfile (and also in README under src/).  A sample script src/run_xapp.sh is provided to show how to invoke the admission control xAPP with various options.

3. Invoking  xAPP docker container directly (ie not in RIC Kubernetes env.)
------------------------------------------------------------------ 
The xAPP docker run time can be configured with a json configuration file (example provided under init/config-file.json). Once such a file is availabel (say under directory /home/user/test-config), the docker image can be invoked as
docker run --net host -it --rm -v "/home/user/test-config:/opt/ric/config" --name "AC-xAPP" <image>
See REAMDE.md under the init directory for more details.


4. Invoking docker xAPP container in RIC Kubernetes environment
--------------------------------------------------------------
xAPPs are deployed via the xapp manager in the RIC. In  order to be deployable, a helm chart of the xAPP is created and uploaded to the helm 
repository. Typically the helm chart must be created by the RIC team rather than the xAPP owner since the helm chart will contain several environment
specific parameters. However, the xAPP owner may provide any xAPP specific configuration via a JSON configuration file and associated schema.
A sample configuration json file (which MUST be named config-file.json) and schema are provided under init/. Parameters of the JSON 
are documented in README under the init/ directory.

5. Testing Admission Control xAPP
---------------------------------
The Admission Control xAPP can be tested using mockups available under the test directory.
1.  mock-e2term-server  can be used to send X2 SgNB Addition Requests at a specified rate and monitor responses from the xAPP
2.  mock_ves_collectory.py is a simply cherrypy web-server which can be used to receive metrics from the AC xAPP 
3.  mock_a1_mediator can be used to test sending policies to the AC xAPP.

The README.md under the test directory contains instructions on how to build and use these mock up tools.


6. Layout 
---------
   a. src/ contains code for the admission control xAPP. See README.md under src for more information, as well as how to run the compiled executable directly (without docker container).

   b. test/ contains unit tests as well as mock up tools for integration testing

   c. schemas/ contains the various JSON schemas, sample responses/inputs  used by the Admission Control (AC) xAPP for setting/getting policies as wellreporting metrics.

   d. init/  contains a sample JSON configuration file and python script for starting xAPP in a container (only the JSON config should be modified as per environment)

   e. asn1c_defs : Contains all the getter/setting  asn1c generated code for manipulating/decoding/encoding ASN1 E2AP/E2SM/X2AP packages 
