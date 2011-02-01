#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>

#define SERVER_NAME "Simple Server/01 (Linux)"
#define WEB_ROOT "/home/john/www"
#define ERROR_400 "400.html"
#define ERROR_404 "404.html"
#define ERROR_501 "501.html"
#define ERROR_505 "505.html"

typedef struct content_t{
    int size;
    char *str;
} content_t;

typedef struct http_request {
    short status;
    char method[8];
    char uri[2048];
    char http_version[4];
} http_request;

struct types {
    char extension[10];
    char mime[40];
};

struct types t[] = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"css", "text/css"},
    {"gif", "image/gif"},
    {"png", "image/x-png"},
    {"jpg", "image/jpeg"},
    {"jpe", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"mp3", "audio/mpeg"}
};



char DEBUG = 0;
char *SERVER_PORT = "8080";

void print_d(const char *format, ...) {
    if(DEBUG == 0) {
        return;
    } else {
        va_list args;
        va_start(args, format);
        printf("DEBUG: ");
        vprintf(format, args);
        va_end(args);
    }
}



