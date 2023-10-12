#!/bin/bash

#Dr Hauschilds script to clean up shared memory leaks
ME='whoami'

IPCS_S=`ipcs -s | egrep "0x[0-9a-f]+ [0-9]+" | grep $ME | cut -f2 -d" "`
IMCS_M=`ipcs -m | egrep "0x[0-9a-f]+ [0-9]+" | grep $ME | cut -f2 -d" "`
IPCS_Q=`ipcs -q | egrep "0x[0-9a-f]+ [0-9]+" | grep $ME | cut -f2 -d" "`

echo $IPCS_S

echo $IPCS_M

echo $IPCS_Q

for id in $IPCS_M; do
	ipcrm -m $id;
done

for id in $IPCS_S; do
	ipcrm -s $id;
done

for id in $IPCS_Q; do
	ipcrm -q $id;
done
