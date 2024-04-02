/**
 * The following `likely` and `unlikely` definition from <linux/compiler.h>
 * only requires the `__builtin_expect()` in gnu gcc.
 */
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

typedef int (*list_cmp_func_t)(const struct list_head *,
                               const struct list_head *);

void q_list_sort(struct list_head *head, bool descend);