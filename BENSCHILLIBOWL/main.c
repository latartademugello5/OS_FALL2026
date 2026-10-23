#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "BENSCHILLIBOWL.h"

// Global pointer to the restaurant
BENSCHILLIBOWL *bcb;

#define ORDER_LIMIT 3
#define NUM_CUSTOMERS 10
#define NUM_COOKS 2
#define ORDERS_PER_CUSTOMER 3

void *BENSCHILLIBOWLCustomer(void *tid) {
    long customer_id = (long)tid;

    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        Order *order = (Order*)malloc(sizeof(Order));
        if (!order) {
            perror("Failed to allocate order");
            pthread_exit(NULL);
        }

        order->customer_id = customer_id;
        order->menu_item = PickRandomMenuItem();
        order->next = NULL;

        int assigned_id = AddOrder(bcb, order);
        printf("Customer %ld adds order #%d (%s)\n", customer_id, assigned_id, order->menu_item);
        
        usleep(rand() % 100000);
    }
    pthread_exit(NULL);
}

void *BENSCHILLIBOWLCook(void *tid) {
    long cook_id = (long)tid;
    int orders_fulfilled = 0;

    while (1) {
        Order *order = GetOrder(bcb);
        if (order == NULL) {
            break;
        }

        printf("Cook %ld fulfills order #%d (%s) for Customer %d\n", 
               cook_id, order->order_number, order->menu_item, order->customer_id);
        
        orders_fulfilled++;
        free(order);

        usleep(rand() % 100000);
    }

    printf("Cook %ld is done. Fulfilled %d orders.\n", cook_id, orders_fulfilled);
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    int expected_total_orders = NUM_CUSTOMERS * ORDERS_PER_CUSTOMER;

    bcb = OpenRestaurant(ORDER_LIMIT, expected_total_orders);

    pthread_t customers[NUM_CUSTOMERS];
    pthread_t cooks[NUM_COOKS];

    long i;
    for (i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customers[i], NULL, BENSCHILLIBOWLCustomer, (void*)(i + 1));
    }

    for (i = 0; i < NUM_COOKS; i++) {
        pthread_create(&cooks[i], NULL, BENSCHILLIBOWLCook, (void*)(i + 1));
    }

    for (i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    sleep(1);

    CloseRestaurant(bcb);

    return 0;
}