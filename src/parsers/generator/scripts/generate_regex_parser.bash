#!/bin/bash

PARSER_DIR="src/parsers/generator/"

cp "$PARSER_DIR"/regex_parser.hpp "$PARSER_DIR"/obsolete/regex_parser_backup.hpp
cp "$PARSER_DIR"/regex_parser.cpp "$PARSER_DIR"/obsolete/regex_parser_backup.cpp
make parsegen
../bin/parsegen -g "$PARSER_DIR"/regex.pgen -d "$PARSER_DIR" -i "$PARSER_DIR"  -n regex_parser
clang-format -i "$PARSER_DIR"/regex_parser.hpp "$PARSER_DIR"/regex_parser.cpp
