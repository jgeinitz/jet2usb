/*
 * datas socket for jetdirect
 */


#ifndef DATASOCKET_H
#define DATASOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>



int init_data_listener(int *fds, int port);

int new_data_socket(int verb, int master_fd, int *socket );

int copyTo(int *from, int to, int verbose, int tst);


#ifdef __cplusplus
}
#endif

#endif /* DATASOCKET_H */
