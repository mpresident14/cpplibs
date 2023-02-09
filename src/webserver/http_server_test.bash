#!/bin/bash

# https://stackoverflow.com/questions/53472993/how-do-i-make-a-bazel-sh-binary-target-depend-on-other-binary-targets
# --- begin runfiles.bash initialization ---
# Copy-pasted from Bazel's Bash runfiles library (tools/bash/runfiles/runfiles.bash).
set -euo pipefail
if [[ ! -d "${RUNFILES_DIR:-/dev/null}" && ! -f "${RUNFILES_MANIFEST_FILE:-/dev/null}" ]]; then
  if [[ -f "$0.runfiles_manifest" ]]; then
    export RUNFILES_MANIFEST_FILE="$0.runfiles_manifest"
  elif [[ -f "$0.runfiles/MANIFEST" ]]; then
    export RUNFILES_MANIFEST_FILE="$0.runfiles/MANIFEST"
  elif [[ -f "$0.runfiles/bazel_tools/tools/bash/runfiles/runfiles.bash" ]]; then
    export RUNFILES_DIR="$0.runfiles"
  fi
fi
if [[ -f "${RUNFILES_DIR:-/dev/null}/bazel_tools/tools/bash/runfiles/runfiles.bash" ]]; then
  source "${RUNFILES_DIR}/bazel_tools/tools/bash/runfiles/runfiles.bash"
elif [[ -f "${RUNFILES_MANIFEST_FILE:-/dev/null}" ]]; then
  source "$(grep -m1 "^bazel_tools/tools/bash/runfiles/runfiles.bash " \
            "$RUNFILES_MANIFEST_FILE" | cut -d ' ' -f 2-)"
else
  echo >&2 "ERROR: cannot find @bazel_tools//tools/bash/runfiles:runfiles.bash"
  exit 1
fi
# --- end runfiles.bash initialization ---


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

$(rlocation cpplibs/src/webserver/main) > /dev/null &


test_get_file
test_post_json

kill %1
if [[ $FAILURES == "0" ]]; then
  exit 0
else
  exit 1
fi


