//http://www.w3.org/Protocols/rfc2616/rfc2616.html
#include "network_utils.h"
#include "server.h"

char *get_type(char *ext)
{
    int i;
    for(i = 0; t[i].extension != NULL; i++) {
        if(strcmp(t[i].extension, ext) == 0) {
            return t[i].mime;
        }
    }
    return strdup("text/plain");
}


content_t    *read_file(char *f);
content_t    *error(int status);
content_t    *build_response(http_request *r);
http_request *parse_request(char *request);

content_t *read_file(char *f)
/* returns NULL on failure */
{
    content_t *content;
    FILE *file;
    size_t result;
    content = malloc(sizeof(content_t));
    file = fopen(f, "rb");
    if(file == NULL) {
        print_d("Error reading file: %s.\n", f);
        return NULL;
    }

    if(fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error repositioning stream position indicator to the end.\n");
        return NULL;
    }

    if((content->size = ftell(file)) < 0) {
        fprintf(stderr, "Error getting current position in stream.\n");
        return NULL;
    }

    if(fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Error repositioning stream position indicator to the beginning.\n");
        return NULL;
    }

    content->str = (char *)malloc(content->size+1);
    if(content->str == NULL) {
        fprintf(stderr, "Error allocating memory block.\n");
        return NULL;
    }

    result = fread(content->str, 1, content->size, file);
    if(result < 0) {
        fprintf(stderr, "Error reading data from file stream.\n");
        return NULL;
    }

    fclose(file);
    return content;
}

content_t *error(int status) {
    content_t *content;
    content = malloc(sizeof(content_t));
    content->str = malloc(72);
    content->size = 72;
    sprintf(content->str, "<html>\n<body>\n<h1>The server returned a %d error.</h1>\n</body>\n</html>", status);
    return content;
}


content_t *build_response(http_request *r)
{
    print_d("building response\n");
    time_t rawtime;
    content_t *content, *resp;
    struct tm *timeinfo;
    char date[80], temp_buffer[4000];

    resp    = malloc(sizeof(content_t));
    content = malloc(sizeof(content_t));

    if(r->status == 0) {
        char str[3000] = {0};
        strcat(str, WEB_ROOT);
        strcat(str, r->uri);
        if((content = read_file(str)) != NULL) {
            r->status = 200;
        } else {
            r->status = 404;
        }
    }
    print_d("status code '%d'\n", r->status);
    char *status;
    char ext[10] = {0};
    char *mime = strdup("text/plain");
    switch(r->status) {
        case 200:
            sscanf(r->uri, "%*[^.].%s", ext);
            mime = get_type(ext);
            status = strdup("200 OK");
            break;
        case 400:
            if((content = read_file(ERROR_400))) {
                content = error(r->status);
            }
            status = strdup("404 Bad Request");
            break;
        case 404:
            if((content = read_file(ERROR_404)) == NULL) {
                content = error(r->status);
            }
            status = strdup("404 Not Found");
            break;
        case 501:
            if((content = read_file(ERROR_501)) == NULL) {
                content = error(r->status);
            }
            status = strdup("501 Not Implemented");
            break;
        case 505:
            if((content = read_file(ERROR_505)) == NULL) {
                content = error(r->status);
            }
            status = strdup("505 Version Not Supported");
            break;
        default:
            if((content = read_file(ERROR_501)) == NULL) {
                content = error(100);
            }
            status = strdup("100 Unknown Error");
    }

    print_d("the status is '%s'\n", status);
    /* Date */
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (date, sizeof date, "%A, %d-%b-%y %H:%M:%S %Z", timeinfo);

    sprintf(temp_buffer, "HTTP/1.1 %s\r\n"
                         "Date: %s\r\n"
                         "Server: %s\r\n"
                         "Content-Type: %s\r\n"
                         //"Transfer-Encoding: chunked\r\n"
                         "Content-Length: %d\r\n"
                         "Accept-Charset: ISO-8859-1\r\n"
                         "Connection: close\r\n\r\n",
                         status, date, SERVER_NAME, mime, content->size);

    resp->str  = malloc(strlen(temp_buffer) + content->size + 1);

    if(resp->str == NULL) { printf("Failed to allocate memory.\n"); exit(1); }

    resp->size = strlen(temp_buffer) + content->size + 1; 
    memcpy(resp->str, temp_buffer, strlen(temp_buffer));
    memcpy(resp->str+strlen(temp_buffer), content->str, content->size);
    return resp;
}

