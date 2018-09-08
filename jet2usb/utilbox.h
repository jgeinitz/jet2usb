/* 
 * File:   utilbox.h
 * Author: gei
 *
 * Created on 25. August 2018, 07:35
 */

#ifndef UTILBOX_H
#define UTILBOX_H

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/socket.h>
#include <netinet/in.h>

int parsearguments(int ac, char **av,
        int *verbose,
        int *jetport,
        int *cmdport,
        int *testmode,
        char *printer,
        int printermaxlen);
int testprinter(char *printer, int testmode);
int init_data_listener(int *fds, int port,struct sockaddr_in6 *dataAddress);
int init_cmd_listener(int *fds, int port,struct sockaddr_in *dataAddress);
int talkTo(int sd, int *t, int v);
int copyTo(int from, int to, int verbose, int tst);

void resetGuesser();
void guessJob(char *b, int l);
int isPS();

#ifdef __cplusplus
}
#endif

#endif /* UTILBOX_H */

