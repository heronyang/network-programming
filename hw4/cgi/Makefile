CC=gcc
all:
	$(CC) -Wall rbs.c main.c html_client.c -o main
	cp main ~/public_html/cgi-bin/proxy.cgi
	cp main ../www/cgi-bin/proxy.cgi
