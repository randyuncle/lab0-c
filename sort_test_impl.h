/**
 * The program that do the list modification for the `sort_test.c`
 */

#include <list.h>
#include <stdbool.h>
#include <stdlib.h>

struct list_head *merge_list(struct list_head *left, struct list_head *right);
void shuffle(struct list_head *head);
void worst_case_generator(struct list_head *head);