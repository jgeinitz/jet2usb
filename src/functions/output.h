/*
 * output to destination device
 *
 */

int testprinter(char *printer, int *printerfd, int testmode);

int copyfrom(int *print_fd, int *datasocket, int verbose);
