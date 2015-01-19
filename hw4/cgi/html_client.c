#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constant.h"
#include "variable.h"
#include "type.h"
Request req[MAX_REQUEST];

// Tool
char *replace_str(const char *str, const char *old, const char *new) {

    char *ret, *r;
    const char *p, *q;
    int oldlen = strlen(old);
    int count, retlen, newlen = strlen(new);
    int samesize = (oldlen == newlen), l;

    if (!samesize) {
        for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
            count++;
        retlen = p - str + strlen(p) + count * (newlen - oldlen);
    } else
        retlen = strlen(str);

    if ((ret = malloc(retlen + 1)) == NULL)
        return NULL;

    r = ret, p = str;
    while (1) {
        if (!samesize && !count--)
            break;
        if ((q = strstr(p, old)) == NULL)
            break;
        l = q - p;
        memcpy(r, p, l);
        r += l;
        memcpy(r, new, newlen);
        r += newlen;
        p = q + oldlen;
    }
    strcpy(r, p);

    return ret;

}

char *wrap_html(char *s) {
    fprintf(stderr, "wrapping:\n%s\n", s);
    char *r;
    r = s;
    r = replace_str(r, "&",     "&amp;");
    r = replace_str(r, "\"",    "&quot;");
    r = replace_str(r, "<",     "&lt;");
    r = replace_str(r, ">",     "&gt;");
    r = replace_str(r, "\r\n",  "\n");
    r = replace_str(r, "\n",    "<br />");
    return r;
}

//
void print_req() {  // debug
    int i;
    for( i=0 ; i<MAX_REQUEST ; i++ ) {

        printf("<p>");

        if(req[i].ip == NULL)   printf("NULL, ");
        else                    printf("%s, ", req[i].ip);
        if(req[i].port == NULL) printf("NULL, ");
        else                    printf("%s, ", req[i].port);
        if(req[i].file == NULL) printf("NULL, ");
        else                    printf("%s, ", req[i].file);
        printf("%d", req[i].socket);

        printf("</p>");

    }
}

void write_head_at(int num, char *content) {
    printf("<script>document.all['res_tr_head'].innerHTML += \"<td>%s</td>\";</script>", content);
}

void write_content_at(int num, char *content, int bold) {
    if(bold)
        printf("<script>document.all('c-%d').innerHTML += \"<b>%s</b>\";</script>", num, content);
    else
        printf("<script>document.all('c-%d').innerHTML += \"%s\";</script>", num, content);
    fflush(stdout);
    fflush(stdout);
    fflush(stdout);
}

void write_content_init(int num) {
    printf("<script>\
            document.all('res_tr_content').innerHTML += \"\
                <td id='c-%d'></td>\";\
            </script>", num);
    fflush(stdout);
    fflush(stdout);
    fflush(stdout);
}

void serve_req_at(int num) {

    write_head_at(num, req[num].ip);
    write_content_init(num);

}

void serve_req() {
    int i;
    for( i=0 ; i<MAX_REQUEST ; i++ ) {
        Request r = req[i];
        if( !(r.ip && r.file) )   continue;
        serve_req_at(i);
    }
}
