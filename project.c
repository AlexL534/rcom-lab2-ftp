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
#define IP_MAX_LENGTH 32

typedef struct {
    char user[MAX_LENGTH];
    char password[MAX_LENGTH];
    char host[MAX_LENGTH];
    char path[MAX_PATH_LENGTH];
    char ip[IP_MAX_LENGTH];
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
        strcpy(parsed_url->password, "password");
    } else { // Format: ftp://<user>:<password>@<host>/<path>
        sscanf(url, "ftp://%255[^:]:%255[^@]@%255[^/]/%1023s", parsed_url->user, parsed_url->password, parsed_url->host, parsed_url->path);
    }

    // Resolve hostname to IP
    struct hostent *he = gethostbyname(parsed_url->host);
    if (he == NULL) {
        printf("Could not resolve hostname %s\n", parsed_url->host);
        return -1;
    }
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    strcpy(parsed_url->ip, inet_ntoa(*addr_list[0]));  // Store the first IP address

    return 0;
}

int openSocket(char* ip, int port) {

    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    /*connect to the server*/
    if (connect(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    return sockfd;
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
        printf("IP: %s\n", parsed_url.ip);
    } else {
        printf("Failed to parse URL\n");
    }

    int socket = openSocket(parsed_url.ip, SERVER_PORT);
    printf("Connected to server at %s\n", parsed_url.ip);

    close(sockfd);
    return 0;
}
