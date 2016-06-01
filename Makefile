ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
INCLUDE_DIR=$(ROOT_DIR)/myMed

CXX=g++
CXXFLAGS=-Wall -I$(INCLUDE_DIR) -L$(ROOT_DIR)/cChord -lmymed 

all: testfile

testfile: testfile.o 
	$(CXX) $(CXXFLAGS) -o $@ $^

testfile.o: testfile.cpp 
	$(CXX) $(CXXFLAGS) -c $^