#!/usr/bin/env python
# coding=utf-8

import os, sys
import time

#天数
N = 2

#打开文件
path = "./LOG/"
dirs = os.listdir(path)

#print(time.time())

#输出所有文件和文件夹
for file in dirs:
    filename = os.path.join(path, file)
    lastmodifytime = os.stat(filename).st_mtime
    endfiletime = time.time() - 3600 * 24 * N
    if endfiletime > lastmodifytime:
        os.remove(filename)
        #print(filename)

