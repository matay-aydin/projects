#ifndef __REQUEST_H__

void requestHandle(int fd, char *buf);
int requestParseURI(char *uri, char *filename, char *cgiargs);
void requestReadhdrs(rio_t *rp);

#endif
