#!/bin/sh
#
# 07-Jan-2009, KAB - script to configure hosts within a SM development system.

# check for valid arguments
if [ "$1" == "-h" ]
then
    echo "Usage: configureHosts.sh [cfgType [baseXdaqPort [hostNameOverride]]]"
    echo "where cfgType is one of 'python' or 'cfg', default is python"
    echo "e.g. 'configureHosts.sh python 50000 cmsroc8.fnal.gov'"
    exit
fi
if [ $# -gt 0 ]
then
    cfgType=$1
else
    cfgType="python"
fi
if [ $# -gt 1 ]
then
    basePort=$2
else
    basePort=50000
fi
if [ $# -gt 2 ]
then
    localHostName=$3
else
    localHostName=$HOSTNAME
fi
buPort=$(($basePort + 0))
fu1Port=$(($basePort + 1))
fu2Port=$(($basePort + 2))
fu3Port=$(($basePort + 3))
fu4Port=$(($basePort + 4))
fu5Port=$(($basePort + 5))
fu6Port=$(($basePort + 6))
fu7Port=$(($basePort + 7))
fu8Port=$(($basePort + 8))
smPort=$(($basePort + 1000))
smpsPort=$(($basePort + 1001))
consPort=$(($basePort + 1002))
buTcpPort=$(($basePort + 1010))
smTcpPort=$(($basePort + 1011))

fuCfgFile="fu_twoOut.py"
smCfgFile="sm_streams.py"
consCfgFile="fuConsumer.py"
if [ "$cfgType" != "python" ]
then
    fuCfgFile="fu_twoOut.cfg"
    smCfgFile="sm_streams.cfg"
    consCfgFile="fuConsumer.cfg"
fi

fileList="cfg/sm_autobu_8fu.xml cfg/eventConsumer.cfg cfg/eventConsumer.py cfg/proxyEventConsumer.cfg cfg/proxyEventConsumer.py cfg/dqmConsumer.cfg cfg/dqmConsumer.py cfg/proxyDQMConsumer.cfg cfg/proxyDQMConsumer.py cfg/fuConsumer.cfg cfg/fuConsumer.py bin/startEverything.csh bin/startEverything.sh soap/setDemoUrlEnvVar.csh soap/setRunNumbers.csh"

for filename in $fileList
do
    finalFile="$filename"
    workFile="$filename.tmp"
    inputFile="$filename.base"

    cp $inputFile $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_BU_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_BU_PORT/$buPort/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU_PORT/$fu1Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU2_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU2_PORT/$fu2Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU3_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU3_PORT/$fu3Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU4_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU4_PORT/$fu4Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU5_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU5_PORT/$fu5Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU6_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU6_PORT/$fu6Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU7_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU7_PORT/$fu7Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_FU8_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_FU8_PORT/$fu8Port/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_SM_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_SM_PORT/$smPort/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_SMPROXY_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_SMPROXY_PORT/$smpsPort/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/STMGR_DEV_CONSFU_HOST/$localHostName/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/STMGR_DEV_CONSFU_PORT/$consPort/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/BU_ENDPOINT_PORT/$buTcpPort/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/SM_ENDPOINT_PORT/$smTcpPort/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/FU_CFG_FILE/$fuCfgFile/" > $workFile
    rm -f $finalFile
    cat $workFile | sed "s/SM_CFG_FILE/$smCfgFile/" > $finalFile

    rm -f $workFile
    cat $finalFile | sed "s/CONS_CFG_FILE/$consCfgFile/" > $workFile
    rm -f $finalFile
    cp $workFile $finalFile

    rm -f $workFile

    if [[ $finalFile =~ csh ]]
    then
        chmod +x $finalFile
    fi
done
