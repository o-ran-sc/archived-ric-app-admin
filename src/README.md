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

Admission Control xAPP Functionality Overview
----------------------------------------------

The admission control xAPP can be used as a prototype to throttle connections during overload events or provide class of service protection.
The xAPP registers to receive SgNB Addition Request messages from the gNodeB. When it receives such a message, it applies a simple sliding window
logic to decide whether to accept or block the request and responds with a acknowledge or reject respectively. 

When the admission control xAPP starts up, it creates the following main objects (see adm-ctrl-xapp.cc)  :
1. Instantiate an xAPP RMR object that can perform RMR related functions.
2. Instantiate a subscription handler object which handles requests from the xAPP and repsonses from the RAN.
3. Instantiate a network protector plugin which applies sliding window logic on X2AP messages from the RAN
4. Instantiate a message processor object that  receives RMR messages
	- register the subscription handler and network protector plugin with the message processor object so that appropriate RMR messages can be sent to the correct function.

Next, the xAPP :
- spins up a thread that periodically polls the protector plugin for metrics and POSTs them on the VES collector (an ONAP measurement mechanism)
	* currently two metrics are reported (SgNB Request Rate and SgNB Accept Rate). The measurement period can be configured at run time (see README.md under init). The protector plugin records number of requests and accepts over the measurement period which are then reported as a JSON by the metrics thread.
        * see the README.md under schemas for more information on the format they are reported in.

- spins up (at least one ) another thread from the xAPP RMR Object and register the message processor object with that so that it can listen and process RMR events.
    * The message processor object is responsible for inspecting the RMR payload, call the appropriate handler (e.g. subscription, network protector, policy) handler based on RMR message type 

    * The message processor object periodically posts some simple processing latency metrics to stdout in the form:
epoch timestamp, number of packets in reporting (fixed in code), average processing latency over this batch (microseconds), standard deviation of processing latency over this batch (micro seconds), maximum processing latency over this batch (micro-seconds).
The latency is measured from the time the message processing object receives meessage from RMR to the time it finishes processing the packet. 

- from the main thread sends out subscription requests (with an event trigger defintion for procedureCode 27 (SgNB Addition Request) ) for specified gNodeBs. Once the subscriptions succeed or fail, main thread goes quiescent till terminated. 



The src directory contains source code for the AC-xAPP core functionality.

PREREQUISITES FOR COMPILING AC-xAPP executable directly
-----------------------------------------------------
In order to compile the code successful, the following packages must be installed (and in search paths): 
1. mdclog  : a logging library
2. rmr     : RIC message router  (if installing on the deb package, next generation nano msg must be separately installed)
3. rapidjson : a header only file for manipulation of json.
4. libcurl : the libcurl header and library for posting to a web-server
5. gcc (version > 4.x) .. has not been tested with other compilers as of now
See the Dockerfile for installation links and details

Once pre-requisites are installed, 'make adm-ctrl-xapp'  will compile the xAPP into an executable (also called adm-ctrl-xapp)

RUNNING ADMISSION CONTROL xAPP
------------------------------
NOTE: Before runnig the admission control xapp,  RMR routes must be made available to allow correct routing of subscription, policy, indication 
and control message. In a test environment, this can be done by exporting an environment variable called RMR_SEED_RT that points to the full
path of a RMR route file. 
An example route file called uta_rtg.rt is provided under the test/ directory

A sample script ./run_xapp.sh  is provided to illustrate and   execute the xAPP with various parameters. 
 -a : provide full path to  JSONschema file used by xAPP for setting/getting policies  
 -v : provide full path to JSON schema file describing the metrics posted by xAPP to VES
 -s : provide full path to JSON payload samples used by xAPP for responding to get/set policies and get metrics
 -u : URL path for VES collector
 -g : comma separated list of gNodeBs to send subscription request to
 -i : time interval  (in seconds) at which to post metrics to the VES collector
 -c : operation model (CONTROL, REPORT, E2AP_PROC_ONLY)
       * in the E2AP_PROC_ONLY mode, the Admission Control xAPP will simply process only the E2AP Indication part of the RMR message. It does not process the X2 portion of the message or invoke the sliding window algorithm or send a response back. This is useful to just test basic functionality without a valid X2 message.

       * in REPORT mode, the xAPP will process the E2AP indication message, X2AP message, invoke the sliding window and record the decision (accept/reject), but not send a control back. Useful to passively understand decision making of the AC xAPP (since metrics will reflect the decision of xAPP) without influencing RAN.

       * CONTROL mode. This is the full control loop where the xAPP will send back an acknowledge or reject based on decision made.

  -- verbose flag : if set, log level is set to DEBUG mode.


