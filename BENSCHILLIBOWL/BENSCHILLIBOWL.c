#include "BENSCHILLIBOWL.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

bool IsFull(BENSCHILLIBOWL *bcb) {
    return bcb->current_size >= bcb->max_size;
}

void AddOrderToBack(Order **orders, Order *order) {
    if (*orders == NULL) {
        *orders = order;
        order->next = NULL;
        return;
    }
    Order *curr = *orders;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = order;
    order->next = NULL;
}

MenuItem PickRandomMenuItem() {
    MenuItem items[] = {
        "Classic Burger", "Cheese Burger", "Veggie Burger", 
        "Chili Dog", "Fries", "Milkshake", "Onion Rings"
    };
    int num_items = sizeof(items) / sizeof(items[0]);
    return items[rand() % num_items];
}

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL *bcb = (BENSCHILLIBOWL*)malloc(sizeof(BENSCHILLIBOWL));
    if (!bcb) return NULL;

    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->max_size = max_size;
    bcb->next_order_number = 1;
    bcb->orders_handled = 0;
    bcb->expected_num_orders = expected_num_orders;

    // Initialize synchronization primitives
    pthread_mutex_init(&(bcb->mutex), NULL);
    pthread_cond_init(&(bcb->can_add_orders), NULL);
    pthread_cond_init(&(bcb->can_get_orders), NULL);

    printf("Restaurant is now open!\n");
    return bcb;
}

void CloseRestaurant(BENSCHILLIBOWL *bcb) {
    assert(bcb->orders_handled == bcb->expected_num_orders);
    assert(bcb->current_size == 0);
    assert(bcb->orders == NULL);

    pthread_mutex_destroy(&(bcb->mutex));
    pthread_cond_destroy(&(bcb->can_add_orders));
    pthread_cond_destroy(&(bcb->can_get_orders));
    
    free(bcb);
    printf("Restaurant is now closed. All orders successfully fulfilled!\n");
}

int AddOrder(BENSCHILLIBOWL *bcb, Order *order) {
    pthread_mutex_lock(&(bcb->mutex));

    while (IsFull(bcb)) {
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }

    order->order_number = bcb->next_order_number++;
    AddOrderToBack(&(bcb->orders), order);
    bcb->current_size++;

    pthread_cond_signal(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));
    return order->order_number;
}

Order *GetOrder(BENSCHILLIBOWL *bcb) {
    pthread_mutex_lock(&(bcb->mutex));

    while (bcb->orders == NULL) {
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }

    Order *order = bcb->orders;
    bcb->orders = order->next;
    bcb->current_size--;
    bcb->orders_handled++;

    pthread_cond_signal(&(bcb->can_add_orders));
    pthread_mutex_unlock(&(bcb->mutex));
    return order;
}