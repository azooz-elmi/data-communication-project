#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define DATA_SIZE 255
#define MAX_CLIENTS 10

struct thread_data {
    int sockfd;
    char username[DATA_SIZE];
};

int client_count = 0;
int client_sockets[MAX_CLIENTS];
char *client_usernames[MAX_CLIENTS];

void broadcast_message(const char *message, int sender_sockfd) {
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] != sender_sockfd) {
            write(client_sockets[i], message, strlen(message));
        }
    }
}

void send_private_message(const char *message, const char *recipient_username, int sender_sockfd) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(client_usernames[i], recipient_username) == 0) {
            write(client_sockets[i], message, strlen(message));
            return;
        }
    }
    // If recipient not found
    char not_found_msg[DATA_SIZE];
    snprintf(not_found_msg, sizeof(not_found_msg), "User %s not found.\n", recipient_username);
    write(sender_sockfd, not_found_msg, strlen(not_found_msg));
}

void *handle_client(void *thread_arg) {
    struct thread_data *my_data = (struct thread_data *) thread_arg;
    int sockfd = my_data->sockfd;
    char buffer[DATA_SIZE];
    int n;

    // Add the new client to the list
    client_sockets[client_count] = sockfd;
    client_usernames[client_count] = strdup(my_data->username);
    client_count++;

    // Welcome message
    char welcome_msg[DATA_SIZE];
    snprintf(welcome_msg, sizeof(welcome_msg), "Welcome %s! Type /help for a list of commands.\n", my_data->username);
    write(sockfd, welcome_msg, strlen(welcome_msg));

    while (1) {
        bzero(buffer, sizeof(buffer));
        n = read(sockfd, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            perror("ERROR reading from socket");
            break; // Exit if the connection is lost
        }

        // Handle commands
        if (strcmp(buffer, "/list") == 0) {
            char list_message[DATA_SIZE] = "Connected users:\n";
            for (int i = 0; i < client_count; i++) {
                if (client_usernames[i] != NULL) {
                    strcat(list_message, client_usernames[i]);
                    strcat(list_message, "\n");
                }
            }
            write(sockfd, list_message, strlen(list_message));
            continue; // Skip broadcasting for this command
        } else if (strcmp(buffer, "/help") == 0) {
            const char *help_message = 
                "Available commands:\n"
                "/list - Show connected users\n"
                "/help - Show this help message\n"
                "/logout - Disconnect from the server\n"
                "/msg <username> <message> - Send a private message\n";
            write(sockfd, help_message, strlen(help_message));
            continue; // Skip broadcasting for this command
        } else if (strncmp(buffer, "/msg ", 5) == 0) {
            // Handle private message
            char recipient[DATA_SIZE], message[DATA_SIZE];
            sscanf(buffer + 5, "%s %[^\n]", recipient, message);
            char private_message[DATA_SIZE];
            snprintf(private_message, sizeof(private_message), "[Private from %s]: %s\n", my_data->username, message);
            send_private_message(private_message, recipient, sockfd);
            continue; // Skip broadcasting for this command
        } else if (strcmp(buffer, "/logout") == 0) {
            char logout_message[DATA_SIZE];
            snprintf(logout_message, sizeof(logout_message), "%s has logged out.\n", my_data->username);
            broadcast_message(logout_message, sockfd);
            break; // Exit the loop
        }

        printf("%s: %s\n", my_data->username, buffer);
        broadcast_message(buffer, sockfd); // Broadcast the message to other clients
    }

    // Remove the client from the list
    close(sockfd);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] == sockfd) {
            free(client_usernames[i]);
            client_usernames[i] = client_usernames[client_count - 1];
            client_sockets[i] = client_sockets[client_count - 1];
            client_count--;
            break;
        }
    }

    free(my_data); // Free the allocated memory
    return NULL;
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    pthread_t pt;

    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            continue; // Continue to accept other connections
        }

        struct thread_data *t_data = malloc(sizeof(struct thread_data));
        if (t_data == NULL) {
            perror("ERROR allocating memory");
            close(newsockfd);
            continue; // Continue to accept other connections
        }

        // Receive the username from the client
        bzero(t_data->username, DATA_SIZE);
        read(newsockfd, t_data->username, DATA_SIZE - 1);
        t_data->sockfd = newsockfd;

        pthread_create(&pt, NULL, handle_client, (void *)t_data);
        pthread_detach(pt); // Detach the thread to allow it to clean up after itself
    }

    close(sockfd);
    return 0;
}
