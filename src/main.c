/* 
 * File:   main.c
 * Author: Juergen Geinitz
 * $Id$
 *
 * License: GPL
 * 
 * Started on 25. August 2018, 07:15
 */
# include "fun.h"
# include "version.h"

char Version[512];
char *me;

/*
 * 
 */
int main(int ac, char** av) {
    int terminating = 0;
    
    me = av[0];
    sprintf(Version, "%s_%s.%s.%s.%s",
            me,
            VERSION_NAME,
            VERSION_MAJOR, VERSION_MINOR, VERSION_SUB);
    
    
    setup();
    parseCommandline();
    prepareSockets();
    while ( !terminating ) {
        prepareSelect();
        switch ( performSelect() ) {
            case 1:
                readDataFromSocket();
                break;
            case 2:
                readCommandSocket();
                break;
            case 3:
                readDataFromPrinter();
                break;
            case 4:
                writeDataToPrinter();
                break;
            case 5:
                writeDataToSocket();
                break;
            case 6:
                writeCommandToPrinter();
                break;
            case 7:
                writeCommandSocket();
                break;
            case 8:
                acceptDataSocket();
                break;
            case 9:
                acceptCommandSocket();
                break;
            default:
                terminating = terminateRequest();
                break;
        }
    }
    /**********************************************
     * bye
     **********************************************/
    syslog(LOG_INFO, "program ends.");
    return (EXIT_SUCCESS);
}
