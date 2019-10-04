CXX:= g++ --std=c++14 -O2
CC:= gcc -O2

SRC:=./
E2AP_c:=./E2AP-c
E2SM := ./E2SM
X2AP:=./X2AP
SUBSCR:=$(E2AP_c)/subscription
JSON := ./json
PLUGININTERFACE :=./plugin-interface
CURL := ./curl
PLUGIN:=./protector-plugin
ASN1C_DEFS:=../asn1c_defs/all-defs

# Logging library and flags
CLOGFLAGS:= `pkg-config mdclog --cflags`
LOG_LIBS:= `pkg-config mdclog --libs`
CURL_LIBS:= `pkg-config libcurl --libs`

# Keep include dirs separate so we have transparency
BASEFLAGS=  -Wall  -std=c++14 $(CLOGFLAGS) 

XAPPFLAGS= -I./
ASN1C_FLAGS=-I$(ASN1C_DEFS) -DASN_DISABLE_OER_SUPPORT
SUBSCRFLAGS=  -I$(SUBSCR)
JSONFLAGS=  -I$(JSON) 
X2FLAGS=  -I$(X2AP) 
PLUGINFLAGS = -I$(PLUGIN)
PLUGININTERFACEFLAGS = -I$(PLUGININTERFACE)
CURLFLAGS = -I$(CURL)
E2AP_C_FLAGS = -I$(E2AP_c)
E2SM_FLAGS = -I$(E2SM)
C_BASEFLAGS=-Wall $(CLOGFLAGS) -DASN_DISABLE_OER_SUPPORT

LIBS= -lrmr_nng  -lnng -lpthread -lm $(LOG_LIBS) $(CURL_LIBS)
COV_FLAGS= -fprofile-arcs -ftest-coverage

XAPP_SRC= adm-ctrl-xapp.cc \
     xapp_utils.cc \
     message_processor_class.cc \
     get_config.cc

CURL_SRC=$(wildcard $(CURL)/*.cc)
JSON_SRC=$(wildcard $(JSON)/*.cc)
SUBSCR_SRC= $(wildcard $(SUBSCR)/*.cc)
X2AP_SRC = $(wildcard $(X2AP)/*.cc)
PLUGINS_SRC = $(wildcard $(PLUGIN)/*.cc)
PLUGININTERFACE_SRC = $(wildcard $(PLUGININTERFACE)/*.cc)
E2AP_C_SRC =$(wildcard $(E2AP_c)/*.cc)
E2SM_SRC = $(wildcard $(E2SM)/*.cc)
ASN1C_SRC= $(wildcard $(ASN1C_DEFS)/*.c)

CURL_OBJ = $(CURL_SRC:.cc=.o)
JSON_OBJ = $(JSON_SRC:.cc=.o)
XAPP_OBJ= ${XAPP_SRC:.cc=.o} 
SUBSCR_OBJ= ${SUBSCR_SRC:.cc=.o} 
X2AP_OBJ= ${X2AP_SRC:.cc=.o}
PLUGINS_OBJ = ${PLUGINS_SRC:.cc=.o}
PLUGININTERFACE_OBJ = $(PLUGININTERFACE_SRC:.cc=.o)
E2AP_C_OBJ = $(E2AP_C_SRC:.cc=.o)
E2SM_OBJ = $(E2SM_SRC:.cc=.o)
ASN1C_MODULES = $(ASN1C_SRC:.c=.o)

$(ASN1C_MODULES): export CFLAGS = $(C_BASEFLAGS) $(ASN1C_FLAGS)
$(CURL_OBJ):export CPPFLAGS=$(BASEFLAGS) $(CURLFLAGS)
$(JSON_OBJ):export CPPFLAGS= $(BASEFLAGS) $(JSONFLAGS)
$(PLUGININTERFACE_OBJ):export CPPFLAGS=$(BASEFLAGS) $(PLUGININTERFACEFLAGS) $(JSONFLAGS)
$(SUBSCR_OBJ): export CPPFLAGS=$(BASEFLAGS) $(ASN1C_FLAGS) $(E2AP_C_FLAGS) $(SUBSCRFLAGS)
$(E2AP_C_OBJ): export CPPFLAGS = $(BASEFLAGS) $(ASN1C_FLAGS) $(E2AP_C_FLAGS)
$(E2SM_OBJ): export CPPFLAGS = $(BASEFLAGS) $(ASN1C_FLAGS) $(E2SM_FLAGS)
$(X2AP_OBJ): export CPPFLAGS = $(BASEFLAGS) $(ASN1C_FLAGS) $(X2FLAGS)
$(PLUGINS_OBJ): export CPPFLAGS = $(BASEFLAGS) $(ASN1C_FLAGS) $(X2FLAGS) $(PLUGINFLAGS) $(PLUGININTERFACEFLAGS) $(JSONFLAGS) 
$(XAPP_OBJ):export CPPFLAGS=$(BASEFLAGS)  $(JSONFLAGS) $(SUBSCRFLAGS) $(PLUGINFLAGS) $(CURLFLAGS) $(E2AP_C_FLAGS) $(ASN1C_FLAGS) $(E2SM_FLAGS) $(X2FLAGS) $(XAPPFLAGS) $(PLUGININTERFACEFLAGS)

OBJ= $(SUBSCR_OBJ) $(XAPP_OBJ) $(JSON_OBJ)  $(PLUGINS_OBJ) $(CURL_OBJ) $(E2AP_C_OBJ) $(ASN1C_MODULES) $(E2SM_OBJ) $(X2AP_OBJ) $(PLUGININTERFACE_OBJ)

adm-ctrl-xapp: $(OBJ)
	$(CXX) -o $@  $(OBJ) $(LIBS) $(CPPFLAGS) $(CLOGFLAGS) 

install: adm-ctrl-xapp
	install  -D adm-ctrl-xapp  /usr/local/bin/adm-ctrl-xapp

clean:
	-rm *.o $(JSON)/*.o $(E2AP_c)/*.o $(SUBSCR)/*.o $(PLUGIN)/*.o $(E2SM)/*.o $(X2AP)/*.o $(CURL)/*.o $(PLUGININTERFACE)/*.o adm-ctrl-xapp
