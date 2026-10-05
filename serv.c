#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "list/list.h"

#define PORT 9001
#define ACK "ACK: "

list_t *mylist = NULL;
int servSockD = -1;
int clientSocket = -1;

void handle_signal(int sig) {
    if (mylist != NULL) {
        list_free(mylist);
        mylist = NULL;
    }
    if (clientSocket != -1) close(clientSocket);
    if (servSockD != -1) close(servSockD);
    printf("\nServer terminated gracefully.\n");
    exit(0);
}

int main(int argc, char const *argv[]) {
    int n, val, idx;
    char buf[1024];
    char sbuf[1024];
    char *token;
    struct sockaddr_in servAddr;

    signal(SIGINT, handle_signal);

    servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD < 0) {
        exit(1);
    }

    int opt = 1;
    setsockopt(servSockD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        exit(1);
    }

    listen(servSockD, 1);
    clientSocket = accept(servSockD, NULL, NULL);

    mylist = list_alloc();

    while (1) {
        n = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';

        token = strtok(buf, " \n\r");
        if (!token) continue;

        memset(sbuf, '\0', sizeof(sbuf));

        if (strcmp(token, "exit") == 0) {
            if (mylist) {
                list_free(mylist);
                mylist = NULL;
            }
            close(clientSocket);
            close(servSockD);
            exit(0);
        } else if (strcmp(token, "print") == 0) {
            char *s = listToString(mylist);
            if (s) {
                snprintf(sbuf, sizeof(sbuf), "%s", s);
                free(s);
            } else {
                snprintf(sbuf, sizeof(sbuf), "NULL");
            }
        } else if (strcmp(token, "get_length") == 0) {
            val = list_length(mylist);
            snprintf(sbuf, sizeof(sbuf), "Length = %d", val);
        } else if (strcmp(token, "add_back") == 0) {
            token = strtok(NULL, " \n\r");
            if (token) {
                val = atoi(token);
                list_add_to_back(mylist, val);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            } else {
                snprintf(sbuf, sizeof(sbuf), "Error: Missing Value");
            }
        } else if (strcmp(token, "add_front") == 0) {
            token = strtok(NULL, " \n\r");
            if (token) {
                val = atoi(token);
                list_add_to_front(mylist, val);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            } else {
                snprintf(sbuf, sizeof(sbuf), "Error: Missing Value");
            }
        } else if (strcmp(token, "add_position") == 0) {
            char *idx_str = strtok(NULL, " \n\r");
            char *val_str = strtok(NULL, " \n\r");
            if (idx_str && val_str) {
                idx = atoi(idx_str);
                val = atoi(val_str);
                list_add_at_index(mylist, val, idx);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            } else {
                snprintf(sbuf, sizeof(sbuf), "Error: Invalid Arguments");
            }
        } else if (strcmp(token, "remove_back") == 0) {
            val = list_remove_from_back(mylist);
            snprintf(sbuf, sizeof(sbuf), "Removed = %d", val);
        } else if (strcmp(token, "remove_front") == 0) {
            val = list_remove_from_front(mylist);
            snprintf(sbuf, sizeof(sbuf), "Removed = %d", val);
        } else if (strcmp(token, "remove_position") == 0) {
            token = strtok(NULL, " \n\r");
            if (token) {
                idx = atoi(token);
                val = list_remove_at_index(mylist, idx);
                snprintf(sbuf, sizeof(sbuf), "Removed = %d", val);
            } else {
                snprintf(sbuf, sizeof(sbuf), "Error: Missing Index");
            }
        } else if (strcmp(token, "get") == 0) {
            token = strtok(NULL, " \n\r");
            if (token) {
                idx = atoi(token);
                val = list_get_elem_at(mylist, idx);
                snprintf(sbuf, sizeof(sbuf), "Value at %d = %d", idx, val);
            } else {
                snprintf(sbuf, sizeof(sbuf), "Error: Missing Index");
            }
        } else {
            snprintf(sbuf, sizeof(sbuf), "Unknown Command");
        }

        send(clientSocket, sbuf, strlen(sbuf) + 1, 0);
        memset(buf, '\0', sizeof(buf));
    }

    if (mylist) list_free(mylist);
    if (clientSocket != -1) close(clientSocket);
    if (servSockD != -1) close(servSockD);
    return 0;
}