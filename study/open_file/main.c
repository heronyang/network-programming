#include <stdio.h>
#include <fcntl.h>

#define SIZE 1024

int main() {

    int fd = open("test.txt", O_RDONLY), n;
    char buf[SIZE];

    n = read(fd, buf, SIZE);
    if(n < 0) {
        printf("error\n");
    }
    printf("buf: %s///", buf);

    return 0;

}
