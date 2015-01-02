#include <windows.h>
#include <string.h>
#include <list>
using namespace std;

#include "resource.h"

/* Macros */
#define SERVER_PORT				7799

#define BUF_LENGTH				10240
#define MAX_SPLIT				128
#define REQUEST_CONTENT_LENGTH	128
#define MAX_REQUEST				5
#define MAX_PATH_LENGTH			512
#define FILES_PATH_DIR			"C:\\Users\\heron\\Desktop\\NP_hw3\\Debug\\files\\"

#define WM_SOCKET_NOTIFY		(WM_USER + 1)
#define SERVER_RESPONSE			1234

#define TYPE_PLAIN_TEXT			0
#define TYPE_GET_REQUEST		1

/* Types */
typedef struct {
	char *ip;
	char *port;
	char *file;
	int	socket;
	FILE *fp;
} Request;

/* Functions */
BOOL CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
int EditPrintf (HWND, TCHAR *, ...);
void WriteToSock(SOCKET sock, char *s);
void write_content_at(SOCKET sock, int num, char *_content, int bold);
void serve_req(SOCKET sock);
char *wrap_html(char *s);

/* Global Variables */
list<SOCKET> Socks, ServerSocks;
Request req[MAX_REQUEST];
static HWND hwndEdit;
SOCKET CurrentSock;
int ServerCount = 0;


/* Tool Functions */
void StripString(char *s) {
	int i;
	for (i = 0; i < strlen(s); i++) {
		while (s[i] == 52 && i<strlen(s)) {
			s[i] = '\0';
			i++;
		}
	}
}

char **split(char **result, char *working, const char *src, const char *delim) {
	int i;
	strcpy(working, src);
	char *p = strtok(working, delim);
	for (i = 0; p != NULL && i < (MAX_SPLIT - 1); i++, p = strtok(NULL, delim)){
		result[i] = p;
		result[i + 1] = NULL;
	}
	return result;
}

void parse_param(const char *str) {

	int i;
	char *result[MAX_SPLIT] = { NULL };
	char working[256] = { 0x0 };
	char mydelim[] = "!@#$%^&*()_-";

	char *ind, *val;
	char ele;
	int num;

	split(result, working, str, mydelim);

	// save to global struct
	for (i = 0; result[i] != NULL; i++) {

		ind = strtok(result[i], "=");
		if (ind == NULL) continue;
		val = strtok(NULL, "=");
		if (val == NULL) continue;

		if (sscanf(ind, "%c%d", &ele, &num) != 2)   perror("scanf");
		num--;

		if (ele == 'h') {
			req[num].ip = (char *)malloc(BUF_LENGTH);
			strcpy(req[num].ip, val);
		}
		else if (ele == 'p') {
			req[num].port = (char *)malloc(BUF_LENGTH);
			strcpy(req[num].port, val);
		}
		else if (ele == 'f') {
			req[num].file = (char *)malloc(BUF_LENGTH);
			strcpy(req[num].file, val);
		}

	}

}

int contain_prompt(char *s) {
	int i;
	for (i = 0; i < strlen(s); i++) {
		if (s[i] == '%')	return 1;
	}
	return 0;
}

/* Requests */
void req_init() {
	int i;
	for (i = 0; i < MAX_REQUEST; i++) {
		req[i].socket = 0;
	}
}

void req_print(HWND hwndEdit) {
	int i;
	for (i = 0; i < MAX_REQUEST; i++) {
		if (req[i].ip == NULL) {
			EditPrintf(hwndEdit, TEXT("NULL, "));
		}
		else {
			EditPrintf(hwndEdit, TEXT("%s, "), req[i].ip);
		}
		if (req[i].port == NULL) {
			EditPrintf(hwndEdit, TEXT("NULL, "));
		}
		else {
			EditPrintf(hwndEdit, TEXT("%s, "), req[i].port);
		}
		if (req[i].file == NULL) {
			EditPrintf(hwndEdit, TEXT("NULL, "));
		}
		else {
			EditPrintf(hwndEdit, TEXT("%s, "), req[i].file);
		}
		EditPrintf(hwndEdit, TEXT("\r\n"));
	}
}

/* RBS */
void write_command_init(int ind, HWND hwndEdit) {

	char filepath[MAX_PATH_LENGTH] = FILES_PATH_DIR;
	strcat(filepath, req[ind].file);
	if ((req[ind].fp = fopen(filepath, "r")) == NULL){
		EditPrintf(hwndEdit, TEXT("fopen error: %d\r\n"), errno);
	}

}

void write_command_close(int ind) {
	fclose(req[ind].fp);
}

