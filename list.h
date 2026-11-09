#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// User node structure
struct node {
   char username[30];
   int socket;
   struct node *next;
};

// DM connection node structure
struct dm_node {
   int socket1;
   int socket2;
   struct dm_node *next;
};

// Room structure containing users in that room
struct room {
   char room_name[30];
   struct node *users;
   struct room *next;
};

// User Operations
struct node* insertFirstU(struct node *head, int socket, char *username);
struct node* findU(struct node *head, char* username);
struct node* findUBySocket(struct node *head, int socket);
struct node* removeU(struct node *head, int socket);
void freeListU(struct node *head);

// Room Operations
struct room* insertRoom(struct room *head, char *room_name);
struct room* findRoom(struct room *head, char *room_name);
void removeUserFromRoom(struct room *r, int socket);
void freeListRooms(struct room *head);

// DM Operations
struct dm_node* addDM(struct dm_node *head, int sock1, int sock2);
struct dm_node* removeDM(struct dm_node *head, int sock1, int sock2);
bool isConnectedDM(struct dm_node *head, int sock1, int sock2);
void removeAllDMsForSocket(struct dm_node **head, int socket);
void freeListDMs(struct dm_node *head);

#endif