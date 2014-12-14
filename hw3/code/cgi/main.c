/*
 * Network Programming - HW3
 * Author:      Heron Yang
 * Director:    I-Chen Wu (Prof.)
 * Date:        Sun Dec 14 12:03:03 CST 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constant.h"
#include "variable.h"
#include "type.h"
#include "html_client.h"
#include "rbs.h"

#define MAX_SPLIT               128
#define REQUEST_CONTENT_LENGTH  128

Request req[MAX_REQUEST];

//
void req_init() {
    int i;
    for( i=0 ; i<MAX_REQUEST ; i++ ) {
        req[i].socket = 0;
    }
}

//
char **split(char **result, char *working, const char *src, const char *delim) {

    int i;

    strcpy(working, src);
    char *p=strtok(working, delim);
    for(i=0; p!=NULL && i < (MAX_SPLIT -1); i++, p=strtok(NULL, delim) ) {
        result[i]=p;
        result[i+1]=NULL;
    }

    return result;

}

void parse_param(const char *str) {

    int i;
    char *result[MAX_SPLIT] = {NULL};
    char working[256] = {0x0};
    char mydelim[] = "!@#$%^&*()_-";

    char *ind, *val;
    char ele;
    int num;

    split(result, working, str, mydelim);

    // save to global struct
    for( i=0 ; result[i]!=NULL ; i++) {

        ind = strtok(result[i], "=");
        if(ind == NULL) continue;
        val = strtok(NULL, "=");
        if(val == NULL) continue;

        if( sscanf(ind, "%c%d", &ele, &num) != 2 )   perror("scanf");
        num --;

        if(ele == 'h') {
            req[num].ip = malloc(REQUEST_CONTENT_LENGTH);
            strcpy(req[num].ip, val);
        } else if(ele == 'p') {
            req[num].port = malloc(REQUEST_CONTENT_LENGTH);
            strcpy(req[num].port, val);
        } else if(ele == 'f') {
            req[num].file = malloc(REQUEST_CONTENT_LENGTH);
            strcpy(req[num].file, val);
        }

    }

}

/* UI */
void html_init() {
    char *content = "<html> \
        <head> \
        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" /> \
        <title>Network Programming Homework 3</title> \
        </head> \
        <body style=\"font-family: 'Courier New', Courier, monospace;\">";
    char *table_html = "<table id=\"result_table\" width=\"800\" border=\"1\">\
                        <tr id=\"res_tr_head\"></tr>\
                        <tr id=\"res_tr_content\"></tr>\
                        </table>";
    printf("Content-Type: text/html\n\n");
    printf("%s%s", content, table_html);

}

void html_end() {
    char *content = "<style>\
        td {\
            font-size: small;\
            vertical-align: top;\
        }\
        </style>\
        </body> \
        </html>";
    printf("%s", content);
}

/* Main */
int main(void) {

    char *data;

    html_init();
    req_init();

    data = getenv("QUERY_STRING");
    if(data == NULL)
        printf("<p>Error! Error in passing data from form to script.</p>");
    else {
        parse_param(data);
        if(DEBUG)   print_req();
        serve_req();
        rbs();
    }

    html_end();

    return 0;
}
