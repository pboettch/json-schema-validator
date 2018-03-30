#!/bin/bash

PWD=$(realpath `dirname $0`/../JSON-Schema-Test-Suite/tests/draft4)

TESTS=`find $PWD | grep json$`

FAILCOUNT=0

for T in $TESTS
do
    ./json-schema-test < $T
    FAILCOUNT=$(($FAILCOUNT + $?))
done

echo $FAILCOUNT tests failed

