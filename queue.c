#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (head) {
        // if head exists, clean the queue.
        element_t *iterator, *next;
        list_for_each_entry_safe (iterator, next, head, list) {
            list_del(&iterator->list);
            q_release_element(iterator);
        }
        free(head);
    }
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;  // the first node of inserted list_head is NULL
    element_t *new = (element_t *) malloc(sizeof(element_t));
    if (!new)
        return false;  // no memory space for `new`
    int s_len = strlen(s) + 1;
    new->value = (char *) malloc(s_len * sizeof(char));
    if (!new->value) {
        free(new);
        return false;  // no memory space for `new->value`
    }
    memcpy(new->value, s, s_len);  // insert value
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;  // the first node of inserted list_head is NULL
    element_t *new = (element_t *) malloc(sizeof(element_t));
    if (!new)
        return false;  // no memory space for `new`
    int s_len = strlen(s) + 1;
    new->value = (char *) malloc(s_len * sizeof(char));
    if (!new->value) {
        free(new);
        return false;  // no memory space for `new->value`
    }
    memcpy(new->value, s, s_len);  // insert value
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;  // `head` is NULL, or there's no list in `head`
    element_t *remove = list_first_entry(head, element_t, list);
    list_del(&remove->list);
    if (sp) {
        size_t q = bufsize > strlen(remove->value) + 1
                       ? strlen(remove->value) + 1
                       : bufsize;
        memcpy(sp, remove->value, q);
        sp[bufsize - 1] = '\0';
    }
    return remove;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;  // `head` is NULL, or there's no list in `head`
    element_t *remove = list_last_entry(head, element_t, list);
    list_del(&remove->list);
    if (sp) {
        size_t q = bufsize > strlen(remove->value) + 1
                       ? strlen(remove->value) + 1
                       : bufsize;
        memcpy(sp, remove->value, q);
        sp[bufsize - 1] = '\0';
    }
    return remove;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    int size = 0;
    struct list_head *p;
    list_for_each (p, head)
        size++;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;  // `head` is NULL, or there's no list in `head`
    /*if the foreward pointer hits the backward pointer, then they're in the
     * middle of the list*/
    struct list_head *foreward = head->next, *backward = head->prev;
    for (; foreward != backward && foreward->next != backward;
         foreward = foreward->next, backward = backward->prev)
        ;
    list_del(foreward);
    q_release_element(container_of(foreward, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;  // `head` is NULL, or there's no list in `head`
    element_t *iterator, *next;
    /*note that the list is sorted*/
    list_for_each_entry_safe (iterator, next, head, list) {
        if (&next->list != head && !strcmp(iterator->value, next->value)) {
            do {
                element_t *next_to_safe =
                    list_entry(next->list.next, element_t, list);
                list_del(&next->list);
                q_release_element(next);
                next = next_to_safe;
            } while (&next->list != head &&
                     !strcmp(iterator->value, next->value));
            list_del(&iterator->list);
            q_release_element(iterator);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;  // `head` is NULL, or there's no list in `head`
    struct list_head *curr = head->next, *next = curr->next;
    for (; curr != head && next != head; curr = curr->next, next = curr->next)
        list_move(curr, next);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;  // `head` is NULL, or there's no list in `head`
    struct list_head *iterator, *next;
    /*move each item the iterator points to to the head*/
    list_for_each_safe (iterator, next, head)
        list_move(iterator, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;  // `head` is NULL, or there's no list in `head`
    struct list_head *iterator, *next, *start = head, dummy;
    INIT_LIST_HEAD(
        &dummy);  // pointer dummy serves as same as pointer head does
    int i = 0;
    list_for_each_safe (iterator, next, head) {
        i++;
        if (i < k)
            continue;
        list_cut_position(&dummy, start,
                          iterator);  // cut k node of the list out as an
                                      // independent list to be reverse
        q_reverse(&dummy);
        list_splice_init(&dummy,
                         start);  // take dummy back to the original list
        start = next->prev;
        i = 0;
    }
}

/* Merging two independent list */
struct list_head *merge_two_list(struct list_head *left,
                                 struct list_head *right,
                                 bool descend)
{
    // reference: 你所不知道的 C 語言: linked list 和非連續記憶體
    struct list_head *head = NULL;
    struct list_head **p = &head;
    for (; left && right; p = &((*p)->next)) {
        element_t *l = list_entry(left, element_t, list);
        element_t *r = list_entry(right, element_t, list);
        if ((!descend && strcmp(l->value, r->value) < 0) ||
            (descend && strcmp(l->value, r->value) > 0)) {
            *p = left;
            left = left->next;
        } else {
            *p = right;
            right = right->next;
        }
    }
    *p = (struct list_head *) ((uintptr_t) left |
                               (uintptr_t) right);  // <stdint.h>
    return head;
}

/* Doing the list seperation for merge sort (divide and conquer) */
struct list_head *divide(struct list_head *head,
                         struct list_head *end,
                         bool descend)
{
    if (head == end)
        return head;
    // find middle point
    struct list_head *foreward = head, *backward = end;
    for (; foreward != backward && foreward->next != backward;
         foreward = foreward->next, backward = backward->prev)
        ;
    if (foreward == backward)
        backward = backward->next;
    // make the list no longer be circular
    foreward->next = NULL;
    backward->prev = NULL;
    // keep divide until hit the break point
    struct list_head *left = divide(head, foreward, descend);
    struct list_head *right = divide(backward, end, descend);
    // conquer the partitions
    return merge_two_list(left, right, descend);
}

/* Sort elements of queue in ascending/descending order */
void self_q_sort(struct list_head *head, bool descend)
{
    // merge sort machemic
    if (!head || list_empty(head) || list_is_singular(head))
        return;  // `head` is NULL, no list in `head`, or one element
    struct list_head *end = head->prev;
    // make the list no longer be circular
    end->next = NULL;
    head->next->prev = NULL;
    head->next = divide(head->next, end, descend);
    // make the list to be circular again (move the head back)
    struct list_head *curr;
    for (curr = head; curr->next; curr = curr->next)
        curr->next->prev = curr;
    curr->next = head;
    curr->next->prev = curr;
}

/* Start of the list_sort */

/** The definition of `likely` and `unlikely` from <linux/compiler.h>
 *
 * @bug These definitions encountered an error outside the Linux kernel
 * environment. Therefore, I replaced them with `__glibc_likely` and
 * `__glibc_unlikely`, respectively, which originate from gcc's
 * `__builtin_expect` to enhance optimization.
 *        - Error code: implicit declaration of function ‘__branch_check__’
 *
 * #ifndef likely
 * #define likely(x) (__branch_check__(x, 1, __builtin_constant_p(x)))
 * #endif
 * #ifndef unlikely
 * #define unlikely(x) (__branch_check__(x, 0, __builtin_constant_p(x)))
 * #endif
 */

typedef int (*list_cmp_func_t)(const struct list_head *,
                               const struct list_head *);

int list_cmp(const struct list_head *a, const struct list_head *b)
{
    element_t *element_a = list_entry(a, element_t, list);
    element_t *element_b = list_entry(b, element_t, list);

    return strcmp(element_a->value, element_b->value);
}

/*
 * Returns a list organized in an intermediate format suited
 * to chaining of merge() calls: null-terminated, no reserved or
 * sentinel head node, "prev" links not maintained.
 */
static struct list_head *merge(list_cmp_func_t cmp,
                               struct list_head *a,
                               struct list_head *b)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
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

/*
 * Combine final list merge with restoration of standard doubly-linked
 * list structure.  This approach duplicates code from merge(), but
 * runs faster than the tidier alternatives of either a separate final
 * prev-link restoration pass, or maintaining the prev links
 * throughout.
 */
static void merge_final(list_cmp_func_t cmp,
                        struct list_head *head,
                        struct list_head *a,
                        struct list_head *b)
{
    struct list_head *tail = head;
    uint8_t count = 0; /* change the data type to `uint8_t` from <stdint.h>
                        * rather that origin data type `u8` defined in
                        * <linux/type.h>
                        */

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
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
    tail->next = b;
    do {
        /*
         * If the merge is highly unbalanced (e.g. the input is
         * already sorted), this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if (__glibc_unlikely(!++count)) /* from gcc __builtin_except  */
            cmp(b, b);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * The comparison function @cmp must return > 0 if @a should sort after
 * @b ("@a > @b" if you want an ascending sort), and <= 0 if @a should
 * sort before @b *or* their original order should be preserved.  It is
 * always called with the element that came first in the input in @a,
 * and list_sort is a stable sort, so it is not necessary to distinguish
 * the @a < @b and @a == @b cases.
 *
 * This is compatible with two styles of @cmp function:
 * - The traditional style which returns <0 / =0 / >0, or
 * - Returning a boolean 0/1.
 * The latter offers a chance to save a few cycles in the comparison
 * (which is used by e.g. plug_ctx_cmp() in block/blk-mq.c).
 *
 * A good way to write a multi-word comparison is::
 *
 *	if (a->high != b->high)
 *		return a->high > b->high;
 *	if (a->middle != b->middle)
 *		return a->middle > b->middle;
 *	return a->low > b->low;
 *
 *
 * This mergesort is as eager as possible while always performing at least
 * 2:1 balanced merges.  Given two pending sublists of size 2^k, they are
 * merged to a size-2^(k+1) list as soon as we have 2^k following elements.
 *
 * Thus, it will avoid cache thrashing as long as 3*2^k elements can
 * fit into the cache.  Not quite as good as a fully-eager bottom-up
 * mergesort, but it does use 0.2*n fewer comparisons, so is faster in
 * the common case that everything fits into L1.
 *
 *
 * The merging is controlled by "count", the number of elements in the
 * pending lists.  This is beautifully simple code, but rather subtle.
 *
 * Each time we increment "count", we set one bit (bit k) and clear
 * bits k-1 .. 0.  Each time this happens (except the very first time
 * for each bit, when count increments to 2^k), we merge two lists of
 * size 2^k into one list of size 2^(k+1).
 *
 * This merge happens exactly when the count reaches an odd multiple of
 * 2^k, which is when we have 2^k elements pending in smaller lists,
 * so it's safe to merge away two lists of size 2^k.
 *
 * After this happens twice, we have created two lists of size 2^(k+1),
 * which will be merged into a list of size 2^(k+2) before we create
 * a third list of size 2^(k+1), so there are never more than two pending.
 *
 * The number of pending lists of size 2^k is determined by the
 * state of bit k of "count" plus two extra pieces of information:
 *
 * - The state of bit k-1 (when k == 0, consider bit -1 always set), and
 * - Whether the higher-order bits are zero or non-zero (i.e.
 *   is count >= 2^(k+1)).
 *
 * There are six states we distinguish.  "x" represents some arbitrary
 * bits, and "y" represents some arbitrary non-zero bits:
 * 0:  00x: 0 pending of size 2^k;           x pending of sizes < 2^k
 * 1:  01x: 0 pending of size 2^k; 2^(k-1) + x pending of sizes < 2^k
 * 2: x10x: 0 pending of size 2^k; 2^k     + x pending of sizes < 2^k
 * 3: x11x: 1 pending of size 2^k; 2^(k-1) + x pending of sizes < 2^k
 * 4: y00x: 1 pending of size 2^k; 2^k     + x pending of sizes < 2^k
 * 5: y01x: 2 pending of size 2^k; 2^(k-1) + x pending of sizes < 2^k
 * (merge and loop back to state 2)
 *
 * We gain lists of size 2^k in the 2->3 and 4->5 transitions (because
 * bit k-1 is set while the more significant bits are non-zero) and
 * merge them away in the 5->2 transition.  Note in particular that just
 * before the 5->2 transition, all lower-order bits are 11 (state 3),
 * so there is one list of each smaller size.
 *
 * When we reach the end of the input, we merge all the pending
 * lists, from smallest to largest.  If you work through cases 2 to
 * 5 above, you can see that the number of elements we merge with a list
 * of size 2^k varies from 2^(k-1) (cases 3 and 5 when x == 0) to
 * 2^(k+1) - 1 (second merge of case 5 when x == 2^(k-1) - 1).
 */
void list_sort(struct list_head *head, list_cmp_func_t cmp)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    /*
     * Data structure invariants:
     * - All lists are singly linked and null-terminated; prev
     *   pointers are not maintained.
     * - pending is a prev-linked "list of lists" of sorted
     *   sublists awaiting further merging.
     * - Each of the sorted sublists is power-of-two in size.
     * - Sublists are sorted by size and age, smallest & newest at front.
     * - There are zero to two sublists of each size.
     * - A pair of pending sublists are merged as soon as the number
     *   of following pending elements equals their size (i.e.
     *   each time count reaches an odd multiple of that size).
     *   That ensures each later final merge will be at worst 2:1.
     * - Each round consists of:
     *   - Merging the two sublists selected by the highest bit
     *     which flips when count is incremented, and
     *   - Adding an element from the input as a size-1 sublist.
     */
    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (__glibc_likely(bits)) { /* from gcc __builtin_except  */
            struct list_head *a = *tail, *b = a->prev;

            a = merge(cmp, b, a);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(cmp, pending, list);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(cmp, head, pending, list);
}

/* Sort elements of queue in ascending/descending order by `list_sort.c` */
void q_sort(struct list_head *head, bool descend)
{
    // merge sort machemic
    if (!head || list_empty(head) || list_is_singular(head))
        return;  // `head` is NULL, no list in `head`, or one element
    list_sort(head, list_cmp);
    if (descend)
        q_reverse(head);
}

/* End of the list_sort */

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;  // `head` is NULL, or there's no list in `head`
    if (list_is_singular(head))
        return 1;
    // this section cares about value in 'element_t' structure
    struct list_head *curr = head->next;
    element_t *p, *c_max = list_entry(curr, element_t, list);
    for (; c_max->list.prev != head;) {
        p = list_entry(c_max->list.next, element_t, list);
        if (strcmp(p->value, c_max->value) < 0) {
            list_del(&p->list);
            q_release_element(p);
        } else
            c_max = p;
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;  // `head` is NULL, or there's no list in `head`
    if (list_is_singular(head))
        return 1;
    // this section cares about value in 'element_t' structure
    struct list_head *curr = head->prev;
    element_t *p, *c_max = list_entry(curr, element_t, list);
    for (; c_max->list.prev != head;) {
        p = list_entry(c_max->list.prev, element_t, list);
        if (strcmp(p->value, c_max->value) < 0) {
            list_del(&p->list);
            q_release_element(p);
        } else
            c_max = p;
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;  // `head` is NULL, or there's no list in `head`
    if (list_is_singular(head))
        return list_entry(head->next, queue_contex_t, chain)->size;
    // merging by 'q_context_t' structure
    queue_contex_t *main_q = list_entry(head->next, queue_contex_t, chain);
    // 分段合併
    for (struct list_head *curr = head->next->next; curr != head;
         curr = curr->next) {
        queue_contex_t *c = list_entry(curr, queue_contex_t, chain);
        list_splice_init(c->q, main_q->q);
        main_q->size += c->size;
        c->size = 0;
    }
    // sorting
    q_sort(main_q->q, descend);
    return main_q->size;
}
