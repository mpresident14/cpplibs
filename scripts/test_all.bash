#!/bin/bash
cd $HOME/cs/cpplibs
TESTS=""
for file in $(find src -type d -regex ".*tests$")
do
  TESTS+="//${file}:all "
done
bazel test $TESTS