A high level explanation of Various parts of the code is provided next  w ....

xAPP RMR Framework
-------------------

The base XaPP class is defined in xapp_utils.hpp.  An XaPP class object  can be instantiated in a number of ways :

1. XaPP(char *app_name, char *protocol, int message_size) : where 
	- app_name points to a char array to identify this xapp (e.g "my_app_1") 
	- protocol points to a char array to specify the protocol/port on  which this xAPP listens for messages (e.g "tcp:4561")
	- message_size is the maximum size of messages allowed to send ..

	In this invocation, the number of listener threads (invoked when XaPP.Start() is called) is determined automatically based on hardware


2. XaPP(char *app_name, char *protocol, int message_size, int num_threads) : (RECOMMENDED)    where 
	- app_name points to a char array to identify this xapp (e.g "my_app_1") 
	- protocol points to a char array to specify the protocol/port on  which this xAPP listens for messages (e.g "tcp:4561")
	- message_size is the maximum size of messages allowed to send ..
	- num_threads is the number of threads to invoke on which the XaPP listens for and processes messages


3. The XaPP object can start listening to messages using the Start function, with two variants :

	- XaPP.Start(message_handler) : in this invocation, any message received by the XaPP is passed on the message_handler function which must have the signature bool *(rmr_mbuf_t *).  
        If the message handler wishes to respond, it must modify the message buffer in place and respond with a true value, else respond with false.

	- XaPP.Start(message_handler, error_handler) : in this invocation, the user can specify an error_handler function, which must have the signature void *(rmr_mubf_t *) if the sending of a message (processed by message_handler) fails. 


4. The XaPP object can be used to send messages using the Send function with two variants :
	- XaPP.Send(int message_type, int message_length, void *message_payload)
	- XaPP.Send(rmr_mbuf_t * message);
	They return true if message was sent successfuly and false if not


If the XaPP is started with multiple listening threads, they all share the same RMR context and invoke the same message handler/error handler user functions. Hence these are expected to be thread safe. Since all threads will listen to RMR, there is no gaurantee on which thread receives which message.


ASN1 PDU Processing
-------------------

The RAN uses ASN1 encoded PDUs to communicate with the RIC. An open source asn1c compiler is used to generate getters/setters for these ASN1 PDUs.
The getters/setters are under asn1c_defs/all-defs/.
Broadly, there are three ASN1 specification that the current xAPP is required to handle :

1. E2AP : This is the specification of how packets are sent between RAN and RIC. It includes subscription related work flows as well as the indication/control work flow.

2. E2SM : This is the service model specification used to specify what kind of subscriptions etc are being sent

3. X2AP : This is the protocol used to communiate between eNodeBs and gNOdeBs.
          X2AP PDUs are wrapped in E2AP indication messages and sent to RIC for reporting or control decisions.

The Admission Control xAPP contains a set of classes to handle generation and decoding of these PDUs which can be re-used. The classes provide methods for setting fields in the PDU, encoding a PDU and extraction of fields from a decoded PDU. 

1. src/E2AP-c  : This directory contains generic code for setting/encoding/decoding/getting parameters for E2AP Indication and E2AP control packets.
2. src/E2AP-c/subscription : This directory contains generic code for setting/encoding/decoding/getting parameters for all Subscription related messages 
	- Subscription Request, Response, Failure
	- Subscription Delete Request, Response, Failure
   It also contains a subscription_handler which is thread-safe, and manages the subscription request/response process. 
   NOTE : Currently subscriptions are uniquely identified by Request ID (can be easily changed in the future).

3. src/X2AP : Contains generic code for setting/encoding/decoding/getting SgNB Addition Request, Response (Acknowledge and Failure)

4. src/E2SM : Contains generic code for setting/encoding/decoding/getting  parameters for the E2 Service Model (E2SM)

Other Components
----------------
1. curl : simple class for using libcurl to post metrics

2. json : class for validating against schema and get/set JSON payload  values (uses rapidjson)

3. protector-plugin : Contains the code code for accept/reject logic based on sliding window, as well as a wrapper around the sliding window for setting/getting policies and retreiving metrics.

4. adm-ctrl-xapp.cc, get_config.cc, message_processing.cc
	- adm-ctrl-xapp.cc is the main code that applies all start up parameters etc, triggers subscription, starts up xAPP listener threads for receiving messages over RMR and responding to them and a thread to post metrics to VES collector

	- get_config.cc  : processing command line and environment variables at start up.

	- message_processor_class.cc  : The RMR message processing engine. It listens for messages on RMR and invokes appropriate handler (e.g subscriptions, E2AP, Policy). 
