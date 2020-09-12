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

int parsearguments(
		int    ac,
		char **av,
        int   *verbose,
        int   *jetport,
        int   *cmdport,
        int   *testmode,
        char  *printer,
        int    printermaxlen);


void resetGuesser();

void guessJob(char *b, int l);

int isPS();

#ifdef __cplusplus
}
#endif

#endif /* UTILBOX_H */