void write_command_next(SOCKET sock, int ind) {

	int n, r;
	char buf[BUF_LENGTH];
	if (!fgets(buf, BUF_LENGTH, req[ind].fp)) {
		OutputDebugString("fgets error");
		return;
	}

	// write to html client
	write_content_at(sock, ind, wrap_html(buf), 1);
	if (buf[0] == '\n')	return;
	
	// write to server
	EditPrintf(hwndEdit, TEXT("=== write_command_next send: Start===\r\n%s\r\n=== End ===\r\n"), buf);
	r = send(req[ind].socket, buf, strlen(buf), 0);
	if (r == SOCKET_ERROR) {
		OutputDebugString("send error");
		closesocket(req[ind].socket);
		WSACleanup();
		return;
	}

}

int bash_new(HWND hwndEdit, HWND hwnd, int ind) {

	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (r != NO_ERROR) {
		EditPrintf(hwndEdit, TEXT("WSAStartup function failed with error: %d\r\n"), r);
		return 1;
	}
	//----------------------
	// Create a SOCKET for connecting to server
	SOCKET csock;
	csock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (csock == INVALID_SOCKET) {
		EditPrintf(hwndEdit, TEXT("socket function failed with error: %ld\r\n"), WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	struct hostent *hostname;
	hostname = gethostbyname(req[ind].ip);
	if (hostname == NULL) {
		EditPrintf(hwndEdit, TEXT("get hostname failed with error: %ld\r\n"), WSAGetLastError());
		return 1;
	}

	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	memcpy(&clientService.sin_addr, hostname->h_addr_list[0], hostname->h_length);
	clientService.sin_port = htons(atoi(req[ind].port));

	//----------------------
	// Connect to server.
	r = connect(csock, (SOCKADDR *)& clientService, sizeof (clientService));
	if (r == SOCKET_ERROR) {
		EditPrintf(hwndEdit, TEXT("connect function failed with error: %ld\r\n"), WSAGetLastError());
		r = closesocket(csock);
		if (r == SOCKET_ERROR) {
			EditPrintf(hwndEdit, TEXT("closesocket function failed with error: %ld\r\n"), WSAGetLastError());
		}
		WSACleanup();
		return 1;
	}

	EditPrintf(hwndEdit, TEXT("connected to %s\r\n"), req[ind].ip);
	
	int err = WSAAsyncSelect(csock, hwnd, SERVER_RESPONSE, FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE);

	if (err == SOCKET_ERROR) {
		EditPrintf(hwndEdit, TEXT("=== Error: select error ===\r\n"));
		closesocket(csock);
		WSACleanup();
		return 1;
	}

	req[ind].socket = csock;

	return 0;
}

void rbs(HWND hwndEdit, HWND hwnd) {
	
	int i;
	for (i = 0; i < MAX_REQUEST; i++) {
		Request r = req[i];
		if (!(r.ip && r.port && r.file))	continue;
		write_command_init(i, hwndEdit);
		bash_new(hwndEdit, hwnd, i);
		ServerCount++;
	}

}

/* UI */
void html_init(HWND hwndEdit, SOCKET sock) {
	char *header = "HTTP/1.1 200 OK\r\n\
				   Cache-Control: public\r\n\
				   Content-Type: text/html\r\n\r\n";
	char *content = "<html> \
        <head> \
        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" /> \
        <title>Network Programming Homework 3</title> \
        </head> \
        <body style=\"font-family: 'Courier New', Courier, monospace;\">\
        <style>\
        td {\
            font-size: small;\
            vertical-align: top;\
        }\
        </style>";
    char *table_html = "<table id=\"result_table\" width=\"800\" border=\"1\">\
                        <tr id=\"res_tr_head\"></tr>\
                        <tr id=\"res_tr_content\"></tr>\
                        </table>";

	int r;

	WriteToSock(sock, header);
	WriteToSock(sock, content);
	WriteToSock(sock, table_html);

}

void html_end(HWND hwndEdit, int sock) {
    char *content = "</body></html>";
	int r;
	r = send(sock, content, strlen(content), 0);
	if (r == SOCKET_ERROR) {
		EditPrintf(hwndEdit, TEXT("=== Error: socket send ===\r\n"));
		WSACleanup();
		return;
	}
}

/* Handlers */
int HandleRecieveBuf(HWND hwndEdit, char *buf) {
	char seps[] = " ?";
	char *token;
	char t;
	//
	if (buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T') {
		// is for web GET request
		strtok(buf, seps);
		strtok(NULL, seps);
		token = strtok(NULL, seps);	// get request path
		EditPrintf(hwndEdit, TEXT("token: %s\r\n"), token);
		
		t = token[0];
		if (t != 'h' && t != 'p' && t != 'f') return TYPE_PLAIN_TEXT;
		
		parse_param(token);
		req_print(hwndEdit);
		return TYPE_GET_REQUEST;
	}
	return TYPE_PLAIN_TEXT;
}

void WriteToSock(SOCKET sock, char *s) {
	
	int r;

	r = send(sock, s, strlen(s), 0);
	if (r == SOCKET_ERROR) {
		OutputDebugString("=== Error: socket send ===\r\n");
		WSACleanup();
		return;
	}

}

void HandleClient(SOCKET ssock, HWND hwndEdit, HWND hwnd) {
	
	// layout init
	html_init(hwndEdit, ssock);
	
	// layout update corresponding to requests
	serve_req(ssock);

	// save socket as currrent html layout socket
	CurrentSock = ssock;
	
	// start server connections (read/write happens in callback handler)
	rbs(hwndEdit, hwnd);
	
}

void HandleServerResponse(SOCKET sock, int ind, char *buf) {
	EditPrintf(hwndEdit, TEXT("=== Handle Server Response: Start===\r\n%s\r\n=== End ===\r\n"), buf);
	write_content_at(sock, ind, wrap_html(buf), 0);
	if (contain_prompt(buf)) {
		// should send
		write_command_next(sock, ind);
	}
}

// HTML Client
char *replace_str(const char *str, const char *old, const char *new_str) {

	char *ret, *r;
	const char *p, *q;
	int oldlen = strlen(old);
	int count, retlen, newlen = strlen(new_str);
	int samesize = (oldlen == newlen), l;

	if (!samesize) {
		for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
			count++;
		retlen = p - str + strlen(p) + count * (newlen - oldlen);
	}
	else
		retlen = strlen(str);

	if ((ret = (char *)malloc(retlen + 1)) == NULL)
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
		memcpy(r, new_str, newlen);
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
	r = replace_str(r, "&", "&amp;");
	r = replace_str(r, "\"", "&quot;");
	r = replace_str(r, "<", "&lt;");
	r = replace_str(r, ">", "&gt;");
	r = replace_str(r, "\r\n", "\n");
	r = replace_str(r, "\n", "<br />");
	return r;
}

void write_head_at(SOCKET sock, int num, char *_content) {
	char content[BUF_LENGTH];
	sprintf(content, "<script>document.all['res_tr_head'].innerHTML += \"<td>%s</td>\";</script>", _content);
	WriteToSock(sock, content);
}

void write_content_at(SOCKET sock, int num, char *_content, int bold) {
	char content[BUF_LENGTH];
	if (bold)
		sprintf(content, "<script>document.all('c-%d').innerHTML += \"<b>%s</b>\";</script>", num, _content);
	else
		sprintf(content, "<script>document.all('c-%d').innerHTML += \"%s\";</script>", num, _content);
	WriteToSock(sock, content);
}

void write_content_init(SOCKET sock, int num) {
	char content[BUF_LENGTH];
	sprintf(content, "<script>\
		    document.all('res_tr_content').innerHTML += \"\
						<td id='c-%d'></td>\";</script>", num);
	WriteToSock(sock, content);
}

void serve_req_at(SOCKET sock, int num) {

	write_head_at(sock, num, req[num].ip);
	write_content_init(sock, num);

}

void serve_req(SOCKET sock) {
	int i;
	for (i = 0; i<MAX_REQUEST; i++) {
		Request r = req[i];
		if (!(r.ip && r.port && r.file))   continue;
		serve_req_at(sock, i);
	}
}



/* ================= */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow) {
	return DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, MainDlgProc);
}

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

	WSADATA wsaData;

	static SOCKET msock, ssock, csock;
	static struct sockaddr_in sa;

	char recvbuf[BUF_LENGTH];

	int err;
	int r, i;

	list<SOCKET>::iterator iter;

	switch(Message) 
	{
		case WM_INITDIALOG:
			hwndEdit = GetDlgItem(hwnd, IDC_RESULT);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_LISTEN:

					WSAStartup(MAKEWORD(2, 0), &wsaData);

					//create master socket
					msock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

					if( msock == INVALID_SOCKET ) {
						EditPrintf(hwndEdit, TEXT("=== Error: create socket error ===\r\n"));
						WSACleanup();
						return TRUE;
					}

					err = WSAAsyncSelect(msock, hwnd, WM_SOCKET_NOTIFY, FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE);

					if ( err == SOCKET_ERROR ) {
						EditPrintf(hwndEdit, TEXT("=== Error: select error ===\r\n"));
						closesocket(msock);
						WSACleanup();
						return TRUE;
					}

					//fill the address info about server
					sa.sin_family		= AF_INET;
					sa.sin_port			= htons(SERVER_PORT);
					sa.sin_addr.s_addr	= INADDR_ANY;

					//bind socket
					err = bind(msock, (LPSOCKADDR)&sa, sizeof(struct sockaddr));

					if( err == SOCKET_ERROR ) {
						EditPrintf(hwndEdit, TEXT("=== Error: binding error ===\r\n"));
						WSACleanup();
						return FALSE;
					}

					err = listen(msock, 2);
		
					if( err == SOCKET_ERROR ) {
						EditPrintf(hwndEdit, TEXT("=== Error: listen error ===\r\n"));
						WSACleanup();
						return FALSE;
					}
					else {
						EditPrintf(hwndEdit, TEXT("=== Server START ===\r\n"));
					}

					break;
				case ID_EXIT:
					EndDialog(hwnd, 0);
					break;
			};
			break;

		case WM_CLOSE:
			EndDialog(hwnd, 0);
			break;

		case WM_SOCKET_NOTIFY:
			switch (WSAGETSELECTEVENT(lParam)) {
				case FD_ACCEPT:
					ssock = accept(msock, NULL, NULL);
					// NOTE: push accepted sock into our list
					Socks.push_back(ssock);
					EditPrintf(hwndEdit, TEXT("=== Accept one new client(%d), List size:%d ===\r\n"), ssock, Socks.size());
					break;
				case FD_READ:
					for (iter = Socks.begin(); iter != Socks.end(); iter++) {
						csock = *iter;
						do {
							memset(recvbuf, 0, sizeof(recvbuf));
							r = recv(csock, recvbuf, BUF_LENGTH, 0);
							if (r > 0) {
								EditPrintf(hwndEdit, TEXT("=== recv: START ===\r\n%s\r\n== recv: END ==\r\n"), recvbuf);
								if (HandleRecieveBuf(hwndEdit, recvbuf) == TYPE_GET_REQUEST) {
									HandleClient(csock, hwndEdit, hwnd);
									break;
								}
							}
							else if (r == 0) {
								closesocket(csock);
								EditPrintf(hwndEdit, TEXT("connection closed\r\n"));
							}
							else {
								EditPrintf(hwndEdit, TEXT("recv failed: %d\r\n"), WSAGetLastError());
							}
						} while (r > 0);
					}
					
					break;
				case FD_WRITE:
					break;
				case FD_CLOSE:
					break;
			};
			break;

		case SERVER_RESPONSE:
			EditPrintf(hwndEdit, TEXT("get server response: %x\r\n"), WSAGETSELECTEVENT(lParam));
			switch (WSAGETSELECTEVENT(lParam)) {
				case FD_ACCEPT:
					EditPrintf(hwndEdit, TEXT("=== [Server] Accept one new client(%d), List size:%d ===\r\n"), ssock, ServerSocks.size());
					break;
				case FD_READ:
					for (i = 0; i<MAX_REQUEST; i++) {
						csock = req[i].socket;
						do {
							memset(recvbuf, 0, sizeof(recvbuf));
							r = recv(csock, recvbuf, BUF_LENGTH, 0);
							if (r > 0) {
								EditPrintf(hwndEdit, TEXT("=== [server] recv: START ===\r\n%s\r\n== recv: END ==\r\n"), recvbuf);
								HandleServerResponse(CurrentSock, i, recvbuf);
							}
							else if (r == 0) {
								closesocket(csock);
								EditPrintf(hwndEdit, TEXT("[server] connection closed\r\n"));
							}
							else {
								EditPrintf(hwndEdit, TEXT("[server] recv failed: %d\r\n"), WSAGetLastError());
							}
						} while (r > 0);
					}

					// if end
					// layout ending
					// html_end(hwnd, ssock);

					break;
				case FD_WRITE:
					EditPrintf(hwndEdit, TEXT("=== [Server] fd write ===\r\n"));
					break;
				case FD_CLOSE:
					EditPrintf(hwndEdit, TEXT("=== [Server] fd close ===\r\n"));
					ServerCount--;
					if (ServerCount == 0) {
						closesocket(CurrentSock);
						for (i = 0; i < MAX_REQUEST; i++) {
							if (req[i].ip)		req[i].ip = NULL;
							if (req[i].port)	req[i].ip = NULL;
							if (req[i].file)	req[i].file = NULL;
							if (req[i].fp)		fclose(req[i].fp);
						}
					}
					break;
			};
			break;
		
		default:
			return FALSE;


	};

	return TRUE;
}

int EditPrintf (HWND hwndEdit, TCHAR * szFormat, ...) {

	TCHAR   szBuffer [BUF_LENGTH] ;
    va_list pArgList ;

    va_start (pArgList, szFormat) ;
    wvsprintf (szBuffer, szFormat, pArgList) ;
    va_end (pArgList) ;

    SendMessage (hwndEdit, EM_SETSEL, (WPARAM) -1, (LPARAM) -1) ;
    SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) szBuffer) ;
    SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;

	return SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0); 

}