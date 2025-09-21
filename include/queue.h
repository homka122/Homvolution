#include <stdlib.h>

typedef struct node_t {
	struct node_t *next;
	void *value;
} node_t;

typedef struct {
	node_t *head;
	size_t size;
} queue_t;

queue_t *queue_init();
void queue_add(queue_t *queue, void *data);
void *queue_pop(queue_t *queue);
void queue_free(queue_t *queue);