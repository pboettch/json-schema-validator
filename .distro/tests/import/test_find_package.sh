#!/bin/bash -eux

tmp_dir=$(mktemp -d)
cmake -S ./find_package -B ${tmp_dir}
