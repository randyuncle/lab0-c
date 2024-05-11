#include "listsort.h"

void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp);
void timsort_old(void *priv, struct list_head *head, list_cmp_func_t cmp);
void timsort_binary(void *priv, struct list_head *head, list_cmp_func_t cmp);
void timsort_gallop(void *priv, struct list_head *head, list_cmp_func_t cmp);