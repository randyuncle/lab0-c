#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "queue.h"
#include "timsort.h"

int minrun = 0;

int tim_cmp(void *priv, const struct list_head *a, const struct list_head *b)
{
    element_t *element_a = list_entry(a, element_t, list);
    element_t *element_b = list_entry(b, element_t, list);

    int res = strcmp(element_a->value, element_b->value);

    /* if a and b were the same, did not count it */
    if (!res)
        return 0;

    if (priv)
        *((int *) priv) += 1;

    return res;
}

/** For testing only !!
 * Comparison function that doesn't increment the comparison count during
 * insertion sort.
 */
int insert_cmp(void *priv, const struct list_head *a, const struct list_head *b)
{
    element_t *element_a = list_entry(a, element_t, list);
    element_t *element_b = list_entry(b, element_t, list);

    return strcmp(element_a->value, element_b->value);
}

static inline size_t run_size(struct list_head *head)
{
    if (!head)
        return 0;
    if (!head->next)
        return 1;
    return (size_t) (head->next->prev);
}

struct pair {
    struct list_head *head, *next;
};

static size_t stk_size;

static struct list_head *merge(void *priv,
                               list_cmp_func_t cmp,
                               struct list_head *a,
                               struct list_head *b)
{
    struct list_head *head = NULL;
    struct list_head **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

static void build_prev_link(struct list_head *head,
                            struct list_head *tail,
                            struct list_head *list)
{
    tail->next = list;
    do {
        list->prev = tail;
        tail = list;
        list = list->next;
    } while (list);

    /* The final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

static void merge_final(void *priv,
                        list_cmp_func_t cmp,
                        struct list_head *head,
                        struct list_head *a,
                        struct list_head *b)
{
    struct list_head *tail = head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    build_prev_link(head, tail, b);
}

static struct pair find_run(void *priv,
                            struct list_head *list,
                            list_cmp_func_t cmp)
{
    // printf("start find run\n");
    size_t len = 1;
    struct list_head *next = list->next, *head = list;
    struct pair result;

    if (!next) {
        result.head = head, result.next = next;
        return result;
    }

    if (cmp(priv, list, next) > 0) {
        /* decending run, also reverse the list */
        struct list_head *prev = NULL;
        do {
            len++;
            list->next = prev;
            prev = list;
            list = next;
            next = list->next;
            head = list;
        } while (next && cmp(priv, list, next) > 0);
        list->next = prev;
        // printf("\nreverse\n");
    } else {
        do {
            len++;
            list = next;
            next = list->next;
        } while (next && cmp(priv, list, next) <= 0);
        list->next = NULL;
    }

    // rebuild the prev links for each node (important step if need to do
    // insertion sort)
    for (struct list_head *curr = head; curr && curr->next; curr = curr->next)
        curr->next->prev = curr;

    // insertion sort for inserting the elements for making every run be
    // approximately equal length.
    for (struct list_head *in_node = next; in_node && len < minrun; len++) {
        // printf("start insert len = %ld\n", len);
        struct list_head *safe = in_node->next;

        // case for first node hit
        if (!(tim_cmp(priv, in_node, head) > 0)) {
            in_node->next = head;
            head->prev = in_node;
            head = in_node;
            // printf("head chg - head: %s ; prev head: %s\n", list_entry(head,
            // element_t, list)->value, list_entry(head->next, element_t,
            // list)->value);

            in_node = safe;
            next = in_node;

            continue;
        }

        struct list_head *prev = head, *curr = head->next;
        // printf("*start* - prev: %s ; curr: %s\n", list_entry(prev, element_t,
        // list)->value, list_entry(curr, element_t, list)->value);

        // Compare and find the space to insert the node by "galloping"
        // searching.
        while (curr && prev) {
            if (tim_cmp(priv, in_node, curr) > 0) {
                if (curr->next) {
                    if (curr->next->next) {
                        prev = curr->next;
                        curr = curr->next->next;
                        // printf("step 2 - prev: %s ; curr: %s\n",
                        // list_entry(prev, element_t, list)->value,
                        // list_entry(curr, element_t, list)->value);
                    } else {
                        prev = curr;
                        curr = curr->next;
                        // printf("step 1 - prev: %s ; curr: %s\n",
                        // list_entry(prev, element_t, list)->value,
                        // list_entry(curr, element_t, list)->value);
                    }
                } else {
                    prev = curr;
                    curr = NULL;
                    // printf("prev: %s\n", list_entry(prev, element_t,
                    // list)->value);
                    break;
                }
            } else {
                if (!(tim_cmp(priv, in_node, prev) > 0)) {
                    curr = prev;
                    prev = curr->prev;
                    // printf("backward\n");
                }
                // printf("* The else - prev: %s ; curr: %s\n", list_entry(prev,
                // element_t, list)->value, list_entry(curr, element_t,
                // list)->value);
                break;
            }
        }

        // insert to the list
        in_node->next = curr;
        in_node->prev = prev;
        prev->next = in_node;
        if (curr) {
            curr->prev = in_node;
            // printf("in_node output - prev: %s ; next: %s\n",
            // list_entry(in_node->prev , element_t, list)->value,
            // list_entry(in_node->next, element_t, list)->value);
        }  // else
           // printf("in_node output - prev: %s\n", list_entry(in_node->prev ,
           // element_t, list)->value);

        in_node = safe;
        next = in_node;
    }

