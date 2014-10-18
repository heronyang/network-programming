**Variables
- BACKLOG: the maximum length to which the queue of pending connections for sockfd may grow

**Reference
- [INADDR_ANY](http://stackoverflow.com/questions/13979150/why-is-sin-addr-inside-the-structure-in-addr)
- [INADDR_ANY](http://stackoverflow.com/questions/16508685/understanding-inaddr-any-for-socket-programming-c): ```# define INADDR_ANY ((unsigned long int) 0x00000000)``` (localhost)
- [SO_REUSEPORT](http://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t): stop server program complaining the socket is used
