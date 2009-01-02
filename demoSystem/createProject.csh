#!/bin/csh
#
# This script creates a new project area based on the specified TAG,
# checks out the CVS modules releveant for storage manager development,
# and does the initial build.
#
# 26-Apr-2006, KAB
#

# constants
set BASE_DIR = `pwd`

# check for valid arguments
if ($#argv < 1 || $1 == "-h") then
    echo "Usage: createProject.csh <CMSSW_release_label_or_tag>"
    exit
endif

# check if the specified project already exists.  If so, prompt the user
# to decide whether to remove the old one and start fresh.
set tagName = $argv[1]
cd $BASE_DIR
if (-e $tagName && -d $tagName) then
    echo " "
    echo -n "Project $tagName already exists. Create a new copy (y/n [n])? "
    set response = $<
    set response = `echo ${response}n | tr "[A-Z]" "[a-z]" | cut -c1`
    if ($response == "y") then
        set timeStamp = `date '+%Y%m%d%H%M%S'`
        mv $tagName z.${tagName}.${timeStamp}
    else
        exit
    endif
endif

#usage# # prompt the user for a comment on how this project will be used
#usage# echo " "
#usage# echo -n "What will this project instance be used for? "
#usage# set projDesc = "$<"

echo "Creating project..."
source bin/uaf_setup.csh
source bin/cvs_setup.csh
scramv1 p CMSSW $tagName

echo "Checking out CVS modules..."
cd $tagName/src
#usage# rm -f usage.comment
#usage# echo "$projDesc" > usage.comment
#usage# chmod u-w usage.comment
source ../../bin/scram_setup.csh

# needed modules (need to create shared libraries)
cvs co -r $tagName EventFilter/AutoBU
cvs co -r $tagName EventFilter/Processor
cvs co -r $tagName EventFilter/ResourceBroker
cvs co -r $tagName EventFilter/StorageManager
cvs co -r $tagName EventFilter/SMProxyServer

# possibly needed modules (may need specific version)
cvs co -r $tagName IOPool/Streamer

# modules needed only for tweaks or development on them
#cvs co -r $tagName EventFilter/Modules
#cvs co -r $tagName EventFilter/Playback
#cvs co -r $tagName EventFilter/ShmBuffer
#cvs co -r $tagName EventFilter/Utilities
#cvs co -r $tagName FWCore/Framework
#cvs co -r $tagName FWCore/Modules
#cvs co -r $tagName IORawData/DaqSource

# 02-Jan-2009 - using the SM refdev01 work branch with 3_0_0_pre3
if ($tagName == "CMSSW_3_0_0_pre3") then
  cvs update -R -r refdev01_scratch_branch EventFilter/StorageManager
  cvs update -r 1.13 IOPool/Streamer/interface/HLTInfo.h
endif

# 02-Jan-2009 - using the SM refdev01 work branch with 2_1_11
if ($tagName == "CMSSW_2_1_11") then
  cvs update -R -r CMSSW_3_0_0_pre2 EventFilter/AutoBU
  cvs co -r CMSSW_3_0_0_pre2 EventFilter/Modules
  cvs update -R -r CMSSW_3_0_0_pre2 EventFilter/Processor
  cvs update -R -r CMSSW_3_0_0_pre2 EventFilter/ResourceBroker
  cvs co -r CMSSW_3_0_0_pre2 EventFilter/ShmBuffer
  cvs co -r CMSSW_3_0_0_pre2 EventFilter/ShmReader
  cvs update -R -r CMSSW_3_0_0_pre2 EventFilter/SMProxyServer
  cvs co -r CMSSW_3_0_0_pre2 EventFilter/Utilities
  cvs co -r CMSSW_3_0_0_pre2 FWCore/Framework
  cvs co -r CMSSW_3_0_0_pre2 FWCore/MessageLogger
  cvs co -r CMSSW_3_0_0_pre2 FWCore/MessageService
  cvs co -r CMSSW_3_0_0_pre2 FWCore/Modules
  cvs co -r CMSSW_3_0_0_pre2 FWCore/ServiceRegistry
  cvs co -r CMSSW_3_0_0_pre2 FWCore/Version
  cvs update -R -r CMSSW_3_0_0_pre2 IOPool/Streamer
  cvs update -R -r refdev01_scratch_branch EventFilter/StorageManager
  cvs update -r 1.13 IOPool/Streamer/interface/HLTInfo.h
endif

echo "Applying development-specific patches..."
../../bin/applyDevelopmentPatches.pl

echo "Building..."
scramv1 build
