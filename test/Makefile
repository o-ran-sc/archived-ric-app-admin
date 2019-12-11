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

ifeq ($(MODE), perf)
	CXX:= g++ -g -O2  -fprofile-arcs -ftest-coverage
	CC:= gcc -g -O2 -fprofile-arcs -ftest-coverage
else
	CXX:= g++ -g -fprofile-arcs -ftest-coverage
	CC:= gcc -g -fprofile-arcs -ftest-coverage
endif

SRC_DIR=../src
PLUGINS=../src/plugin-interface
ASN1C_DIR=../asn1c_defs
E2AP_C_DIR=../src/E2AP-c
SUBSCR_DIR = ../src/E2AP-c/subscription
E2SM_DIR=../src/E2SM
X2AP_DIR=../src/X2AP
#JSON_DIR=../src/json
A1= ./a1
PROTECTOR=../src/protector-plugin

CLOGFLAGS= `pkg-config mdclog --cflags`
BASEFLAGS= -Wall  -std=c++14 $(CLOGFLAGS)  -DASN_DISABLE_OER_SUPPORT
C_BASEFLAGS= -Wall $(CLOGFLAGS) -DASN_DISABLE_OER_SUPPORT
XAPPFLAGS= -I$(SRC_DIR)
PLUGINFLAGS= -I$(PLUGINS)
E2FLAGS=  -I$(E2AP_C_DIR)  -I$(ASN1C_DIR)/ 
SUBSCRFLAGS= -I$(SUBSCR_DIR)  -I$(ASN1C_DIR)
E2SMFLAGS = -I$(E2SM_DIR) -I$(ASN1C_DIR)/
X2FLAGS= -I$(X2AP_DIR) -I$(ASN1C_DIR)/
A1FLAGS = -I$(A1)
#JSONFLAGS= -I$(JSON_DIR)
PROTECTORFLAGS= -I$(PROTECTOR)

LIBS= -lrmr_nng  -lnng -lpthread -lm 

# Logging library 
LOGLIBS= `pkg-config mdclog --libs`

