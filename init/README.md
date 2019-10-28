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

Configuration parameters for the AC xAPP process  may be provided via command line or environment variables. When testing via command line, 
they can be set using the src/run_xapp.sh script for convenience  (routes however must be set via the RMR_SEED_RT file) as documented under src/README.md

However, when deployed as a docker container, or as a Kubernetes pod by the xAPP manager, configuration information is provided as a JSON file and the executable is invoked via a python script.

CONFIGURATION FOR DOCKER/HELM
-------------------------------
The file config-file.json in this directory contains an example configuration that is supplied to the xAPP executable (adm-ctrl-xapp). The file name
is chosen (config-file.json) to be compatible with the xAPP Manager (which when deploying the xAPP looks for this file to mound as a config map).

The JSON schema for the file is in "schema.json", again following file name convention as required by xAPP manager. In order to be deployed via the xAPP manager, a helm chart conforming to the environment must be created. Creation of the helm chart is outside the scope of the xAPP : it is the responsibility of the integration team since they control the environment (the helm chart is created from a standard template using scripts under the it/dep repo) . The xAPP developer must however provide the config-file.json and schema.json with the former containing place holders for all relevant variables. 

The config-file.json JSON file contains the following information:

1. "service_ports" : Used by xAPP manager to expose service ports 
2. "rmr" : This section contains the various RMR message types ("txMessages", "rxMessages")  that the xAPP expects to either send/receive. Used by the xAPP manager to inform router manager.  

It also contains a static routing table under "contents" which is written  by the python start up script (described below)  to a file listed under "file_path" and exported as the RMR_SEED_RT environment variable in case Route manager is not available .

3. "env" : This lists all the configuration parameters to control the behaviour of the AC xAPP as described in src/README.md


NOTE : The config-file.json parameters are environment specific (e.g depending on location of VES collector, host names in the RMR route table etc) and hence must be modified depending on the supported environment. The provided file in this repository is just a simple example with place holders.



Apart from the config-file.json configuration, a python start up script is also provided (init_script.py). The script simply loads all the necessary
values as provided by config-file.json in the environment, and then invokes the adm-ctrl-xapp executable.
