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

int q_cmp(void *priv, const struct list_head *a, const struct list_head *b)
{
    element_t *element_a = list_entry(a, element_t, list);
    element_t *element_b = list_entry(b, element_t, list);

    int res = strcmp(element_a->value, element_b->value);

    if (!res)
        return 0;

    if (priv)
        *((int *) priv) += 1;

    return res;
}

/* Merging two independent list */
struct list_head *merge_two_list(void *priv,
                                 struct list_head *left,
                                 struct list_head *right,
                                 bool descend,
                                 list_cmp_func_t cmp)
{
    // reference: 你所不知道的 C 語言: linked list 和非連續記憶體
    struct list_head *head = NULL;
    struct list_head **p = &head;
    for (; left && right; p = &((*p)->next)) {
        if ((!descend && cmp(priv, left, right) <= 0) ||
            (descend && cmp(priv, left, right) >= 0)) {
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
struct list_head *divide(void *priv,
                         struct list_head *head,
                         struct list_head *end,
                         bool descend,
                         list_cmp_func_t cmp)
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
    struct list_head *left = divide(priv, head, foreward, descend, cmp);
    struct list_head *right = divide(priv, backward, end, descend, cmp);
    // conquer the partitions
    return merge_two_list(priv, left, right, descend, cmp);
}

/* Sorting function for external program to call */
void sort(void *priv, struct list_head *head, list_cmp_func_t cmp)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;  // `head` is NULL, no list in `head`, or one element
    struct list_head *end = head->prev;
    // make the list no longer be circular
    end->next = NULL;
    head->next->prev = NULL;
    head->next = divide(priv, head->next, end, 0, cmp);
    // make the list to be circular again (move the head back)
    struct list_head *curr;
    for (curr = head; curr->next; curr = curr->next)
        curr->next->prev = curr;
    curr->next = head;
    curr->next->prev = curr;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    // merge sort mechanic
    if (!head || list_empty(head) || list_is_singular(head))
        return;  // `head` is NULL, no list in `head`, or one element
    struct list_head *end = head->prev;
    // make the list no longer be circular
    end->next = NULL;
    head->next->prev = NULL;
    head->next = divide(NULL, head->next, end, descend, q_cmp);
    // make the list to be circular again (move the head back)
    struct list_head *curr;
    for (curr = head; curr->next; curr = curr->next)
        curr->next->prev = curr;
    curr->next = head;
    curr->next->prev = curr;
}

/* Sort elements of queue in ascending/descending order by `list_sort.c` */
void q_list_sort(void *priv, struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;  // `head` is NULL, no list in `head`, or one element
    list_sort(priv, head, q_cmp);
    if (descend)
        q_reverse(head);
}

/* Sort elements of queue in ascending/descending order by Tim sort */
void q_timsort_old(void *priv, struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    timsort_old(priv, head, q_cmp);
    if (descend)
        q_reverse(head);
}

/* Sort elements of queue in ascending/descending order by Tim sort with minrun
 * (linear insertion sort strategy) implementation */
void q_timsort(void *priv, struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    timsort(priv, head, q_cmp);
    if (descend)
        q_reverse(head);
}

/* Sort elements of queue in ascending/descending order by Tim sort with minrun
 * (binary insertion sort strategy) implementation */
void q_timsort_binary(void *priv, struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    timsort_binary(priv, head, q_cmp);
    if (descend)
        q_reverse(head);
}

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
