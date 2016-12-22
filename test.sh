#!/bin/sh

if [ ! -x "$1" ]
then
    exit 1
fi

if [ ! -e "$2" ]
then
    exit 1
fi

$1 < $2
