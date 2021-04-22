#!/bin/bash

g++ -I ../include/ -g main.c src/conf.cpp src/server.cpp src/util.c src/http.cpp src/thread.cpp src/log.cpp  -lpthread
