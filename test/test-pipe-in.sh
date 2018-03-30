#!/bin/bash

# all argument are considered as a program to call (with its arguments),
# the last argument is read from via '<'

set -e

arr=( "$@" )

input=${arr[-1]}
unset 'arr[${#arr[@]}-1]'

${arr[@]} < $input
