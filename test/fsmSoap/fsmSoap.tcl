#!/usr/bin/env expect

# Edit the following line to change the order of state changes
set stateCmds "Configure Enable Halt Configure Enable Stop Halt"


set stateMap(Configure) Ready
set stateMap(Stop)      Ready
set stateMap(Enable)    Enabled
set stateMap(Halt)      Halted

spawn xdaq.exe -p 27000 -c fsmSoap/fsmSoap.xml
expect {
    "Entering Halted inner state"  {send_user "halted state\n"}
    timeout                          {send_user "FAILED: not reached halted state\n"; exit 1}
}


if [fork] {
    # the parent process gets the child process id

    foreach stateCmd $stateCmds {
	expect {
	    "Entering $stateMap($stateCmd) inner state" {}
	    timeout {send_user "FAILED: did not reach $stateMap($stateCmd) state\n"; exit 1}
	}
    }
    
} else {
    # the child process

    foreach stateCmd $stateCmds {
	spawn ./demoSystem/soap/sendSimpleCmdToApp srv-C2D05-05 27000 stor::fsmSoap 0 $stateCmd
	expect {
	    "$stateCmd: EMPTY SOAP MESSAGE" {}
	    timeout {send_user "FAILED: soap failure sending $stateCmd\n"; exit 1}
	}
    }
}

exit 0
