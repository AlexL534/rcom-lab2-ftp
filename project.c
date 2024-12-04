#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <regex.h>

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

#define MAX_LENGTH 256
#define MAX_PATH_LENGTH 1024

typedef struct {
    char user[MAX_LENGTH];
    char password[MAX_LENGTH];
    char host[MAX_LENGTH];
    char path[MAX_PATH_LENGTH];
} URL;

int parse_url(char* url, URL *parsed_url) {
    regex_t regex;

    regcomp(&regex, "^ftp://", 0);
    if (regexec(&regex, url, 0, NULL, 0)) {
        printf("Invalid URL format: must start with ftp://\n");
        return -1;
    }

    regcomp(&regex, "@", 0);
    if (regexec(&regex, url, 0, NULL, 0) != 0) { // Format: ftp://<host>/<path>
        sscanf(url, "ftp://%255[^/]/%1023s", parsed_url->host, parsed_url->path);
        strcpy(parsed_url->user, "anonymous");
        strcpy(parsed_url->password, "anonymous");
    } else { // Format: ftp://<user>:<password>@<host>/<path>
        sscanf(url, "ftp://%255[^:]:%255[^@]@%255[^/]/%1023s", parsed_url->user, parsed_url->password, parsed_url->host, parsed_url->path);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }

    URL parsed_url;
    int result = parse_url(argv[1], &parsed_url);

    if (result == 0) {
        printf("User: %s\n", parsed_url.user);
        printf("Password: %s\n", parsed_url.password);
        printf("Host: %s\n", parsed_url.host);
        printf("Path: %s\n", parsed_url.path);
    } else {
        printf("Failed to parse URL\n");
    }

    return 0;
}