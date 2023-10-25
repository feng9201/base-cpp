#include "httpUrlSplit.h"
#include <iostream>
#include <string>


namespace download {
	using namespace std;

    #define	HTTP_PREFIX	"http://"
    #define	HTTPS_PREFIX	"https://"

    bool httpUrlSplit::get_addr(const char* url, char* domain,char* url_path, size_t size,
        unsigned short* pport)
    {
        const char* ptr;
        unsigned short default_port;

        if (!strncmp(url, HTTP_PREFIX, sizeof(HTTP_PREFIX) - 1)) {
            ptr = url + sizeof(HTTP_PREFIX) - 1;
            default_port = 0;
        }
        else if (!strncmp(url, HTTPS_PREFIX, sizeof(HTTPS_PREFIX) - 1)) {
            ptr = url + sizeof(HTTPS_PREFIX) - 1;
            default_port = 0;
        }
        else {
            return false;
        }

        if (*ptr == 0) {
            return false;
        }

        char buf[256];
        strncpy(buf, ptr, sizeof(buf));

        char* slash = strchr(buf, '/');
        if (slash) {
            strncpy(url_path, slash, size);
            *slash = 0;
        }

        unsigned short port;

        char* col = strchr(buf, ':');
        if (col == NULL) {
            port = default_port;
        }
        else {
            *col++ = 0;
            port = (unsigned short)atoi(col);
            if (port == 0 || port == 65535) {
                port = default_port;
            }
        }

        if (pport) {
            *pport = port;
        }
        strncpy(domain, buf, size);
        return true;
    }

    bool httpUrlSplit::get_addr(const char* url, char* addr, size_t size)
    {
        char  buf[256];
        unsigned short port;

        /*if (!get_addr(url, buf, sizeof(buf), &port)) {
            return false;
        }
        snprintf(addr, size, "%s:%d", buf, port);*/
        return true;
    }
}