    head->prev = NULL;
    head->next->prev = (struct list_head *) len;
    result.head = head, result.next = next;
    return result;
}

static struct list_head *merge_at(void *priv,
                                  list_cmp_func_t cmp,
                                  struct list_head *at)
{
    size_t len = run_size(at) + run_size(at->prev);
    struct list_head *prev = at->prev->prev;
    struct list_head *list = merge(priv, cmp, at->prev, at);
    list->prev = prev;
    list->next->prev = (struct list_head *) len;
    --stk_size;
    return list;
}

static struct list_head *merge_force_collapse(void *priv,
                                              list_cmp_func_t cmp,
                                              struct list_head *tp)
{
    while (stk_size >= 3) {
        if (run_size(tp->prev->prev) < run_size(tp)) {
            tp->prev = merge_at(priv, cmp, tp->prev);
        } else {
            tp = merge_at(priv, cmp, tp);
        }
    }
    return tp;
}

static struct list_head *merge_collapse(void *priv,
                                        list_cmp_func_t cmp,
                                        struct list_head *tp)
{
    int n;
    while ((n = stk_size) >= 2) {
        if ((n >= 3 &&
             run_size(tp->prev->prev) <= run_size(tp->prev) + run_size(tp)) ||
            (n >= 4 && run_size(tp->prev->prev->prev) <=
                           run_size(tp->prev->prev) + run_size(tp->prev))) {
            if (run_size(tp->prev->prev) < run_size(tp)) {
                tp->prev = merge_at(priv, cmp, tp->prev);
            } else {
                tp = merge_at(priv, cmp, tp);
            }
        } else if (run_size(tp->prev) <= run_size(tp)) {
            tp = merge_at(priv, cmp, tp);
        } else {
            break;
        }
    }

    return tp;
}

static int find_minrun(int size)
{
    int one = 0;
    if (size) {
        // To get the first five bits (MAX_MINRUN = 32)
        while (size > 0b11111) {
            one = (size & 0x01) ? 1 : one;  // holding carry
            size >>= 1;
        }
    }

    return size + one;
}

void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp)
{
    stk_size = 0;
    minrun = find_minrun(q_size(head));
    printf("len of min. run = %d\n", minrun);  // at max in 6 bits
    printf("q = %d ; r = %d\n", q_size(head) / minrun, q_size(head) % minrun);

    struct list_head *list = head->next, *tp = NULL;
    if (head == head->prev)
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        /* Find next run */
        struct pair result = find_run(priv, list, cmp);
        result.head->prev = tp;
        tp = result.head;
        list = result.next;
        stk_size++;
        tp = merge_collapse(priv, cmp, tp);
    } while (list);

    /* End of input; merge together all the runs. */
    tp = merge_force_collapse(priv, cmp, tp);

    /* The final merge; rebuild prev links */
    struct list_head *stk0 = tp, *stk1 = stk0->prev;
    while (stk1 && stk1->prev)
        stk0 = stk0->prev, stk1 = stk1->prev;
    if (stk_size <= 1) {
        build_prev_link(head, head, stk0);
        return;
    }
    merge_final(priv, cmp, head, stk1, stk0);
}


void q_timsort(void *priv, struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    timsort(priv, head, tim_cmp);
    if (descend)
        q_reverse(head);
}