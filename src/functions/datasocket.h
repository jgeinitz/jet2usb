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


int init_data_listener(int *fds, int port,struct sockaddr_in6 *dataAddress);



#ifdef __cplusplus
}
#endif

#endif /* DATASOCKET_H */
