#!/bin/bash

function goto
{
	label=$1
	cmd=$(sed -n "/$label:/{:a;n;p;ba};" $0 | grep -v ':$')
	eval "$cmd"
	exit
}


clear

ERROR=${1:-"ERROR"}

Offset=0;

if [ $# != 2 ]; then
	goto $ERROR;
fi

if [ "$1" = "offset" ]; then
	offset=$2;
elif [ "$1" = "index" ]; then
	offset=$(($2*4096))
else
	goto $ERROR;
fi

llvm-pdbutil explain --offset=${offset} game.pdb
echo " "
exit

ERROR:
	echo "The corret syntax of the command is"
	echo "ipdb (offset or index) (size)"




