#!/bin/bash

FAILURES=0

function assert_eq() {
  if [[ "$1" == "$2" ]]; then
    echo "pass"
  else
    diff  <(echo "$1" ) <(echo "$2")
    FAILURES=$((FAILURES+1))
    echo "fail"
  fi
}

function test_get_file() {
  echo "Running $FUNCNAME"

  OUT=`curl http://localhost:8080 -H "h1: test" --silent`

  EXPECTED=`cat src/webserver/testfiles/index.html`
  assert_eq "$OUT" "$EXPECTED"

  echo
}

function test_post_json() {
  echo "Running $FUNCNAME"

  OUT=`curl http://localhost:8080/echo -H "h1: test" -H "h2  : test" --silent -d '{"echo": "hello"}'`

  EXPECTED="hello"
  assert_eq "$OUT" "$EXPECTED"

  echo
}

bazel build //src/webserver:main
bazel-bin/src/webserver/main > /dev/null &


test_get_file
test_post_json

kill %1
if [[ $FAILURES == "0" ]]; then
  exit 0
else
  exit 1
fi


