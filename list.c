#include "list.h"

// USER LIST
struct node* insertFirstU(struct node *head, int socket, char *username) {
   if (findU(head, username) == NULL) {
       struct node *link = (struct node*) malloc(sizeof(struct node));
       if (!link) return head;
       link->socket = socket;
       strncpy(link->username, username, sizeof(link->username) - 1);
       link->username[sizeof(link->username) - 1] = '\0';
       link->next = head;
       head = link;
   }
   return head;
}

struct node* findU(struct node *head, char* username) {
   struct node* current = head;
   while (current != NULL) {
       if (strcmp(current->username, username) == 0) return current;
       current = current->next;
   }
   return NULL;
}

struct node* findUBySocket(struct node *head, int socket) {
   struct node* current = head;
   while (current != NULL) {
       if (current->socket == socket) return current;
       current = current->next;
   }
   return NULL;
}

struct node* removeU(struct node *head, int socket) {
   struct node *current = head, *previous = NULL;
   while (current != NULL && current->socket != socket) {
       previous = current;
       current = current->next;
   }
   if (current == NULL) return head;
   if (current == head) head = head->next;
   else previous->next = current->next;
   free(current);
   return head;
}

void freeListU(struct node *head) {
   struct node *curr = head;
   while (curr != NULL) {
       struct node *next = curr->next;
       free(curr);
       curr = next;
   }
}

// ROOM LIST
struct room* insertRoom(struct room *head, char *room_name) {
   if (findRoom(head, room_name) == NULL) {
       struct room *r = (struct room*) malloc(sizeof(struct room));
       if (!r) return head;
       strncpy(r->room_name, room_name, sizeof(r->room_name) - 1);
       r->room_name[sizeof(r->room_name) - 1] = '\0';
       r->users = NULL;
       r->next = head;
       head = r;
   }
   return head;
}

struct room* findRoom(struct room *head, char *room_name) {
   struct room* current = head;
   while (current != NULL) {
       if (strcmp(current->room_name, room_name) == 0) return current;
       current = current->next;
   }
   return NULL;
}

void removeUserFromRoom(struct room *r, int socket) {
   if (r) {
       r->users = removeU(r->users, socket);
   }
}

void freeListRooms(struct room *head) {
   struct room *curr = head;
   while (curr != NULL) {
       struct room *next = curr->next;
       freeListU(curr->users);
       free(curr);
       curr = next;
   }
}

// DM CONNECTIONS
struct dm_node* addDM(struct dm_node *head, int sock1, int sock2) {
   if (!isConnectedDM(head, sock1, sock2)) {
       struct dm_node *dm = (struct dm_node*) malloc(sizeof(struct dm_node));
       if (!dm) return head;
       dm->socket1 = sock1;
       dm->socket2 = sock2;
       dm->next = head;
       return dm;
   }
   return head;
}

struct dm_node* removeDM(struct dm_node *head, int sock1, int sock2) {
   struct dm_node *curr = head, *prev = NULL;
   while (curr != NULL) {
       if ((curr->socket1 == sock1 && curr->socket2 == sock2) ||
           (curr->socket1 == sock2 && curr->socket2 == sock1)) {
           if (prev == NULL) head = curr->next;
           else prev->next = curr->next;
           free(curr);
           return head;
       }
       prev = curr;
       curr = curr->next;
   }
   return head;
}

bool isConnectedDM(struct dm_node *head, int sock1, int sock2) {
   struct dm_node *curr = head;
   while (curr != NULL) {
       if ((curr->socket1 == sock1 && curr->socket2 == sock2) ||
           (curr->socket1 == sock2 && curr->socket2 == sock1)) {
           return true;
       }
       curr = curr->next;
   }
   return false;
}

void removeAllDMsForSocket(struct dm_node **head, int socket) {
   struct dm_node *curr = *head, *prev = NULL;
   while (curr != NULL) {
       if (curr->socket1 == socket || curr->socket2 == socket) {
           struct dm_node *temp = curr;
           if (prev == NULL) *head = curr->next;
           else prev->next = curr->next;
           curr = curr->next;
           free(temp);
       } else {
           prev = curr;
           curr = curr->next;
       }
   }
}

void freeListDMs(struct dm_node *head) {
   struct dm_node *curr = head;
   while (curr != NULL) {
       struct dm_node *next = curr->next;
       free(curr);
       curr = next;
   }
}