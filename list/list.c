#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/*
 * Allocates memory for a new list and returns a pointer to it.
 */
list_t *list_alloc() {
    list_t *l = (list_t *)malloc(sizeof(list_t));
    if (l == NULL) {
        return NULL;
    }
    l->head = NULL;
    return l;
}

/*
 * Frees all nodes in the list and the list structure itself.
 */
void list_free(list_t *l) {
    if (l == NULL) return;
    
    node_t *current = l->head;
    node_t *next_node;
    
    while (current != NULL) {
        next_node = current->next;
        free(current);
        current = next_node;
    }
    free(l);
}

/*
 * Helper function to allocate and initialize a new node.
 */
node_t *getNode(elem value) {
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) {
        return NULL;
    }
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

/*
 * Prints the list elements to stdout.
 */
void list_print(list_t *l) {
    if (l == NULL || l->head == NULL) {
        printf("NULL\n");
        return;
    }
    node_t *current = l->head;
    while (current != NULL) {
        printf("%d -> ", current->value);
        current = current->next;
    }
    printf("NULL\n");
}

/*
 * Returns a dynamically allocated string representation of the list.
 */
char *listToString(list_t *l) {
    char *buf = (char *)malloc(1024 * sizeof(char));
    if (buf == NULL) return NULL;
    
    buf[0] = '\0';
    
    if (l == NULL || l->head == NULL) {
        strcpy(buf, "NULL");
        return buf;
    }
    
    node_t *current = l->head;
    char temp[32];
    
    while (current != NULL) {
        sprintf(temp, "%d->", current->value);
        strcat(buf, temp);
        current = current->next;
    }
    strcat(buf, "NULL");
    return buf;
}

/*
 * Returns the number of elements in the list.
 */
int list_length(list_t *l) {
    if (l == NULL) return 0;
    
    int count = 0;
    node_t *current = l->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

/*
 * Adds an element to the back (end) of the list.
 */
void list_add_to_back(list_t *l, elem value) {
    if (l == NULL) return;
    
    node_t *new_node = getNode(value);
    if (new_node == NULL) return;
    
    if (l->head == NULL) {
        l->head = new_node;
        return;
    }
    
    node_t *current = l->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

/*
 * Adds an element to the front (head) of the list.
 */
void list_add_to_front(list_t *l, elem value) {
    if (l == NULL) return;
    
    node_t *new_node = getNode(value);
    if (new_node == NULL) return;
    
    new_node->next = l->head;
    l->head = new_node;
}

/*
 * Adds an element at a specific 1-based or 0-based index.
 */
void list_add_at_index(list_t *l, elem value, int index) {
    if (l == NULL) return;
    
    if (index <= 1 || l->head == NULL) {
        list_add_to_front(l, value);
        return;
    }
    
    node_t *new_node = getNode(value);
    if (new_node == NULL) return;
    
    node_t *current = l->head;
    int pos = 1;
    
    while (current->next != NULL && pos < index - 1) {
        current = current->next;
        pos++;
    }
    
    new_node->next = current->next;
    current->next = new_node;
}

/*
 * Removes and returns the element from the back of the list.
 */
elem list_remove_from_back(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;
    
    if (l->head->next == NULL) {
        elem val = l->head->value;
        free(l->head);
        l->head = NULL;
        return val;
    }
    
    node_t *current = l->head;
    while (current->next->next != NULL) {
        current = current->next;
    }
    
    elem val = current->next->value;
    free(current->next);
    current->next = NULL;
    return val;
}

/*
 * Removes and returns the element from the front of the list.
 */
elem list_remove_from_front(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;
    
    node_t *temp = l->head;
    elem val = temp->value;
    l->head = l->head->next;
    free(temp);
    return val;
}

/*
 * Removes and returns the element at a specific index.
 */
elem list_remove_at_index(list_t *l, int index) {
    if (l == NULL || l->head == NULL) return -1;
    
    if (index <= 1) {
        return list_remove_from_front(l);
    }
    
    node_t *current = l->head;
    int pos = 1;
    
    while (current->next != NULL && pos < index - 1) {
        current = current->next;
        pos++;
    }
    
    if (current->next == NULL) return -1;
    
    node_t *target = current->next;
    elem val = target->value;
    current->next = target->next;
    free(target);
    return val;
}

/*
 * Checks if value is in the list. Returns 1 if true, 0 if false.
 */
bool list_is_in(list_t *l, elem value) {
    if (l == NULL) return false;
    
    node_t *current = l->head;
    while (current != NULL) {
        if (current->value == value) {
            return true;
        }
        current = current->next;
    }
    return false;
}

/*
 * Returns the element at a specific index without removing it.
 */
elem list_get_elem_at(list_t *l, int index) {
    if (l == NULL || l->head == NULL) return -1;
    
    node_t *current = l->head;
    int pos = 1;
    
    while (current != NULL && pos < index) {
        current = current->next;
        pos++;
    }
    
    if (current == NULL) return -1;
    return current->value;
}

/*
 * Returns the index of a specific element value.
 */
int list_get_index_of(list_t *l, elem value) {
    if (l == NULL) return -1;
    
    node_t *current = l->head;
    int index = 1;
    
    while (current != NULL) {
        if (current->value == value) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1;
}