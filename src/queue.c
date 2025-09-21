#include "queue.h"

#include <stdlib.h>

queue_t *queue_init() {
	queue_t *queue = malloc(sizeof(queue_t));

	queue->head = NULL;
	queue->size = 0;

	return queue;
}

void queue_add(queue_t *queue, void *data) {
	node_t *node = malloc(sizeof(node_t));
	node->next = NULL;
	node->value = data;

	if (queue->size == 0) {
		queue->head = node;
		queue->size = 1;
		return;
	}

	node_t *head = queue->head;
	queue->head = node;
	queue->size++;
	node->next = head;

	return;
}

void *queue_pop(queue_t *queue) {
	if (queue->size == 0) {
		return NULL;
	}

	if (queue->size == 1) {
		void *data = queue->head->value;
		free(queue->head);
		queue->head = NULL;
		queue->size = 0;
		return data;
	}

	node_t *cur = queue->head;
	while (cur->next && cur->next->next) {
		cur = cur->next;
	}

	void *data = cur->next->value;
	free(cur->next);
	cur->next = NULL;
	queue->size--;

	return data;
}

void queue_free(queue_t *queue) {
	if (queue->size == 0) {
		free(queue);
		return;
	}

	node_t *cur = queue->head;
	while (cur) {
		node_t *next = cur->next;
		free(cur->value);
		free(cur);
		cur = next;
	}

	free(queue);

	return;
}
