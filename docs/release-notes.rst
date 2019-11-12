.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. Copyright (C) 2019 AT&T


Release Notes
=============


This document provides the release notes for the Amber Release of the Admission Control xAPP.

.. contents::
   :depth: 3
   :local:


Version history
---------------

+--------------------+--------------------+--------------------+--------------------+
| **Date**           | **Ver.**           | **Author**         | **Comment**        |
|                    |                    |                    |                    |
+--------------------+--------------------+--------------------+--------------------+
| 2019-11-04         | 1.0.0              |   Ashwin Sridharan | First draft        |
|                    |                    |                    |                    |
+--------------------+--------------------+--------------------+--------------------+



Summary
-------

The Amber release of the AC xAPP supports  full closed loop control as well as report mode operation
for admission control of SgNB Addition requests, reporting of metrics over VES,
and configuration of single instance policies via the A1-Interface.



Release Data
------------

+--------------------------------------+--------------------------------------+
| **Project**                          | RAN Intelligent Controller           |
|                                      |                                      |
+--------------------------------------+--------------------------------------+
| **Repo/commit-ID**                   |        ric-app/admin                 |
|                                      |                                      |
+--------------------------------------+--------------------------------------+
| **Release designation**              |              Amber                   |
|                                      |                                      |
+--------------------------------------+--------------------------------------+
| **Release date**                     |      2019-11-14                      |
|                                      |                                      |
+--------------------------------------+--------------------------------------+
| **Purpose of the delivery**          | open-source xAPP for admission       |
|                                      | control.                             |
|                                      |                                      |
+--------------------------------------+--------------------------------------+

Components
----------

- *src/* contains the main source code. Under that directory :
  
  + *xapp_utils.hpp, xapp_utls.cc* is generic multi-threaded framework for receiving and sending RMR events.
  + *E2AP-c/subscription/* contains generic classes to send/process ASN1 subscription requests, responses, deletes and failures as well as thread-safe subscription handler for managing the subscription process.
  + *E2AP-c/* contains generic classes for generating/processing ASN1  E2AP Indication and Control messages.
  + *E2SM/* contains generic classes for handling generating/processing ASN1 E2SM service model (e.g event trigger etc).
  + *curl/* contains a simple *libcurl* based class for POSTing JSON messages.
  + *json/* contains a generic class for simple JSON key retreival and modification (based on rapidjson)
  + *protector-plugin/* contains code specific to the admission control algorithm and interfaces for setting/getting policy.

- *test/* contains unit tests showing how to use various components as well as mock-ups for integration testing.

- *schemas/* contains the JSON schemas for A1 policy, VES metrics as well as sample payloads.
  
    

Limitations
-----------
- While the xAPP framework used in the AC xAPP supports multi-threading, the admission plugin currently only supports a single thread.
- The admission plugin supports only a single class of service in current release.
- The subscription request parameters (RAN Function ID etc) cannot be changed.
