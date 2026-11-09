#include "server.h"

extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern struct room *room_head;
extern struct dm_node *dm_head;
extern char *server_MOTD;

void reader_lock() {
    pthread_mutex_lock(&mutex);
    numReaders++;
    if (numReaders == 1) pthread_mutex_lock(&rw_lock);
    pthread_mutex_unlock(&mutex);
}

void reader_unlock() {
    pthread_mutex_lock(&mutex);
    numReaders--;
    if (numReaders == 0) pthread_mutex_unlock(&rw_lock);
    pthread_mutex_unlock(&mutex);
}

void writer_lock() { pthread_mutex_lock(&rw_lock); }
void writer_unlock() { pthread_mutex_unlock(&rw_lock); }

char *trimwhitespace(char *str) {
  char *end;
  while (isspace((unsigned char)*str)) str++;
  if (*str == 0) return str;
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;
   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF], tmpbuf[MAXBUFF], cmd[MAXBUFF], username[30];
   char *arguments[80];

   send(client, server_MOTD, strlen(server_MOTD), 0);

   sprintf(username, "guest%d", client);
   writer_lock();
   head = insertFirstU(head, client, username);
   struct room *lobby = findRoom(room_head, "Lobby");
   if (lobby) lobby->users = insertFirstU(lobby->users, client, username);
   writer_unlock();

   while (1) {
      if ((received = read(client, buffer, MAXBUFF)) > 0) {
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);

            arguments[0] = strtok(cmd, delimiters);
            if (arguments[0] == NULL) {
                memset(buffer, 0, sizeof(buffer));
                continue;
            }

            i = 0;
            while (arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, delimiters); 
                if (arguments[i-1] != NULL) strcpy(arguments[i-1], trimwhitespace(arguments[i-1]));
            } 

            if (strcmp(arguments[0], "create") == 0) {
               if (arguments[1] != NULL) {
                   writer_lock();
                   if (findRoom(room_head, arguments[1]) == NULL) {
                       room_head = insertRoom(room_head, arguments[1]);
                       sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
                   } else {
                       sprintf(buffer, "Room '%s' already exists.\nchat>", arguments[1]);
                   }
                   writer_unlock();
               } else sprintf(buffer, "Usage: create <room>\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "join") == 0) {
               if (arguments[1] != NULL) {
                   writer_lock();
                   struct room *r = findRoom(room_head, arguments[1]);
                   struct node *u = findUBySocket(head, client);
                   if (r && u) {
                       r->users = insertFirstU(r->users, client, u->username);
                       sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
                   } else sprintf(buffer, "Room '%s' does not exist.\nchat>", arguments[1]);
                   writer_unlock();
               } else sprintf(buffer, "Usage: join <room>\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "leave") == 0) {
               if (arguments[1] != NULL) {
                   writer_lock();
                   struct room *r = findRoom(room_head, arguments[1]);
                   if (r) {
                       removeUserFromRoom(r, client);
                       sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
                   } else sprintf(buffer, "Room '%s' not found.\nchat>", arguments[1]);
                   writer_unlock();
               } else sprintf(buffer, "Usage: leave <room>\nchat>");
               send(client, buffer, strlen(buffer), 0);
            } 
            else if (strcmp(arguments[0], "connect") == 0) {
               if (arguments[1] != NULL) {
                   writer_lock();
                   struct node *target = findU(head, arguments[1]);
                   if (target && target->socket != client) {
                       dm_head = addDM(dm_head, client, target->socket);
                       sprintf(buffer, "Connected to user '%s'.\nchat>", arguments[1]);
                   } else sprintf(buffer, "User '%s' not found or invalid.\nchat>", arguments[1]);
                   writer_unlock();
               } else sprintf(buffer, "Usage: connect <user>\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "disconnect") == 0) {
               if (arguments[1] != NULL) {
                   writer_lock();
                   struct node *target = findU(head, arguments[1]);
                   if (target) {
                       dm_head = removeDM(dm_head, client, target->socket);
                       sprintf(buffer, "Disconnected from user '%s'.\nchat>", arguments[1]);
                   } else sprintf(buffer, "User '%s' not found.\nchat>", arguments[1]);
                   writer_unlock();
               } else sprintf(buffer, "Usage: disconnect <user>\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }                  
            else if (strcmp(arguments[0], "rooms") == 0) {
                reader_lock();
                sprintf(buffer, "Available Rooms:\n");
                struct room *r = room_head;
                while (r != NULL) {
                    strcat(buffer, " - ");
                    strcat(buffer, r->room_name);
                    strcat(buffer, "\n");
                    r = r->next;
                }
                reader_unlock();
                strcat(buffer, "chat>");
                send(client, buffer, strlen(buffer), 0);                          
            }   
            else if (strcmp(arguments[0], "users") == 0) {
                reader_lock();
                sprintf(buffer, "Connected Users:\n");
                struct node *u = head;
                while (u != NULL) {
                    strcat(buffer, " - ");
                    strcat(buffer, u->username);
                    strcat(buffer, "\n");
                    u = u->next;
                }
                reader_unlock();
                strcat(buffer, "chat>");
                send(client, buffer, strlen(buffer), 0);
            }                           
            else if (strcmp(arguments[0], "login") == 0) {
                if (arguments[1] != NULL) {
                    writer_lock();
                    struct node *u = findUBySocket(head, client);
                    if (u) {
                        strncpy(u->username, arguments[1], sizeof(u->username) - 1);
                        u->username[sizeof(u->username) - 1] = '\0';
                        
                        // Update username in all rooms
                        struct room *r = room_head;
                        while (r != NULL) {
                            struct node *ru = findUBySocket(r->users, client);
                            if (ru) {
                                strncpy(ru->username, arguments[1], sizeof(ru->username) - 1);
                                ru->username[sizeof(ru->username) - 1] = '\0';
                            }
                            r = r->next;
                        }
                    }
                    writer_unlock();
                    sprintf(buffer, "Logged in as %s\nchat>", arguments[1]);
                } else sprintf(buffer, "Usage: login <username>\nchat>");
                send(client, buffer, strlen(buffer), 0);
            } 
            else if (strcmp(arguments[0], "help") == 0) {
                sprintf(buffer, "login <username> - \"login with username\"\ncreate <room> - \"create a room\"\njoin <room> - \"join a room\"\nleave <room> - \"leave a room\"\nusers - \"list all users\"\nrooms - \"list all rooms\"\nconnect <user> - \"connect to user\"\nexit - \"exit chat\"\nchat>");
                send(client, buffer, strlen(buffer), 0); 
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                writer_lock();
                struct room *r = room_head;
                while (r != NULL) {
                    removeUserFromRoom(r, client);
                    r = r->next;
                }
                removeAllDMsForSocket(&dm_head, client);
                head = removeU(head, client);
                writer_unlock();
                close(client);
                pthread_exit(NULL);
            }                         
            else { 
                 // Message filtering based on shared rooms or DM connections
                 reader_lock();
                 struct node *sender = findUBySocket(head, client);
                 char sender_name[30] = "Unknown";
                 if (sender) strcpy(sender_name, sender->username);

                 sprintf(tmpbuf, "\n::[%s]> %s\nchat>", sender_name, sbuffer);
                 
                 struct node *target = head;
                 while (target != NULL) {
                     if (target->socket != client) {
                         bool send_msg = false;
                         
                         // Check DM connection
                         if (isConnectedDM(dm_head, client, target->socket)) {
                             send_msg = true;
                         }
                         
                         // Check room sharing
                         if (!send_msg) {
                             struct room *r = room_head;
                             while (r != NULL) {
                                 if (findUBySocket(r->users, client) && findUBySocket(r->users, target->socket)) {
                                     send_msg = true;
                                     break;
                                 }
                                 r = r->next;
                             }
                         }

                         if (send_msg) {
                             send(target->socket, tmpbuf, strlen(tmpbuf), 0);
                         }
                     }
                     target = target->next;
                 }
                 reader_unlock();
            }
 
            memset(buffer, 0, sizeof(buffer));
      } else {
          break; // Socket closed or error
      }
   }

   // Fallback cleanup if thread exits while loop
   writer_lock();
   struct room *r = room_head;
   while (r != NULL) {
       removeUserFromRoom(r, client);
       r = r->next;
   }
   removeAllDMsForSocket(&dm_head, client);
   head = removeU(head, client);
   writer_unlock();
   close(client);
   return NULL;
}