XAPP_UTILS_SRC= $(SRC_DIR)/xapp_utils.cc 
XAPP_MPROC_SRC= $(SRC_DIR)/message_processor_class.cc
E2AP_SRC= $(wildcard $(E2AP)/*.cc)
X2AP_SRC= $(wildcard $(X2AP_DIR)/*.cc)
A1_SRC= $(wildcard $(A1)/*.cc)
PLUGINS_SRC = $(wildcard $(PLUGINS)/*.cc)
E2AP_C_SRC= $(wildcard $(E2AP_C_DIR)/*.cc)
SUBSCR_SRC= $(wildcard $(SUBSCR_DIR)/*.cc)
E2SM_SRC= $(wildcard $(E2SM_DIR)/*.cc)
X2AP_SRC = $(wildcard $(X2AP_DIR)/*.cc)
ASN1C_SRC = $(wildcard $(ASN1C_DIR)/*.c)
#JSON_SRC = $(wildcard $(JSON_DIR)/*.cc)
PROTECTOR_SRC = $(wildcard $(PROTECTOR)/*.cc)

#===========================================
# TEST MODULES
XAPP_UNIT_TEST = unit_test_xapp.o
X2PDU_UNIT_TEST= unit_test_pdu.o
E2SUB_UNIT_TEST= unit_test_subscription.o
SUBSCRIPTION_FLOW_UNIT_TEST = unit_test_subscription_flow.o
E2AP_IND_UNIT_TEST= unit_test_e2ap_indication.o
E2AP_CTRL_UNIT_TEST= unit_test_e2ap_control.o
E2SM_UNIT_TEST=unit_test_e2sm.o
#JSON_UNIT_TEST = unit_test_json.o
SGNB_ADD_REQ_UNIT_TEST = unit_test_sgnb_addition_request.o
MESSAGE_PROC_UNIT_TEST = unit_test_message_processor.o
SLIDING_WINDOW_UNIT_TEST = unit_test_sliding_window.o
PROTECTOR_UNIT_TEST = unit_test_protector_plugin.o
ADMISSION_UNIT_TEST = unit_test_admission_policy.o
MOCK_E2TERM_OBJ= mock_e2term_server.o
MOCK_A1SERVER_OBJ = mock_a1_mediator.o
#===========================================	


# SOURCE MODULES
XAPP_UTILS_OBJ = $(XAPP_UTILS_SRC:.cc=.o) 
XAPP_MPROC_OBJ= $(XAPP_MPROC_SRC:.cc=.o)
E2AP_OBJ=$(E2AP_SRC:.cc=.o)
X2AP_OBJ=$(X2AP_SRC:.cc=.o)
A1_OBJ= $(A1_SRC:.cc=.o)
PLUGINS_OBJ = $(PLUGINS_SRC:.cc=.o)
SUBSCR_OBJ= $(SUBSCR_SRC:.cc=.o)
E2AP_C_OBJ = $(E2AP_C_SRC:.cc=.o)
E2SM_OBJ = $(E2SM_SRC:.cc=.o)
ASN1C_MODULES = $(ASN1C_SRC:.c=.o)
X2AP_OBJ = $(X2AP_SRC:.cc=.o)
#JSON_OBJ = $(JSON_SRC:.cc=.o)
PROTECTOR_OBJ = $(PROTECTOR_SRC:.cc=.o)

#===========================================

$(XAPP_UTILS_OBJ) $(XAPP_UNIT_TEST)  :export CPPFLAGS= $(BASEFLAGS) $(XAPPFLAGS)

$(A1_OBJ)  $(MOCK_A1SERVER_OBJ) $(JSON_UNIT_TEST) :export CPPFLAGS= $(BASEFLAGS) $(A1FLAGS) $(XAPPFLAGS)

$(XAPP_MPROC_OBJ) $(MESSAGE_PROC_UNIT_TEST) $(MOCK_SUB_CLIENT_OBJ):  export CPPFLAGS=$(BASEFLAGS)  $(XAPPFLAGS) $(E2FLAGS) $(X2FLAGS) $(SUBSCRFLAGS) $(PLUGINFLAGS) $(PROTECTORFLAGS) $(E2SMFLAGS)

$(SUBSCRIPTION_FLOW_UNIT_TEST): export CPPFLAGS=$(BASEFLAGS) $(SUBSCRFLAGS) $(E2SMFLAGS)

$(E2AP_C_OBJ) $(E2AP_IND_UNIT_TEST) $(E2AP_CTRL_UNIT_TEST) : export CPPFLAGS=$(BASEFLAGS) $(E2FLAGS)

$(SUBSCR_OBJ) $(E2SUB_UNIT_TEST): export CPPFLAGS=$(BASEFLAGS)  $(SUBSCRFLAGS)

$(E2SM_OBJ) $(E2SM_UNIT_TEST): export CPPFLAGS= $(BASEFLAGS)  $(E2SMFLAGS) 

$(ASN1C_MODULES): export CFLAGS = $(C_BASEFLAGS) -I$(ASN1C_DIR)/ 

$(X2AP_OBJ) $(SGNB_ADD_REQ_UNIT_TEST)  : export CPPFLAGS= $(X2FLAGS)  $(BASEFLAGS)

$(PLUGINS_OBJ) : export CPPFLAGS = $(BASEFLAGS) $(JSONFLAGS)

$(MOCK_E2TERM_OBJ): export CPPFLAGS = $(BASEFLAGS) $(SUBSCRFLAGS) $(E2FLAGS) $(E2SMFLAGS) $(XAPPFLAGS) $(X2FLAGS)

#$(JSON_OBJ) $(JSON_UNIT_TEST): export CPPFLAGS= $(BASEFLAGS) $(JSONFLAGS)

$(PROTECTOR_OBJ) $(SLIDING_WINDOW_UNIT_TEST) $(PROTECTOR_UNIT_TEST) $(ADMISSION_UNIT_TEST): export CPPFLAGS = $(BASEFLAGS) $(PLUGINFLAGS)  $(PROTECTORFLAGS) $(X2FLAGS)


TEST_XAPP: $(XAPP_UTILS_OBJ) $(XAPP_UNIT_TEST)
	$(CXX)  -o $@  $^ $(LIBS) $(LOGLIBS)

TEST_SUBSCRIPTION_FLOW: $(SUBSCRIPTION_FLOW_UNIT_TEST) $(SUBSCR_OBJ) $(E2SM_OBJ) $(ASN1C_MODULES)
		$(CXX)  -o $@  $^ $(LIBS) $(LOGLIBS)

TEST_SUBSCRIPTION:  $(SUBSCR_OBJ) $(E2SUB_UNIT_TEST) $(ASN1C_MODULES)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_E2AP_INDICATION:  $(E2AP_C_OBJ) $(E2AP_IND_UNIT_TEST) $(ASN1C_MODULES)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_E2AP_CONTROL:  $(E2AP_C_OBJ) $(E2AP_CTRL_UNIT_TEST) $(ASN1C_MODULES)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_E2SM: $(E2SM_OBJ) $(E2SM_UNIT_TEST) $(ASN1C_MODULES)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_X2_SGNB:  $(X2AP_OBJ) $(SGNB_ADD_REQ_UNIT_TEST) $(ASN1C_MODULES)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_JSON:  $(JSON_OBJ)  $(JSON_UNIT_TEST)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_MESSAGE_PROCESSOR: $(MESSAGE_PROC_UNIT_TEST) $(XAPP_MPROC_OBJ) $(ASN1C_MODULES) $(E2AP_C_OBJ) $(PROTECTOR_OBJ) $(E2SM_OBJ) $(X2AP_OBJ) $(SUBSCR_OBJ) $(JSON_OBJ) $(PLUGINS_OBJ)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_ADMISSION:  $(ADMISSION_UNIT_TEST)  $(PROTECTOR_OBJ) $(X2AP_OBJ) $(ASN1C_MODULES) $(PLUGINS_OBJ) 
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_PROTECTOR_PLUGIN: $(PROTECTOR_UNIT_TEST) $(PROTECTOR_OBJ) $(X2AP_OBJ) $(ASN1C_MODULES) $(PLUGINS_OBJ) $(JSON_OBJ)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

TEST_SLIDING_WINDOW: $(SLIDING_WINDOW_UNIT_TEST) $(PROTECTOR_OBJ) $(X2AP_OBJ) $(ASN1C_MODULES) $(PLUGINS_OBJ) $(JSON_OBJ)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)


mock-e2term-server: $(XAPP_UTILS_OBJ) $(E2AP_C_OBJ) $(SUBSCR_OBJ) $(E2SM_OBJ) $(X2AP_OBJ) $(MOCK_E2TERM_OBJ) $(ASN1C_MODULES)
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS) $(CPPFLAGS)


mock-a1-tool: $(A1_OBJ) $(MOCK_A1SERVER_OBJ) $(XAPP_UTILS_OBJ) 
	$(CXX)  -o $@  $^ $(LIBS)  $(LOGLIBS)

all_tests: TEST_XAPP TEST_SUBSCRIPTION TEST_ADMISSION TEST_E2AP_INDICATION TEST_E2AP_CONTROL TEST_E2SM TEST_X2_SGNB TEST_JSON TEST_SUBSCRIPTION_FLOW TEST_MESSAGE_PROCESSOR TEST_SLIDING_WINDOW TEST_PROTECTOR_PLUGIN


install: mock-e2term-server
	install -D mock-e2term-server /usr/local/bin/mock-e2term-server
clean:
	-rm TEST_* *.o e2e-test-client mock-e2term-server e2e-perf-client e2e-perf-server mock-a1-tool
	-cd ../src && make -f ../src/Makefile clean
