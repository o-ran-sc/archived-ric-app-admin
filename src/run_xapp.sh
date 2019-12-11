#! /bin/bash
SAMPLES=../schemas/samples.json
A1_SCHEMA=../schemas/rate-control-policy.json
VES_SCHEMA=../schemas/ves_schema.json
VES_POST_INTERVAL=10
#VES_URL=http://192.168.100.34:9200/acxapp/_doc
VES_URL=http://127.0.0.1:6350
THREADS=1
gNodeB=NYC123
OP_MODE=REPORT
XAPP_ID="ric-xapp-1-sjwn";
#echo "Running ./adm-ctrl-xapp -s $SAMPLES -a $A1_SCHEMA -i $VES_POST_INTERVAL -g $gNodeB -v $VES_SCHEMA -t $THREADS";
./adm-ctrl-xapp -s $SAMPLES -a $A1_SCHEMA -u $VES_URL -i $VES_POST_INTERVAL -g $gNodeB -v $VES_SCHEMA -t $THREADS -c $OP_MODE -x $XAPP_ID --verbose 
