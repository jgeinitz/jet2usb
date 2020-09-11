/*
 * command channel
 *
 */


int init_cmd_listener(int *fds, int port);
int new_command_socket(int verb, int master);
int get_max_cmd_socket(int old_max_sd,fd_set *readfds);
void process_command(int verbose, fd_set *readfds, int *terminate);
