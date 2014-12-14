#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10240

char *replace_string(const char *str, const char *old, const char *new) {

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

int main() {

    char input[] = "hello\nhow are you\n?";
    printf("result = %s\n", replace_string(input, "\n", "<br />"));

    return 0;
}
