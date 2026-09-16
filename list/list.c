/*
 * File: list.c
 * Author: GALTAYB
 * Description: Implementation of a singly linked list with optimal string construction,
 *              NULL safety, and helper functions to prevent code duplication.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* Helper function to get a node at a specific 1-based index to avoid redundant traversal */
static node_t *getNodeAtIndex(list_t *l, int index) {
    if (l == NULL || index <= 0) return NULL;
    node_t *curr = l->head;
    int i = 1;
    while (curr != NULL && i < index) {
        curr = curr->next;
        i++;
    }
    return curr;
}

/* Helper function to allocate and initialize a new node */
static node_t *getNode(elem value) {
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) {
        return NULL;
    }
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

/* Allocates and initializes a new list */
list_t *list_alloc() {
    list_t *l = (list_t *)malloc(sizeof(list_t));
    if (l == NULL) {
        return NULL;
    }
    l->head = NULL;
    return l;
}

/* Frees all allocated memory associated with the list */
void list_free(list_t *l) {
    if (l == NULL) return;
    node_t *curr = l->head;
    while (curr != NULL) {
        node_t *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    free(l);
}

/* Prints the contents of the list */
void list_print(list_t *l) {
    if (l == NULL) return;
    node_t *curr = l->head;
    while (curr != NULL) {
        printf("%d -> ", curr->value);
        curr = curr->next;
    }
    printf("NULL\n");
}

/* Converts list values into a string using snprintf pointer tracking (O(N) time) */
char *listToString(list_t *l) {
    int num_nodes = list_length(l);
    size_t buf_size = (num_nodes * 20) + 10;
    if (buf_size < 64) buf_size = 64;

    char *buf = (char *)malloc(buf_size);
    if (buf == NULL) return NULL;
    
    char *ptr = buf;
    size_t remaining = buf_size;

    if (l == NULL) {
        snprintf(ptr, remaining, "NULL");
        return buf;
    }

    node_t *curr = l->head;
    while (curr != NULL) {
        int written = snprintf(ptr, remaining, "%d->", curr->value);
        if (written < 0 || (size_t)written >= remaining) break;
        ptr += written;
        remaining -= written;
        curr = curr->next;
    }
    snprintf(ptr, remaining, "NULL");
    return buf;
}

/* Returns the length of the list */
int list_length(list_t *l) {
    if (l == NULL) return 0;
    int count = 0;
    node_t *curr = l->head;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

/* Adds an element to the back of the list */
void list_add_to_back(list_t *l, elem value) {
    if (l == NULL) return;
    node_t *new_node = getNode(value);
    if (new_node == NULL) return;

    if (l->head == NULL) {
        l->head = new_node;
        return;
    }
    node_t *curr = l->head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_node;
}

/* Adds an element to the front of the list */
void list_add_to_front(list_t *l, elem value) {
    if (l == NULL) return;
    node_t *new_node = getNode(value);
    if (new_node == NULL) return;

    new_node->next = l->head;
    l->head = new_node;
}

/* Adds an element at a specific 1-based index */
void list_add_at_index(list_t *l, elem value, int index) {
    if (l == NULL || index <= 0) return;
    if (index == 1) {
        list_add_to_front(l, value);
        return;
    }

    node_t *prev = getNodeAtIndex(l, index - 1);
    if (prev == NULL) return;

    node_t *new_node = getNode(value);
    if (new_node == NULL) return;

    new_node->next = prev->next;
    prev->next = new_node;
}

/* Removes an element from the back of the list and returns its value */
elem list_remove_from_back(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;

    if (l->head->next == NULL) {
        elem val = l->head->value;
        free(l->head);
        l->head = NULL;
        return val;
    }

    node_t *curr = l->head;
    while (curr->next->next != NULL) {
        curr = curr->next;
    }

    elem val = curr->next->value;
    free(curr->next);
    curr->next = NULL;
    return val;
}

/* Removes an element from the front of the list and returns its value */
elem list_remove_from_front(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;

    node_t *tmp = l->head;
    elem val = tmp->value;
    l->head = l->head->next;
    free(tmp);
    return val;
}

/* Removes an element at a given 1-based index and returns its value */
elem list_remove_at_index(list_t *l, int index) {
    if (l == NULL || l->head == NULL || index <= 0) return -1;

    if (index == 1) {
        return list_remove_from_front(l);
    }

    node_t *prev = getNodeAtIndex(l, index - 1);
    if (prev == NULL || prev->next == NULL) return -1;

    node_t *tmp = prev->next;
    elem val = tmp->value;
    prev->next = tmp->next;
    free(tmp);
    return val;
}

/* Checks if a value exists in the list */
bool list_is_in(list_t *l, elem value) {
    if (l == NULL) return false;
    node_t *curr = l->head;
    while (curr != NULL) {
        if (curr->value == value) return true;
        curr = curr->next;
    }
    return false;
}

/* Returns the element at a specific 1-based index */
elem list_get_elem_at(list_t *l, int index) {
    node_t *node = getNodeAtIndex(l, index);
    if (node == NULL) return -1;
    return node->value;
}

/* Returns the 1-based index of a given value */
int list_get_index_of(list_t *l, elem value) {
    if (l == NULL) return -1;
    node_t *curr = l->head;
    int index = 1;
    while (curr != NULL) {
        if (curr->value == value) return index;
        curr = curr->next;
        index++;
    }
    return -1;
}