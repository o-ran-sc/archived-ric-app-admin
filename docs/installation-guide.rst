.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. Copyright (C) 2019 AT&T


Installation Guide
==================

.. contents::
   :depth: 3
   :local:

Abstract
--------

This document describes how to install the Admission Control (AC) xAPP. 

Version history

+--------------------+--------------------+--------------------+--------------------+
| **Date**           | **Ver.**           | **Author**         | **Comment**        |
|                    |                    |                    |                    |
+--------------------+--------------------+--------------------+--------------------+
| 2019-11-14         |1.0.8               |Ashwin Sridharan    | Amber Release      |
|                    |                    |                    |                    |
+--------------------+--------------------+--------------------+--------------------+


Introduction
------------

This document provides guidelines on how to install and configure the AC xAPP in various environments/operating modes.
The audience of this document is assumed to have good knowledge in RAN network nd Linux system.


Preface
-------
The AC xAPP can be run directly as a Linux binary, as a docker image, or in a pod in a Kubernetes environment.  The first
two can be used for testing/evaluation. The last option is how an xAPP is deployed in the RAN Intelligent Controller environment.
This document covers all three methods.  




Software Installation and Deployment
------------------------------------
The build process assumes a Linux environment with a gcc (>= 4.0)  compatible compiler and  has been tested on Ubuntu and Fedora. For building docker images,
the Docker environment must be present in the system.


Build Process
~~~~~~~~~~~~~
The AC xAPP can be either tested as a Linux binary or as a docker image.
   1. **Docker Image**:    From the root of the repository, run   *docker --no-cache build -t <image-name> ./* .
      **IMPORTANT** : docker user must have credentials to access to the LF O-RAN docker repository : *nexus3.o-ran-sc.org:10004*,  which can be done with docker login **prior** to running the build

   2. **Linux binary**: 
      The AC xAPP may be compiled and invoked directly. Pre-requisite software packages that must be installed prior to compiling are documented in the Dockerfile (and also in README under src/) in the repository. From within the *src* directory,  run *make adm-ctrl-xapp*.   


Deployment
~~~~~~~~~~
For simple unit tests, integration tests etc., the Linux binary or docker image may be used directly. When tested E2E in a RIC, the AC xAPP must be deployed as a K8 pod using the xAPP manager (or helm for simple tests). In all
scenarios, an important pre-requisite for deployment is the availability of routes (see RMR documentation) to the xAPP indicating where to send E2AP subscription requests and control messages. In production, this set of routes
will be provided by the *Route Manager*. For local testing, static routes can be provided to the RMR library by setting the environment variable *RMR_SEED_RT* to point to the complete path of a  file with routes.  An example RMR route file is provided under *test/uta_rtg.rt*.

1. **Invoking the AC xAPP directly as a Linux binary** :
   
   - Set the environment variable *RMR_SEED_RT* to point to the complete path of a file with static routes.
     
   - Use the provided  sample script src/run_xapp.sh to invoke the AC xAPP. The script lists the  various options which may be changed depending on environment.
     

2. **Invoking  xAPP docker container directly** (not in RIC Kubernetes env.):

   - The xAPP docker run time **must** be configured with a json configuration file appropriate to the test environment which injects various environment variables including the RMR routes   (an example is provided under init/config-file.json).

   - Once such a  file is available (say under directory /home/user/test-config),  the docker image can be invoked as *docker run --net host -it --rm -v "/home/user/test-config:/opt/ric/config" --name  "AC-xAPP" <image>*.  See README.md under the init directory for more details.


3. **Invoking docker xAPP container in RIC Kubernetes environment** :
   In an actual deployment, xAPPs are deployed as K8 pods via the
   xapp-manager in the RIC with their configuration mounted as
   *configmaps*. In order to be deployable by the xapp-manager, a helm
   chart of the xAPP must be created and uploaded to the helm
   repository. Generation of helm chart falls under the purview of the
   RIC integration team rather than the xAPP owner since the helm
   chart will contain several environment specific
   parameters. However, the xAPP owner may provide any xAPP specific
   configuration via a JSON configuration file and associated schema.
   A sample configuration json file (which **MUST** be named
   config-file.json) and schema are provided under init/. Parameters
   of the JSON are documented in README under the init/ directory.

   As an alternative to the xapp-manager, in a test K8 environment,
   the AC xAPP may also be directly installed via Helm (thought it may
   not be controllable via the RIC dashboard).

Testing 
--------

Unit tests for various modules of the AC xAPP are under the *test/* repository. Currently, the unit tests must be compiled and executed  in a Linux environment. All software packages  required for compiling the AC xAPP must be installed (as listed in the Dockerfile). In addition   a pre-requisite for the unit tests is installation of the *Catch2 C++* header file  since the unit tests use this framework.  This can be easily done by running

*wget -nv  --directory-prefix=/usr/local/include/Catch2 https://github.com/catchorg/Catch2/releases/download/v2.9.1/catch.hpp*.

After that, the unit tests can be compiled and run by executing the following commands from the *test/* directory :

- *make all_tests*
- ./run_tests.sh
- If gcovr is installed (https://github.com/gcovr/gcovr) the script  will  also generates a coverage report (as ../coverage_report.html)

In order to run integration tests, the AC-xAPP requires *three* components : an *E2 Termination point* to send and receive RAN messages, an *A1 mediator* to send policy updates and a *VES collector* to receive metrics. The *test/*
directory contains mock-ups for these three components which can be build and executed from the *test/* directory as follows  :

1.  **E2 Termination** :  The E2 termination is responsible for forwarding  messages to and fro between the RAN and RIC. A mock-up of the E2 termination is provided in *test/* that

    - listens and responds to E2AP subscription requests.
    - upon receiving an E2AP subscription request, starts sending E2AP Indication messages that contain the X2AP SgNB Addition Request Message.
    - monitors E2AP control messages from the AC xAPP.
      
    The E2 term executable can be build and executed as follows :

    - *make mock-e2term-server* compiles the executable
    - To invoke it first ensure the *RMR_SEED_RT* environment variable is set to point to complete path of a route file. Then run * ./mock-e2term-server -p <E2 term port number> -r <rate to send E2AP indication messages>
    - *NOTE* : The E2 term port number must be set to the port number listed in the route table that can receive E2AP subscription requests, E2AP indications. Default port that is used is 38000.

2.  **A1 Mediator** : The A1 mediator is responsible for sending policies to the xAPPs over RMR to configure their behaviour. A mock-up of the A1 mediator can be built and executed as follows :
    
    - *make mock-a1-tool* builds the executable.
    - The executable can be run as *./mock-a1-tool -p <port number> <options> *  where port number can be any port not conflicitng  with the xAPP and E2 Term.
    - Note that the A1 mediator also uses RMR and hence the environment variable *RMR_SEED_RT* must also be set when executing *mock-a1-tool* (if static routes are being used).
    - Run ./mock-a1-tool -h to see various options.
        
3.  **VES Collector** : This component is responsible for receiving metrics from xAPPs as JSON payloads. A simple mock-up is available under *test/* which is basically a *cherrypy* web-server that receives VES messages from the Admission Control Xapp  and prints out relevant messages. It can be invoked as *python ./mock_ves_collector.py*.

    - Pre-requisites for the VES collector are the *cherrypy* and *requests* Python modules. They can be installed via pip :  *pip install cherrypy requests*.
      


