/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fun.h
 * Author: gei
 *
 * Created on 21. September 2018, 17:46
 */

#ifndef FUN_H
#define FUN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


    void setup();
    void parseCommandline();
    void prepareSockets();
    void prepareSelect();
    int performSelect();
    void readDataFromSocket();
    void readCommandSocket();
    void readDataFromPrinter();
    void writeDataToPrinter();
    void writeDataToSocket();
    void writeCommandToPrinter();
    void writeCommandSocket();
    void acceptDataSocket();
    void acceptCommandSocket();
    int terminateRequest();
    
    
#ifdef __cplusplus
}
#endif

#endif /* FUN_H */

