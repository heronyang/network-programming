#ifndef HTML_CLIENT_H
#define HTML_CLIENT_H

char *wrap_html(char *s);

void print_req();
void serve_req();
void write_content_at(int num, char *content);

#endif
