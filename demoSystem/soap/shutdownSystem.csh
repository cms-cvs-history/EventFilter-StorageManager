#!/bin/csh

# determine the EP and SM URLs from either input arguments or defaults
source ./setDemoUrlEnvVar.csh $argv[*]
if ("$BUILDER_UNIT_URL" == "" || "$FILTER_UNIT_URL" == "" || \
    "$FILTER_UNIT2_URL" == "" || "$FILTER_UNIT3_URL" == "" || \
    "$FILTER_UNIT5_URL" == "" || "$FILTER_UNIT6_URL" == "" || \
    "$FILTER_UNIT7_URL" == "" || "$FILTER_UNIT8_URL" == "" || \
    "$FILTER_UNIT4_URL" == "" || "$STORAGE_MANAGER_URL" == "" || \
    "$SM_PROXY_URL" == "" || "$CONSUMER_FU_URL" == "") then
    exit
endif

./globalStop.csh

sleep 5

./globalHalt.csh
