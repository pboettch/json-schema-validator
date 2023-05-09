#!/bin/bash -eux

tmp_dir=$(mktemp -d)
cmake -S ./FetchContent -B ${tmp_dir}
