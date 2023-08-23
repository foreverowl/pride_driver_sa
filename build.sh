#!/usr/bin/env bash
BASEDIR=$(dirname "$0")
g++ $BASEDIR/main/main.cpp $(find $BASEDIR/src -name "*.cpp") -I $BASEDIR/include -fpermissive -o $BASEDIR/sa `llvm-config --cxxflags --libs --ldflags --system-libs`
