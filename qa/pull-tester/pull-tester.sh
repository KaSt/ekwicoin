#!/bin/sh
# Copyright (c) 2013 The Ekwicoin Core developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#
# Helper script for pull-tester.
#Param 1: path to ekwicoin srcroot
#Param ...: arguments for build-test.sh

if [ $# -lt 1 ]; then
  echo "usage: $0 [ekwicoin srcroot] build-test arguments..."
fi

killall -q ekwicoin-cli
killall -q ekwicoind

cd $1
shift

./autogen.sh
./configure
./qa/pull-tester/build-tests.sh "$@"
