#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <regex.h>

#define SERVER_PORT 21 //FTP standard control port
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

// Function to send and receive FTP commands
void send_ftp_command(int sockfd, const char *command) {
    char buffer[MAX_LENGTH];
    send(sockfd, command, strlen(command), 0);
    printf("Sent: %s\n", command);

    // Receive response from server
    int n = recv(sockfd, buffer, MAX_LENGTH-1, 0);
    if (n < 0) {
        perror("recv()");
        exit(-1);
    }
    buffer[n] = '\0';
    printf("Received: %s\n", buffer);
}

void receive_server_greeting(int sockfd) {
    char buffer[MAX_LENGTH];
    int n = recv(sockfd, buffer, MAX_LENGTH - 1, 0);
    if (n < 0) {
        perror("recv()");
        exit(-1);
    }
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);
}

void send_user_command(int sockfd, const char *user) {
    char command[MAX_LENGTH];
    snprintf(command, sizeof(command), "USER %s\r\n", user);
    send_ftp_command(sockfd, command);
}

void send_pass_command(int sockfd, const char *password) {
    char command[MAX_LENGTH];
    snprintf(command, sizeof(command), "PASS %s\r\n", password);
    send_ftp_command(sockfd, command);
}

void passive_mode(int sockfd, char *data_ip, int *data_port) {
    char response[MAX_LENGTH];
    char ip[IP_MAX_LENGTH];
    int h1, h2, h3, h4, p1, p2;

    send_ftp_command(sockfd, "PASV\r\n");

    int n = recv(sockfd, buffer, MAX_LENGTH - 1, 0);
    if (n < 0) {
        perror("recv()");
        exit(-1);
    }
    response[n] = '\0';
    printf("PASV Response: %s\n", response);

    if (sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", 
               &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        printf("Error: Could not parse PASV response\n");
        exit(-1);
    }

    // Construct the IP address
    snprintf(data_ip, IP_MAX_LENGTH, "%d.%d.%d.%d", h1, h2, h3, h4);

    // Calculate the data port
    *data_port = (p1 * 256) + p2;

    printf("Data Connection Info - IP: %s, Port: %d\n", data_ip, *data_port);
}

void retrieve_file(int control_sock, int data_sock, const char *file_path) {
    char command[MAX_LENGTH];
    char buffer[MAX_LENGTH];
    FILE *file;

    // Send RETR command
    snprintf(command, sizeof(command), "RETR %s\r\n", file_path);
    send_ftp_command(control_sock, command);

    // Open file for writing
    file = fopen("downloaded_file", "wb");
    if (!file) {
        perror("fopen()");
        exit(-1);
    }

    // Receive data from data socket
    int bytes_received;
    while ((bytes_received = recv(data_sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received < 0) {
        perror("recv()");
    }

    fclose(file);
    printf("File downloaded successfully.\n");
}

int open_data_socket(const char *data_ip, int data_port) {
    return openSocket((char *)data_ip, data_port);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }

    URL parsed_url;
    int result = parse_url(argv[1], &parsed_url);

    if (result != 0) {
        printf("Failed to parse URL\n");
        return -1;
    }

    printf("User: %s\n", parsed_url.user);
    printf("Password: %s\n", parsed_url.password);
    printf("Host: %s\n", parsed_url.host);
    printf("Path: %s\n", parsed_url.path);
    printf("IP: %s\n", parsed_url.ip);

    // Open control socket
    int control_sock = openSocket(parsed_url.ip, SERVER_PORT);
    printf("Connected to server at %s\n", parsed_url.ip);

    // Receive server greeting
    receive_server_greeting(control_sock);

    // Send USER and PASS commands
    send_user_command(control_sock, parsed_url.user);
    send_pass_command(control_sock, parsed_url.password);

    // Enter passive mode
    char data_ip[IP_MAX_LENGTH];
    int data_port;
    passive_mode(control_sock, data_ip, &data_port);

    // Open data socket
    int data_sock = open_data_socket(data_ip, data_port);

    // Retrieve file
    retrieve_file(control_sock, data_sock, parsed_url.path);

    // Close sockets
    close(data_sock);
    close(control_sock);

    return 0;
}
