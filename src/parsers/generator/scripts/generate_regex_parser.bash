#!/bin/bash


INCLUDE_DIR="src/parsers/generator/"
PARSER_DIR=$(pwd)/"$INCLUDE_DIR"

bazel build //src/parsers/generator:parsegen
cp "$PARSER_DIR"/regex_parser.hpp "$PARSER_DIR"/obsolete/regex_parser_backup.hpp
cp "$PARSER_DIR"/regex_parser.cpp "$PARSER_DIR"/obsolete/regex_parser_backup.cpp
bazel run //src/parsers/generator:parsegen -- -g "$PARSER_DIR"/regex.pgen -d "$PARSER_DIR" -i "$INCLUDE_DIR"  -n regex_parser
clang-format -i "$PARSER_DIR"/regex_parser.hpp "$PARSER_DIR"/regex_parser.cpp
