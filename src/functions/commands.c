/*
 * commands.c
 *
 *  Created on: 11.09.2020
 *      Author: gei
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
//#include <commands.h>


int init_cmd_listener(int *fds, int port, struct sockaddr_in *dataAddress) {

    int s = 1;

    if ((*fds = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "cmd side - socket create failed: %m");
        return 1;
    }
    if (setsockopt(*fds, SOL_SOCKET, SO_REUSEADDR,
            (char *) &s, sizeof(s)) < 0) {
        syslog(LOG_ERR, "cmd side - set sockopt fail: %m");
    }
    dataAddress->sin_family = AF_INET;
    dataAddress->sin_addr.s_addr = /*INADDR_ANY*/ inet_addr("127.0.0.1");
    dataAddress->sin_port = htons(port);
    if (bind(*fds, (struct sockaddr *) dataAddress, sizeof(*dataAddress)) < 0) {        syslog(LOG_ERR, "cmd side - cannot bind: %m");
        return 1;
    }
    syslog(LOG_DEBUG, "cmd side - listening on %d", port);
    if (listen(*fds, 10) < 0) {
        syslog(LOG_ERR, "cmd side - cannot listen: %m");
        return 1;
    }
    return 0;
}
