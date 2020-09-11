/*
 * output.c
 *
 *  Created on: 11.09.2020
 *      Author: gei
 */
#include <stdio.h>
#include <syslog.h>

FILE *pfd = NULL;



int testprinter(char *p, int tst) {

    if ( tst ) {
        syslog(LOG_DEBUG, "testprinter: simulated printer is ok\n");
        return 0;
    }
    if ( (pfd=fopen(p,"r+")) == NULL ) {
    	syslog(LOG_ERR,"cannot open %s \"%m\"", p);
        return 1;
    }
    return 0;
}


