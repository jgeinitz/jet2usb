/*
 * datasocket.c
 *
 *  Created on: 11.09.2020
 *      Author: gei
 */

//#include <datasocket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>


int init_data_listener(
		int *fds,
		int port,
		struct sockaddr_in6 *dataAddress ) {

    int opt = 1;

    if ((*fds = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "data side - socket create failed %m");
        return 1;
    }
    if (setsockopt(*fds, SOL_SOCKET, SO_REUSEADDR,
            (char *) &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "data side - set sockopt fail: %m");
    }
    dataAddress->sin6_family = AF_INET6;
    dataAddress->sin6_addr   = in6addr_any;
    dataAddress->sin6_port   = htons(port);
    if (bind(*fds,(struct sockaddr *) dataAddress, sizeof(*dataAddress) ) < 0) {
        syslog(LOG_ERR, "data side - cannot bind: %m");
        return 1;
    }
    syslog(LOG_DEBUG, "data side - listening on %d", port);
    if (listen(*fds, 5) < 0) {
    	syslog(LOG_ERR, "data side - cannot listen: %m");
        return 1;
    }
    return 0;
}
