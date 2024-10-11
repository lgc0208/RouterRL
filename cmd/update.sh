#!/bin/bash
###
 # @Author       : LIN Guocheng
 # @Date         : 2024-04-10 11:02:38
 # @LastEditors  : LIN Guocheng
 # @LastEditTime : 2024-10-09 04:03:27
 # @FilePath     : /root/RouterRL/cmd/update.sh
 # @Description  : Update configures in omnetpp and inet.
### 
path=$(pwd)
cp -r $path/modules/ipv4/* $INET_ROOT/src/inet/networklayer/ipv4/
cp -r $path/modules/udpapp/* $INET_ROOT/src/inet/applications/udpapp/
cp -r $path/modules/init_config/* $INET_ROOT/src/inet/networklayer/configurator/ipv4/

cd $INET_ROOT
make makefiles
make -j32
. setenv

cd $path