http_request *parse_request(char *request)
{
    print_d("parsing request\n");
    http_request *r;
    r = malloc(sizeof(http_request));

    if(sscanf(request, "%s %s HTTP/%s\n", r->method, r->uri, r->http_version) == 0) {
        print_d("bad request\n");
        r->status = 400;
        return r;
    }

    if(strcmp("GET", r->method)) {
        print_d("method '%s' not supported\n", r->method);
        r->status = 501;
        return r;
    }

    if(strcmp("1.1", r->http_version)) {
        print_d("http version '%s' not supported\n", r->http_version);
        r->status = 505;
        return r;
    }

    r->status = 0;
    print_d("parse_request returned a status of '%d'\n", r->status);
    return r;
}


int main(int argc, char **argv) {
    int listen_fd, max, c;   
    fd_set read_fds;

    while((c = getopt(argc, argv, "p:v::")) != -1) {
        switch(c) {
            case 'v':
                DEBUG = 1;
                break;
            case 'p':
                SERVER_PORT = optarg;
                break;
        }
    }



    if((listen_fd = create_tcp_passive_socket(SERVER_PORT)) == -1) {
        return 1;
    }

    max = listen_fd;

    while(1) {
        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);
        FD_SET(fileno(stdin), &read_fds);

        if (select(max+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            return 0;
        }

        if (FD_ISSET(listen_fd, &read_fds)) {
            int tcp_fd;
            int nbytes;
            char recv_buffer[500];                      

            if ((tcp_fd = accept(listen_fd, NULL, NULL)) < 0 ) {
                if (errno == EINTR) {
                    continue;
                }
                else {
                    printf("Error accepting new connections.\n");
                }
            }

            if ((nbytes = recv(tcp_fd, recv_buffer, sizeof recv_buffer, 0)) <= 0) {
                if (nbytes == 0) {
                    printf("selectserver: socket %d hung up\n", tcp_fd);
                }
                else {
                    perror("recv");
                }
                printf("No data was received. Closing connection...\n");
                close(tcp_fd); // bye!
            }

            else {
                print_d("=========== RECEIVED ===========\n%s\n", recv_buffer);
                http_request *r;
                content_t *response;

                r = parse_request(recv_buffer);
                //if(r == NULL) {
                //    print_d("Invalid request\n");
                //    close(tcp_fd);
                //    continue;
                //}

                response = build_response(r);
                print_d("\n============= SENT ==============\n");
                if(DEBUG != 0) {
                    printf("DEBUG: \n");
                    print_packet(response->str, response->size); 
                }
                /* 
                 * There might be an issue when response->str is too big
                 * causing send to not send the entire packet.
                 */  
                int size1 = response->size;
                int i;
                for(i = 0; size1 > 0; i++) {
                    char *c = malloc(1024+1);
                    memset(c, 0, 1024+1);
                    memcpy(c, response->str+(1024*i), 1024);
                    size1 -= 1024;
                    send(tcp_fd, c, 1024, 0);
                }
 
                //send(tcp_fd, response->str, response->size, 0);
                send(tcp_fd, 0, 1, 0);
                memset(recv_buffer, 0, sizeof recv_buffer);
                close(tcp_fd);
            }
        }


        if (FD_ISSET(fileno(stdin), &read_fds)) {
            char buffer[100]; 
            if (fgets(buffer, sizeof buffer, stdin) == NULL) {
                break;
            }
            printf("\n");
        }


   } 

    close(listen_fd);

    return 0;
}
