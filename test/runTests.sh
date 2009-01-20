#!/usr/bin/env sh

tests="fsmSoap/fsmSoap.tcl"

error=0

for test in $tests ; do
    startTime=`date "+%H:%M:%S"`
    printf "%-32s: $startTime " $test
    
    if $test > $test.log 2>&1
	then
	date "+%H:%M:%S Passed"
    else
	date "+%H:%M:%S FAILED"
	error=1
    fi
done

exit $error
