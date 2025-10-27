// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
// clang-format on

#include "queue.h"

static void test_queue_init(void **state) {
	(void)state;

	queue_t *q = queue_init();
	assert_non_null(q);
	assert_null(q->head);
	assert_int_equal(q->size, 0);
	queue_free(q);
}

static void test_queue_add_and_pop(void **state) {
	(void)state;

	queue_t *q = queue_init();
	int *a = malloc(sizeof(int));
	*a = 17;
	int *b = malloc(sizeof(int));
	*b = 122;

	queue_add(q, a);
	queue_add(q, b);

	assert_int_equal(q->size, 2);

	void *first = queue_pop(q);
	assert_ptr_equal(first, a);

	void *second = queue_pop(q);
	assert_ptr_equal(second, b);

	assert_int_equal(q->size, 0);
	assert_null(queue_pop(q));

	queue_free(q);
}

static void test_queue_free_empty(void **state) {
	(void)state;

	queue_t *q = queue_init();
	queue_free(q);
}

static void test_queue_free_filled(void **state) {
	(void)state;

	(void)state;

	queue_t *q = queue_init();
	int *a = malloc(sizeof(int));
	*a = 17;
	int *b = malloc(sizeof(int));
	*b = 122;

	queue_add(q, a);
	queue_add(q, b);

	queue_free(q);
}

int main(void) {
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(test_queue_init),
			cmocka_unit_test(test_queue_add_and_pop),
			cmocka_unit_test(test_queue_free_empty),
			cmocka_unit_test(test_queue_free_filled),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
