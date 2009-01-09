#!/usr/bin/env sh

tests="fsmSoap/fsmSoap.tcl"

for test in $tests ; do
    startTime=`date "+%H:%M:%S"`
    printf "%-32s: $startTime " $test
    
#    $test > $test.log 2>&1
    if $test > $test.log 2>&1
	then
	date "+%H:%M:%S Passed"
    else
	date "+%H:%M:%S FAILED"
    fi
done
