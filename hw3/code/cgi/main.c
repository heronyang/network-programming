#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_REQUEST             5

#define MAX_SPLIT               128
#define REQUEST_CONTENT_LENGTH  128

typedef struct {
    char *ip;
    char *port;
    char *file;
} Request;

Request req[MAX_REQUEST];

void print_req() {  // debug
    int i;
    for( i=0 ; i<MAX_REQUEST ; i++ ) {
        printf("<p>");
        if(req[i].ip == NULL)   printf("NULL, ");
        else                    printf("%s, ", req[i].ip);
        if(req[i].port == NULL) printf("NULL, ");
        else                    printf("%s, ", req[i].port);
        if(req[i].file == NULL) printf("NULL");
        else                    printf("%s", req[i].file);

        printf("</p>");
    }
}

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
        val = strtok(NULL, "=");
        if(val == NULL) continue;

        printf("<p><i>%s, %s (len = %d)</i></p>", ind, val, strlen(val));
        
        if( sscanf(ind, "%c%d", &ele, &num) != 2 )   perror("scanf");
        num --;
        printf("<p>_%c_, _%d_</p>", ele, num);

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

int main(void) {

    char *data;
    printf("Content-Type: text/html\n\n");
    data = getenv("QUERY_STRING");

    if(data == NULL)
        printf("<p>Error! Error in passing data from form to script.</p>");
    else {
        printf("<p>param: <b>%s</b></p>", data);
        parse_param(data);
        print_req();
    }

    return 0;
}
