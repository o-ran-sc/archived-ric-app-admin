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


This document gives an overview of the base class of a simple xAPP class. It is used to build the adm-ctrl-xapp executable

Pre-requisitis : nanomsg libraries, nng libraries and ric messaging library + header must be installed and visible on standard library/include paths (via LD_LIBRARY_PATH, LIBRARY_PATH, CPATH)

Usage Guidelines :
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

1. The basic X2AP builder class (which is responsible for encoding/decoding and setting the X2AP-ELEMENTARY-PROCEDURE) is in x2ap_pdu_builder.hpp
2. Any other IE  requires own code for processing. Currently ResourceStatusRequest builder is under ResourceStatuRequest_builder.hpp and ResourceStatusResponse builder under ResourceStatusResponse_builder.hpp



--- The adm-ctrl-xapp is a basic example to demonstrate RMR. The adm-ctrl-xapp just uses the basic xAPP class but with message processing done by message_processing.cc. 

It receives ResourceStatusRequest message over RMR (message type defined in RMR_services.h), decodes it, extracts elements and responds with a ResourceStatusResponse message (message type defined in RMR_services.h). As an example for verification, it subtracts 1 from the received enb1 and enb2 ids before inserting in response ... 
