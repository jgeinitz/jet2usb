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
    
    openlog(NULL, LOG_CONS | LOG_PID, LOG_LPR);

    parseCommandline();
    setup(me, Version);
    prepareSockets();
    while ( !terminating ) {
        prepareSelect();
        switch ( performSelect() ) {
            case timeout:
                selectTimeout();
                break;
            case readData:
                readDataFromSocket();
                break;
            case readCmd:
                readCommandSocket();
                break;
            case readPrinter:
                readDataFromPrinter();
                break;
            case writePrinter:
                writeDataToPrinter();
                break;
            case writeData:
                writeDataToSocket();
                break;
/*            case 6:
                writeCommandToPrinter();
                break;
 */
            case writeCmd:
                writeCommandSocket();
                break;
            case newData:
                acceptDataSocket();
                break;
            case newCmd:
                acceptCommandSocket();
                break;
            case finish:
                terminating = terminateRequest();
                break;
            default:
                printf("%s: select error -- bailing out\n",me);
                syslog(LOG_ERR,"select error");
                terminating = 1;
        }
    }
    /**********************************************
     * bye
     **********************************************/
    syslog(LOG_INFO, "program ends.");
    return (EXIT_SUCCESS);
}
