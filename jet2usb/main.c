/* 
 * File:   main.c
 * Author: Juergen Geinitz
 *
 * License: GPL
 * 
 * Started on 25. August 2018, 07:15
 */

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


char *me;

/*
 * 
 */
int main(int ac, char** av) {

#define P_MAX_LEN 64
    me = av[0];

    /**********************************************
     * bye
     **********************************************/
    syslog(LOG_INFO, "program ends.");
    return (EXIT_SUCCESS);
}
