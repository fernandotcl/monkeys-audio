#!/bin/sh
export LIBTOOLIZE_OPTIONS=--quiet
exec autoreconf -fi "$@"
