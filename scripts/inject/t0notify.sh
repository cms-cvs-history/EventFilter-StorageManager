#!/bin/sh
#$Id: t0notify.sh,v 1.4 2010/07/21 09:58:29 babar Exp $

. /etc/init.d/functions

# base dir for code
export SMT0_BASE_DIR=/nfshome0/smpro/sm_scripts_cvs/inject

# db config file
export SMT0_CONFIG=/nfshome0/smpro/configuration/db.conf

#local run dir
export SMT0_LOCAL_RUN_DIR=/nfshome0/smpro/t0inject

if test -d "/opt/injectworker"; then
    export SMT0_BASE_DIR=/opt/injectworker/inject
    export SMT0_CONFIG=/opt/injectworker/.db.conf
    export SMT0_LOCAL_RUN_DIR=/store/injectworker
fi

if [ ! -d $SMT0_BASE_DIR ]; then
    echo "SMT0_BASE_DIR ($SMT0_BASE_DIR) does not exist or is no directory"
    exit
fi

# Application name
SM_APP=NotifyWorker

# main perl script
SMT0_SCRIPT=$SMT0_BASE_DIR/$SM_APP.pl

if [ ! -x $SMT0_SCRIPT ]; then
    echo "SM Tier0 script ($SMT0_SCRIPT)does not exist or is not executable"
    exit
fi

# directory to monitor
SMT0_MONDIR=$SMT0_LOCAL_RUN_DIR/logs

if [ ! -d $SMT0_MONDIR ]; then
    echo "SMT0_MONDIR ($SMT0_MONDIR) does not exist or is no directory"
    exit
fi

if [ ! -r $SMT0_CONFIG ]; then
    echo "SMT0_CONFIG ($SMT0_CONFIG) can not be read"
    exit
fi

#exported variables
export SM_NOTIFYSCRIPT=$SMT0_BASE_DIR/sendNotification.sh
export SM_HOOKSCRIPT=$SMT0_BASE_DIR/sm_hookscript.pl

#
# Define rules to start and stop daemons
#
start(){
    #
    # Setting up environment
    #
    mkdir -p ${SMT0_LOCAL_RUN_DIR}/logs
    mkdir -p ${SMT0_LOCAL_RUN_DIR}/done
    mkdir -p ${SMT0_LOCAL_RUN_DIR}/workdir

    ( # Double-fork
        cd ${SMT0_LOCAL_RUN_DIR}/workdir
        rm -f $SM_APP-`hostname`.*
        exec > $SM_APP-`hostname`.$$
        exec 2>&1
        exec </dev/null
        $SMT0_SCRIPT $SMT0_MONDIR $SMT0_LOCAL_RUN_DIR/logs $SMT0_CONFIG &
    )
}

stop(){
    gracetime=15
    pkill -15 -f -P 1 -u smpro $SMT0_SCRIPT
    while pgrep -f -P 1 -u smpro $SMT0_SCRIPT > /dev/null && [ $gracetime -gt 0 ]; do
        echo -n .
        let gracetime=$gracetime-1
        sleep 1
    done
    if [ $gracetime -eq 0 ]; then
        echo "$SM_APP did not terminate within 15 seconds, killing it!"
        pkill -9 -f -P 1 -u smpro $SMT0_SCRIPT
    fi
    if [ -f /tmp/.$SM_APP.lock ]; then
        echo 'Lockfile is still there!'
    fi
}

status(){
    pgrep -l -f -P 1 -u smpro $SMT0_SCRIPT
}

cleanup(){
    find $SMT0_LOCAL_RUN_DIR/{logs,done} -type f -name "*.log*" -exec rm -f {} \;
    rm -f $SMT0_LOCAL_RUN_DIR/workdir/$SM_APP*
}


# See how we were called.
case "$1" in
    start)
        start
        status
    ;;
    stop)
        stop
        test ! status
    ;;
    status)
        status
    ;;
    cleanup)
        cleanup
    ;;
    *)
        echo $"Usage: $0 {start|stop|status|cleanup}"
        RETVAL=1
    ;;
esac