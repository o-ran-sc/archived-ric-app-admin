.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. Copyright (C) 2019 AT&T


User Guide
==========

This is the user guide of AC xAPP, describing its various features and how to configure them .

.. contents::
   :depth: 3
   :local:

..  a user guide should be how to use the component or system; it should not be a requirements document
..  delete this content after editing it


Description
-----------
.. Describe the target users of the project, for example, modeler/data scientist, ORAN-OSC platform admin, marketplace user, design studio end user, etc
.. Describe how the target users can get use of a O-RAN SC component.
.. If the guide contains sections on third-party tools, is it clearly stated why the O-RAN-OSC platform is using those tools? Are there instructions on how to install and configure each tool/toolset?

The AC xAPP provides rate control of SgNB Addition Requests via a standard sliding window control algorithm which is configurable at run-time. Please see :ref:`Installation Guide` for instructions on compiling and executing the AC xAPP. This document explains the various configurable parameters of the AC xAPP executable, policies and metrics.

Upon start up, the AC xAPP will spin up a thread on which to listen for RMR events, another thread to post metrics and then start sending subscription requests for the SgNB Addition Request on the main thread. When an E2AP indication
message with the X2AP SgNB Addition Request message is received, the AC xAPP will run the sliding window algorithm, decide a response (see below for specifics) and return the response. The next section shows how to configure
various facets of the AC xAPP that cover subscription, policy for configuration, metric reporting and behaviour of the core algorithm itself.


Run Time options
----------------
The AC xAPP takes the following parameters (either on the command line) or as environment variables on invocation (see *src/run_xapp.sh* for an example of providing arguments on command line and *init/config-file.json* for environment variables)  :

1. List of comma separated gNodeBs to send subscription requests to. Can be specified with :
   
   - Use *-g or --gNodeB* on command line.
   - Set the "GNODEB" environment variable

2. The A1 policy schema file which specifies the schema and parameters of acceptable policy. The AC xAPP sliding window algorithm can be configured with the following 5 parameters via the policy (see *schemas/sample.json* for an example).  The AC xAPP policy schema file is outlined in *schemas/adm-ctrl-xapp-policy-schema.json*.   The policy parameters are :
   
   - "enforce": A boolean flag on whether to enforce policy or not.
   - "window_length" : Length of the sliding window in minutes.
   - "trigger_threshold: Threshold of events in sliding window above which to start blocking.
   - "blocking_rate": A percentage between [0-100] of connections to block if above *trigger_threshold*.

     For example, a window length of 1, trigger threshold of 5000 and blocking rate of 50 would mean that if in a window of 1 minute (60 seconds), there are more than 5000 events, then start blocking half of the incoming connections.
     Note that the window is always updated *prior* to blocking. In the above example, if arrival rate exceeds 84 events/sec ) over a period exceeding 1 minute ((84 * 60 = 5040 > 5000), then blocking will be triggered.
     
     The A1 policy file location can be specified to the  AC xAPP with 
     -  *-a* on command line.
     -  "A1_SCHEMA_FILE" environment variable.


3. The VES schema to which all reported metrics must comply. The VES schema file can be specified with

   - *-v* on command line.
   - "VES_SCHEMA_FILE" environment variable.

4. Set of sample JSON payloads for policy and metrics that the AC xAPP uses as templates to generate payloads. Values in the template payload are modified/retrieved rather than construct the entire payload from scratch. The JSON file
   containing the payloads can be specified with :
   - *-s* on command line.
   - "SAMPLE_FILE" environment variable.

    
5. URL of the VES collector to which to send metrics. Can be specified with :

   - *-u* on command line.
   - "VES_COLLECTOR_URL" environment variable.

6. Reporting interval for metrics in seconds (i.e., how often to post metrics to VES). Can be specified with :

   - *-i* on command line.
   - "VES_MEASUREMENT_INTERVAL" environment variable.

7. Log level. Can be specified with :

   - *--verbose** flag on command line which will set the log level to most verbose (DEBUG).
   - "LOG_LEVEL" environment variable. This allows finer grained control. Options are MDCLOG_ERR, MDCLOG_INFO, MDCLOG_DEBUG.

8. The operating mode of the AC xAPP.  The AC xAPP can be run in three modes which can be specified with :

   - *-c* option on command line.
   - "OPERATING_MODE" environment variable.

   The three operating modes supported are :

   - "E2AP_PROC_ONLY" . In this mode, the AC xAPP simply decodes and records the E2AP message.
   - "REPORT" . In this mode, the AC xAPP decodes the E2AP and the underlying X2AP message. If the message is an SgNB Addition Request Message, it executes the admission control algorithm and records the result. It does **not** however send a response back to RAN. This is useful when testing passively without interfering with RAN operation.
   - "CONTROL". In this mode, the AC xAPP executes the full control loop where it also sends back an SgNB Addition Response or Reject message.
