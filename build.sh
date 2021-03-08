#!/bin/bash

g++ -I ../include/ -g main.c src/conf.cpp src/Server.cpp src/util.c src/http.cpp src/thread.cpp -DDEBUG -lpthread
