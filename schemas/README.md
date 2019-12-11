This directory contains the various JSON payloads received/sent by the xAPP and the associated schemas.

1. Policy messages are set (and requested) via RMR. The A1 mediator which sends these requests verifies that the messages conform to schema specified by the xAPP developer (the AC xAPP also validates this).  The schema for the policies is provided by the xAPP developer 
	- The AC xAPP policy schema file is rate-control-policy.json

2. Similarly, metrics posted to the VES collector must conform to the global schema specified by the VES collector.
	- the ves schema file is ves_schema.json

3. The samples.json file contains templates (example payloads)  of various JSON payloads that the AC xAPP receives (e.g message_receives_example) or sends (message_sends_example, metrics).  The samples.json file is just a convenient place to host all the example payloads. It is loaded at run time by the admission_policy.cc wrapper around the protector plugin. The wrapper extracts the value (which is JSON paAyload) corresponding to the relevant keys 
nd uses them as base templates for appropriate actions. 

	* When the AC xAPP receives a policy, it uses the message_receives_example example payloadexample payload to extract the "enforce", "window_size" etc. keys from the payload and update protector plugin values.
	* When the AC xAPP wants to send a response to policy set, it updates the "status" and "message" keys in the message_sends_example to inform SUCCESS or FAILURE.
	* When the AC xAPP wants to send metrics to the VES collector, it updates the following keys  in the payload at the "metrics" key
		- all key values under "additionalFields" based on number of SgNB Addition requests accepted/rejected
		- "measurementInterval"  under "measurementFields" based on the configured measurement interval (see README.md under init)
