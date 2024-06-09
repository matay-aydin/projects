#ifndef _LIST
#define _LIST

#define ALL (1 << 0)

typedef struct node_ts* node_t;

struct node_ts {
    void *data;
    struct node_ts *next;
};

extern const int NODE_SIZE;

typedef struct list_ts* clist_t;

struct list_ts {
    node_t head;
    size_t size;
};

extern const int LIST_SIZE;

clist_t construct_list();

void push(clist_t list, void *data, size_t bytes, size_t pos);

void pop(clist_t list, size_t pos);

void* get_from(clist_t list, size_t pos);

size_t size(clist_t list);

void destruct(clist_t* list);

#endif
