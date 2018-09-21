/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
# include "fun.h"

typedef struct t_MyString {
    int     len;
    int     maxlen;
    uint8_t *buffer;
} MyString;

typedef struct t_MyStringListe {
    struct t_MyStringListe *next;
    MyString               *text;
} MyStringListe;


MyString *newMyString(int maxlen) {
    MyString *t;
    uint8_t  *c;
    if ( (t=malloc(sizeof(MyString))) == NULL ) {
        fprintf(stderr,"PANIC: OUT OF MEMORY");
        syslog(LOG_ERR,"OUT OF MEMORY");
        exit(250);
    }
    t->maxlen = maxlen;
    t->len = 0;
    if ( (c=malloc(maxlen*(sizeof(uint8_t)))) == NULL ) {
        fprintf(stderr,"PANIC: OUT OF MEMORY (II)");
        syslog(LOG_ERR,"OUT OF MEMORY (II)");
        exit(250);
    }
    t->buffer = c;
    return t;
}

void deleteMyString(MyString *t ) {
    if ( t == NULL ) {
        return; // Avoid double free
    }
    if ( t->buffer != NULL ) {
        free(t->buffer);
    }
    free(t);
    return;
}

void newMyStringListe(MyStringListe *base, int maxlen) {
    if ( (base=malloc(sizeof(MyStringListe))) == NULL ) {
        syslog(LOG_ERR,"OUT OF MEMORY in MyStringListe");
        exit(253);
    }
    base->text = newMyString(maxlen);
    base->next = NULL;
    return;    
}

void deleteMyStringListe(MyStringListe *base) {
    MyStringListe *t,*u;
    t = base;
    while ( t != NULL ) {
        deleteMyString(t->text);
        u = t;
        t = t->next;
        free(u);
    }
}

#define DUMPLENGTH 8
void hexdump(MyString *in, char *introText, int doSyslog, int doPrintf) {
    int i,j;
    char string1[DUMPLENGTH*4 + 8];
    char string2[DUMPLENGTH + 8];
    char tempString[DUMPLENGTH*4 + 8];
    
    if ( doSyslog ) syslog(LOG_DEBUG,">>>>>>> %s", introText);
    if ( doPrintf ) printf(">>>>>>> %s\n", introText);
    for ( j=0; j < in->len; j += DUMPLENGTH ) {
        (void) strncpy(tempString, "", 2);
        (void)sprintf(string1,"%04X", j);
        (void) strncpy(string2, "", 2);
        for (i=0; i < DUMPLENGTH; i++ ){
            (void)strncpy(tempString, string1 ((DUMPLENGTH*4)+8));
            if ( (i+j) > in->len ) {
                (void)sprintf(string1, "%s   ", tempString);
            } else {
                (void)sprintf(string1, "%s %02X", tempString, in->buffer[j+i]);
                (void) strncpy(tempString, string2, DUMPLENGTH+8 );
                (void)sprintf(string2,"%s%c", tempString, in->buffer[j+i]);
            }
        }
        if ( doSyslog )
            syslog(LOG_DEBUG, "%s %s", string1, string2);
        if ( doPrintf ) {
            (void)printf("%s %s\n", string1, string2);
        }
    }
}

/************************************************************************/


void setup() {

}

void parseCommandline() {

}

void prepareSockets() {

}

void prepareSelect() {

}

int performSelect() {
    return -1;

}

void readDataFromSocket() {

}

void readCommandSocket() {

}

void readDataFromPrinter() {

}

void writeDataToPrinter() {

}

void writeDataToSocket() {

}

void writeCommandToPrinter() {

}

void writeCommandSocket() {

}

void acceptDataSocket() {

}

void acceptCommandSocket() {

}

int terminateRequest() {
    return 1;
}
