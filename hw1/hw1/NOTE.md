##Test
- ./bin/removetag test.html |3 ./bin/removetag test.html | ./bin/number |1 ./bin/number

##Exec Family
- execl:    run file by having absolute file path
- execlp:   run file by search in $PATH
- execv:    same as execl, but only two arguments (path, char* argv[])
- execve:   3 arguments, the 3rd one is for adding new environment variables

##Variables
- BACKLOG: the maximum length to which the queue of pending connections for sockfd may grow

##Reference
- [INADDR_ANY](http://stackoverflow.com/questions/13979150/why-is-sin-addr-inside-the-structure-in-addr)
- [INADDR_ANY](http://stackoverflow.com/questions/16508685/understanding-inaddr-any-for-socket-programming-c): ```# define INADDR_ANY ((unsigned long int) 0x00000000)``` (localhost)
- [SO_REUSEPORT](http://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t): stop server program complaining the socket is used
