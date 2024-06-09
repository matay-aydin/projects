#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

const int NODE_SIZE = sizeof(struct node_ts);
const int LIST_SIZE = sizeof(struct list_ts);

clist_t construct_list() {
    clist_t list = (clist_t) malloc(LIST_SIZE);
    list->size = 0;
    list->head = NULL;
    return list;
}

void push(clist_t list, void *data, size_t bytes, size_t pos) {
    if (pos > list->size && list->size > 0)
        return;
    size_t proper = (bytes > 8) ? bytes : 8;
    node_t newnode = (node_t) malloc(NODE_SIZE);
    newnode->data = malloc(proper);
    memcpy(newnode->data, data, bytes);
    newnode->next = NULL;
    if (list->head == NULL)
        list->head = newnode;
    else if (pos == 0) {
        newnode->next = list->head;
        list->head = newnode;
    } else {
        node_t ptr = list->head;
        while (pos > 1) {
            ptr = ptr->next;
            pos--;
        }
        newnode->next = ptr->next;
        ptr->next = newnode;
    }
    list->size += 1;
}

void pop(clist_t list, size_t pos) {
    if (list->head == NULL || list->size <= pos)
        return;
    node_t ptr = list->head;
    if (pos == 0) {
        list->head = list->head->next;
    } else if (pos == list->size - 1) {
        while (ptr->next->next != NULL)
            ptr = ptr->next;
        node_t hold = ptr;
        ptr = ptr->next;
        hold->next = NULL;
    } else {
        while (pos > 1) {
            ptr = ptr->next;
            pos--;
        }
        node_t hold = ptr;
        ptr = ptr->next;
        hold->next = ptr->next;
    }
    free(ptr->data);
    free(ptr);
    list->size -= 1;
}

void* get_from(clist_t list, size_t pos) {
    if (list->head == NULL || pos >= list->size)
        return NULL;
    node_t ptr = list->head;
    while (pos > 0) {
        ptr = ptr->next;
        pos--;
    }
    return ptr->data;
}

size_t size(clist_t list) {
    return list->size;
}

void destruct(clist_t* list) {
    while ((*list)->head != NULL)
        pop(*list, 0);
    free(*list);
    *list = NULL;
}
