.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. Copyright (C) 2019 AT&T




Admission Control xAPP  Overview
================================

The Admission Control (AC) xAPP repository contains open-source code for a prototype xAPP that 
can execute the full control loop  to regulate the number of 5G connection requests at a gNodeB.

The AC  xAPP subscribes and listens for SgNB Addition Request X2AP messages. Upon
receiving one, it makes a decision on whether to accept or reject the request based on a simple
sliding window. If the decision is to accept the request, it sends an SgNB Addition Acknowledge, else
it sends an SgNB Addition Reject message.

The AC xAPP repository contains code to handle the E2 subscription process, encode/decode E2AP Indication/Control messages, X2AP SgNB Addition Request/Response
and supports the A1-interface which can be used to send policies to configure the admission control behaviour.




