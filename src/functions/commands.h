/*
 * command channel
 *
 */


int init_cmd_listener(int *fds, int port,struct sockaddr_in *dataAddress);
int talkTo(int sd, int *t, int v);

