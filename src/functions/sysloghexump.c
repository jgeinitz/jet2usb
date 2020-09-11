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
#include <unistd.h>
#include <errno.h>
#include <string.h>

void sysloghexdump(char *buffer, int len) {
    int i, j;
    int offset;
    char outbuffer1[16][4];
    char outbuffer2[32];
    char outbuffer3[16];
    char sendit[512];

    for (j = 0; j < 512; j++)
        sendit[j] = '\0';
    i=0;
    syslog(LOG_INFO,"will dump %d bytes", len);
    do {
        if ((i % 16) == 0) {
            for (j = 0; j < 16; j++)
                (void)strncpy(outbuffer1[j], "  ", 2);
            offset = i;
        }
        
        if ( i < len ) {
            sprintf(outbuffer1[(i % 16)], "%02X ",
                    (unsigned short int) buffer[i]);
                
            if ( (buffer[i] >= ' ') && (buffer[i] <= 'z' )) {
                sprintf(outbuffer3, "%c", (char) buffer[i]);
                (void) strcat(outbuffer2, outbuffer3);
            } else
                (void) strcat(outbuffer2, ".");
        }
        
        if (((i % 16) == 15) || (i >= len)) {
            sprintf(sendit, "%05d ", offset);
            for (j = 0; j < 16; j++) {
                (void) strcat(sendit, outbuffer1[j]);
            }
            (void) strcat(sendit, " ");
            (void) strcat(sendit, outbuffer2);
            syslog(LOG_INFO, "%s",sendit);
            for (j = 0; j < 512; j++)
                sendit[j] = '\0';
        }
        i++;
    } while ( i < len );
}

