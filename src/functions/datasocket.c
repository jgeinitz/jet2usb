/*
 * datasocket.c
 *
 *  Created on: 11.09.2020
 *      Author: gei
 */

#include "datasocket.h"

static struct sockaddr_in6 dataAddress;

int init_data_listener(int *fds,int port) {

    int opt = 1;

    if ((*fds = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "data side - socket create failed %m");
        return 1;
    }

    if (setsockopt(*fds, SOL_SOCKET, SO_REUSEADDR,
            (char *) &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "data side - set sockopt fail: %m");
    }

    dataAddress.sin6_family = AF_INET6;
    dataAddress.sin6_addr   = in6addr_any;
    dataAddress.sin6_port   = htons(port);

    if (bind(*fds,(struct sockaddr *) &dataAddress, sizeof(dataAddress) ) < 0) {
        syslog(LOG_ERR, "data side - cannot bind: %m");
        return 1;
    }

    if (listen(*fds, 5) < 0) {
    	syslog(LOG_ERR, "data side - cannot listen: %m");
        return 1;
    }
    return 0;
}

int new_data_socket(int verb, int master_fd, int *socket )
{
    socklen_t addrlen;

    if (verb)
        syslog(LOG_DEBUG,"new data socket request");

    if (*socket != 0) {
        syslog(LOG_ERR, "can only handle one connection at a time");
        return 1;
    }
    addrlen = sizeof (dataAddress);
    if ( (*socket = accept(master_fd,
    		(struct sockaddr *) &dataAddress,
			&addrlen))
    		< 0
    ) {
    	syslog(LOG_ERR, "accept error: %m");
    	return 1;
    }
    if (verb)
    	syslog(LOG_INFO, "new data socket %d\n", *socket);
    return 0;
}


#define RBUFS 2049
int copyTo(int *from, int *to, int verbose, int testmode) {
    int bytes;
    char buffer[RBUFS];
    if ( (bytes=read(*from,buffer,RBUFS-1)) < 0 ) {
        if ( verbose ) {
            syslog(LOG_INFO,"read error on socket");
        }
        if ( *from ) close(*from);
        *from=0;
        return 1;
    }
    if ( bytes > 0 ) {
        if ( verbose ) {
            syslog(LOG_INFO,"rcv'd %d bytes from socket %d will send to %d",
                    bytes, *from, *to);
        }
        //guessJob(buffer,255);
        if ( testmode || (verbose & 64) ) {
            int i;
            fprintf(stderr,"++++++++\n");
            for (i=0; i<bytes; i++ ) {
                fprintf(stderr, "%02X %c ", (unsigned short int)buffer[i], (unsigned char) (buffer[i] & 0xff) );
            }
            fprintf(stderr,"\n++++----\n");
            fflush(stderr);
        }
        if ( ! testmode ) {
            if ( write(*to,buffer,bytes) < 0 ) {
                syslog(LOG_ERR,"write error to printer %m");
                close(*to);
                *to=-1;
                if ( *from ) close(*from);
                *from=0;
                return 1;
            }
        }
        return 0;
    } else { // bytes MUST be 0
        if ( verbose )
            syslog(LOG_INFO,"received 0 bytes assume EOF");
        if ( *from ) close(*from);
        *from=0;
        return 1;
    }
}

