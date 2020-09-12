/*
 * output to destination device
 *
 */

int testprinter(char *printer, int *printerfd, int testmode);
int printeropen(int *printer_fd);
int copyfrom(int *print_fd, int *datasocket, int verbose);
