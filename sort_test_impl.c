#include <stdint.h>

#include "queue.h"
#include "sort_test_impl.h"

/* Merging two independent list (different from the implementation in `queue.c`)
 */
struct list_head *merge_list(struct list_head *left, struct list_head *right)
{
    struct list_head *head = NULL;
    struct list_head **p = &head;
    for (int i = 0; left && right; p = &((*p)->next), i++) {
        if (i % 2) {
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

/* the shuffle algorithm introduced by Fisher–Yates */
void shuffle(struct list_head *head)
{
    int len = q_size(head);
    struct list_head *pos, *safe;
    /* similar as `list_for_each_entry_safe` in Linux Kernel List Management API
     */
    for (pos = head->next, safe = pos->next; pos != head && len;
         pos = safe, safe = safe->next, len--) {
        struct list_head *j = head->next;
        /* randomly choose a number between 0~(len - 1) */
        for (int r = rand() % len; r;
             r--) /* the seed `srand` had been defined in qtest*/
            j = j->next;
        if (pos == j)
            continue;
        list_swap(pos, j); /* from Linux Kernel List Management API */
    }
}

/**
 * A function to join the split list with the guarantee of the `next` pointer
 * in the `struct list_head` structure.
 */
static struct list_head *worst_merge_join(struct list_head *left_head,
                                          struct list_head *right_head)
{
    struct list_head *head = NULL;
    struct list_head **p = &head;
    for (; left_head; p = &((*p)->next), left_head = left_head->next)
        *p = left_head;
    for (; right_head; p = &((*p)->next), right_head = right_head->next)
        *p = right_head;
    return head;
}

/**
 * A function to reorganize a continious string list to the worst case of merge
 * sort with bottom-up recursive call implementation.
 */
static struct list_head *worst_merge_split(struct list_head *head)
{
    if (head != NULL && head->next != NULL) {
        struct list_head *left_head = NULL, *right_head = NULL;
        // Find the left_head and right_head
        struct list_head *curr = head;
        struct list_head **pl = &left_head;
        struct list_head **pr = &right_head;
        // apply the code with pointer of pointer
        for (int count = 1; curr != NULL; curr = curr->next, count++) {
            if (count % 2) {  // odd case
                *pl = curr;
                pl = &((*pl)->next);
            } else {  // even case
                *pr = curr;
                pr = &((*pr)->next);
            }
        }

        *pl = NULL;
        *pr = NULL;

        // Recursive split
        left_head = worst_merge_split(left_head);
        right_head = worst_merge_split(right_head);
        // List joining
        return worst_merge_join(left_head, right_head);
    }

    return head;
}

void worst_case_generator(struct list_head *head)
{
    struct list_head *end = head->prev;
    // make the list no longer be circular
    end->next = NULL;
    head->next->prev = NULL;
    // reconstruct the sorted list to the worst case scenario
    head->next = worst_merge_split(head->next);
    // make the list be circular again
    struct list_head *curr;
    for (curr = head; curr->next; curr = curr->next)
        curr->next->prev = curr;
    curr->next = head;
    curr->next->prev = curr;
}

/* The following code generates another form of the in-theory worst case for
merge sort in the discussion */
// void worst_case_generator(struct list_head *head) {
//     int x = 0, y = size(head);
//     int middle = (x & y) + ((x ^ y) >> 1);

//     struct list_head *head = current->q;
//     struct list_head *iterator; int i = 0;
//     list_for_each(iterator, head){
//         if (i == middle)
//             break;
//         i++;
//     }

//     /* reverse the latter half of the list */
//     head->prev->next = iterator;
//     iterator->prev->next = NULL;
//     iterator->prev = head->prev;
//     q_reverse(iterator);
//     /* update the head of the latter half of the list */
//     head->prev = iterator;
//     iterator = iterator->next;

//     /* merging two list */
//     head->next->prev = NULL;
//     head->prev->next = NULL;
//     iterator->prev = NULL;
//     head->next = merge_list(head->next, iterator);
//     /* rebuild the `prev` pointer */
//     struct list_head *curr;
//     for (curr = head; curr->next; curr = curr->next)
//         curr->next->prev = curr;
//     curr->next = head;
//     curr->next->prev = curr;
// }