#!/bin/sh
# $Id: setup_sm.sh,v 1.15 2008/08/07 15:54:10 jserrano Exp $

if test -e "/etc/profile.d/sm_env.sh"; then 
    source /etc/profile.d/sm_env.sh;
fi

###
echo     5 > /proc/sys/vm/dirty_background_ratio
echo    15 > /proc/sys/vm/dirty_ratio
echo   128 > /proc/sys/vm/lower_zone_protection
echo 16384 > /proc/sys/vm/min_free_kbytes
###

store=/store
if test -n "$SM_STORE"; then
    store=$SM_STORE
fi
lookarea=$store/lookarea
if test -n "$SM_LOOKAREA"; then
    lookarea=$SM_LOOKAREA
fi

hname=`hostname | cut -d. -f1`;
nname="node"`echo $hname | cut -d- -f3` 

if test -x "/sbin/multipath"; then
    echo "Refresh multipath devices"
    /sbin/multipath
fi

case $hname in
    cmsdisk0)
        echo "cmsdisk0 needs manual treatment"
        exit 0;
        ;;
#    cmsdisk1)
#        nname=node_cmsdisk1
#        ;;
    srv-S2C17-01)
        nname=node_cms-tier0-stage
        ;;
    
    srv-C2D05-02)
        nname=node_cmsdisk1
	for i in $store/satacmsdisk*; do 
            sn=`basename $i`
            if test -z "`mount | grep $sn`"; then
                echo "Attempting to mount $i"
                mount -L $sn $i
            fi
	done
        ;;
    srv-c2c07-* | srv-C2C07-*)
	for i in $store/sata*a*v*; do 
            sn=`basename $i`
            if test -z "`mount | grep $sn`"; then
                echo "Attempting to mount $i"
                mount -L $sn $i
            fi
	done
        ;;
    *)
        echo "Unknown host: $hname"
        exit 1;
        ;;
esac

if test -n "$SM_LA_NFS" -a "$SM_LA_NFS" != "local"; then
    if test -z "`mount | grep $lookarea`"; then
        mkdir -p $lookarea
        chmod 000 $lookarea
        echo "Attempting to mount $lookarea"
        mount -t nfs -o rsize=32768,wsize=32768,timeo=14,intr $SM_LA_NFS $lookarea
    fi
fi

if test -n "$SM_CALIB_NFS" -a -n "$SM_CALIBAREA"; then
    if test -z "`mount | grep $SM_CALIBAREA`"; then
        mkdir -p $SM_CALIBAREA
        chmod 000 $SM_CALIBAREA
        echo "Attempting to mount $SM_CALIBAREA"
        mount -t nfs -o rsize=32768,wsize=32768,timeo=14,intr $SM_CALIB_NFS $SM_CALIBAREA
    fi
fi

if test -x "/sbin/multipath"; then
    echo "Flushing unused multipath devices"
    /sbin/multipath -F
fi

su - cmsprod -c "~cmsprod/$nname/t0_control.sh stop" >/dev/null 2>&1
su - cmsprod -c "~cmsprod/$nname/t0_control.sh start"
su - smpro -c "~smpro/scripts/t0inject.sh stop" >/dev/null 2>&1
rm -f /tmp/.20*-${hname}-*.log.lock
su - smpro -c "~smpro/scripts/t0inject.sh start"
