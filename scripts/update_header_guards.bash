#!/bin/bash
cd $HOME/cs/cpplibs
for file in $(find src -type f)
do
  CAPS=$(echo "PREZ_${file#src/}" | tr '[:lower:]' '[:upper:]')
  GUARD=$(echo $CAPS | sed -E 's=(/|\.)=_=g')
  # [^()]*_HPP[^()] is kind've a heuristic more than exact
  sed -E -i "s=(#define|#ifndef) [^()]*_HPP[^()]*$=\1 $GUARD=g" $file
done
