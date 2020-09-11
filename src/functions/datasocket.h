/*
 * datas socket for jetdirect
 */


#ifndef DATASOCKET_H
#define DATASOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <netinet/in.h>


int init_data_listener(int *fds, int port);

int new_data_socket(int verb, int master_fd, int *socket );

#ifdef __cplusplus
}
#endif

#endif /* DATASOCKET_H */
