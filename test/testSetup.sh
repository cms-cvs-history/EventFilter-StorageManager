#
# 02-Jan-2009, KAB - This script sets up a bash environment
# for storage manager development.
# It needs to be run from the root directory of the SM development area.
#
# 22-Jan-2009, MO  - changes to make the script work from the
#                    EventFilter/StorageManager/test directory

demoSystemDir=`pwd`/demoSystem
export STMGR_DIR="$demoSystemDir"

# initial setup
source $demoSystemDir/bin/uaf_setup.sh
source $demoSystemDir/bin/cvs_setup.sh

# check if this script is being run from inside a CMSSW project area
selectedProject=""
cmsswVersion=`pwd | sed -nre 's:.*/(CMSSW.*)/src/EventFilter/StorageManager/test.*:\1:p'`

if [[ "$cmsswVersion" != "" ]]
then
    selectedProject="../../../../../$cmsswVersion"

else
    # check for existing project areas.  Prompt the user to choose one.
    projectCount=`ls -1d CMSSW* | wc -l`
    if [ $projectCount -eq 0 ]
    then
        echo "No project areas currently exist; try createProjectArea.csh"
        return
    fi
    projectList=`ls -dt CMSSW*`
    if [ $projectCount -eq 1 ]
    then
        selectedProject=$projectList
    else
        echo " "
        echo "Select a project:"
        for project in $projectList
        do
            echo -n "  Use $project (y/n [y])? "
            read response
            response=`echo ${response}y | tr "[A-Z]" "[a-z]" | cut -c1`
            if [ "$response" == "y" ]
            then
                selectedProject=$project
                break
            fi
        done
    fi
fi
if [ "$selectedProject" == "" ]
then
    echo "No project selected.  Exiting..."
    return
fi

# set up the selected project
cd ${selectedProject}/src
source $demoSystemDir/bin/scram_setup.sh
cd - > /dev/null

# define useful aliases

alias startEverything="cd $demoSystemDir/bin; source ./startEverything.sh"

# this test may need to be more sophisticated if Python configs
# are enabled in 2_2_X...
if [[ $selectedProject =~ CMSSW_3_ ]]
then
    alias startConsumer="cd $demoSystemDir/log/client; cmsRun ../../cfg/eventConsumer.py"
    alias startConsumer1="cd $demoSystemDir/log/client1; cmsRun ../../cfg/eventConsumer.py"
    alias startConsumer2="cd $demoSystemDir/log/client2; cmsRun ../../cfg/eventConsumer.py"

    alias startProxyConsumer="cd $demoSystemDir/log/client1; cmsRun ../../cfg/proxyEventConsumer.py"

    alias startProxyDQMConsumer="cd $demoSystemDir/log/client; cmsRun ../../cfg/proxyDQMConsumer.py"

    alias startDQMConsumer="cd $demoSystemDir/log/client; cmsRun ../../cfg/dqmConsumer.py"
else
    alias startConsumer="cd $demoSystemDir/log/client; cmsRun ../../cfg/eventConsumer.cfg"
    alias startConsumer1="cd $demoSystemDir/log/client1; cmsRun ../../cfg/eventConsumer.cfg"
    alias startConsumer2="cd $demoSystemDir/log/client2; cmsRun ../../cfg/eventConsumer.cfg"

    alias startProxyConsumer="cd $demoSystemDir/log/client1; cmsRun ../../cfg/proxyEventConsumer.cfg"

    alias startProxyDQMConsumer="cd $demoSystemDir/log/client; cmsRun ../../cfg/proxyDQMConsumer.cfg"

    alias startDQMConsumer="cd $demoSystemDir/log/client; cmsRun ../../cfg/dqmConsumer.cfg"
fi

alias cleanupShm="FUShmCleanUp_t"
alias killEverything="killall -9 xdaq.exe; sleep 2; FUShmCleanUp_t; cd $demoSystemDir/bin; ./removeOldLogFiles.sh; ./removeOldDataFiles.sh; ./removeOldDQMFiles.sh; cd - > /dev/null"

alias globalConfigure="cd $demoSystemDir/soap; ./globalConfigure.sh"
alias globalEnable="cd $demoSystemDir/soap; ./globalEnable.sh"
alias globalStop="cd $demoSystemDir/soap; ./globalStop.sh"
alias globalHalt="cd $demoSystemDir/soap; ./globalHalt.sh"

# 02-Jan-2008 - if needed, create a shared memory key file so that we
# can use shared memory keys independent of other developers
keyDir="/tmp/$USER"
if [ ! -d $keyDir ]
then
    mkdir $keyDir
fi

keyFile="$keyDir/shmKeyFile"
touch $keyFile
export FUSHM_KEYFILE=$keyFile
export SMPB_SHM_KEYFILE=$keyFile

keyFile="$keyDir/semKeyFile"
touch $keyFile
export FUSEM_KEYFILE=$keyFile

# 02-Jan-2009 - define the number of FUs that we want
# Valid values are 1..8.
export SMDEV_FU_PROCESS_COUNT=2

# 02-Jan-2009 - define whether we want a big HLT config or not
# Valid values are 0 (small config) and 1 (big config)
export SMDEV_BIG_HLT_CONFIG=0