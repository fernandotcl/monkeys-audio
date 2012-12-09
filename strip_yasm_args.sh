#!/bin/sh
set -e
command=""
while test -n "$1"; do
	if test "$1" != "-fPIC" -a "$1" != '-DPIC' -a "$1" != '-fno-common'; then
        command="$command $1"
	fi
	shift
done
echo $command
exec $command
