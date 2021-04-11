#include "dd_headers.h"

// Wrote this linked list in SENG265...

DD_LL_Leader_t *ll_leader()
{
  DD_LL_Leader_t *leader = (DD_LL_Leader_t *)malloc(sizeof(DD_LL_Leader_t));
  if (!leader)
  {
    printf("Error trying to malloc in ll_leader");
    return NULL;
  }

  leader->head = NULL;
  leader->cursor = NULL;
  leader->cursor_prev = NULL;
  leader->length = 0;
  return leader;
}

DD_LL_Node_t *ll_node(DD_Task_t *dd_task)
{
  DD_LL_Node_t *item = (DD_LL_Node_t *)malloc(sizeof(DD_LL_Node_t));
  if (!item)
  {
    printf("Error trying to malloc in ll_node");
    return NULL;
  }

  item->task = dd_task;
  item->next = NULL;
  return item;
}

void ll_next(DD_LL_Leader_t *leader)
{
  if (leader->cursor == NULL)
    return;
  leader->cursor_prev = leader->cursor;
  leader->cursor = leader->cursor->next;
}

// Don't use these. Always use a cursor...

// void ll_prepend(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add) {
//   to_add->next = leader->head;
//   leader->head = to_add;
//   leader->length++;
// }

// void ll_append(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add) {
//   if (leader->head == NULL) {
//     leader->head = to_add;
//   } else {
//     /* Move to end of list */
//     DD_LL_Node_t *runner = leader->head;
//     for (; runner->next; runner = runner->next) {}
//     runner->next = to_add;
//   }
//   leader->length++;
// }

void ll_cursor_prepend(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add)
{
  if (leader->cursor == NULL)
  {
    printf("ll_cursor_prepend cursor is NULL\n");
    free(to_add);
    return;
  }
  to_add->next = leader->cursor;
  if (leader->cursor == leader->head)
  {
    /* leader->cursor_prev would be null so ->next would crash */
    leader->head = to_add;
  }
  else
  {
    leader->cursor_prev->next = to_add;
  }
  leader->cursor_prev = to_add;
  leader->length++;
}

void ll_cursor_append(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add)
{
  if (leader->cursor == NULL)
  {
    printf("ll_cursor_append cursor is NULL\n");
    free(to_add);
    return;
  }
  to_add->next = leader->cursor->next;
  leader->cursor->next = to_add;
  leader->length++;
}

DD_LL_Node_t *ll_cursor_unlink(DD_LL_Leader_t *leader)
{
  if (leader->cursor == NULL)
  {
    printf("ll_cursor_unlink cursor is NULL\n");
    return NULL;
  }
  if (leader->cursor == leader->head)
  {
    /* leader->cursor_prev would be null so ->next would crash */
    leader->head = leader->cursor->next;
  }
  else
  {
    leader->cursor_prev->next = leader->cursor->next;
  }
  leader->cursor->next = NULL;
  DD_LL_Node_t *item = leader->cursor;
  // Don't make an assumption of what the cursor should point at next. They have
  // the prev_cursor to reset the cursor if they need ot unlink in a loop.
  leader->cursor = NULL;
  leader->length--;
  return item;
}

void ll_print(DD_LL_Leader_t *leader)
{
  if (leader == NULL)
  {
    printf("ll_print leader is NULL\n");
    return;
  }
  DD_LL_Node_t *runner = leader->head;
  DD_Task_t *task;

  printf("LL: list: items: %d\n", leader->length);
  for (; runner; runner = runner->next)
  {
    task = runner->task;
    printf("ll_print task: '%s'", task->id);
    if (runner == leader->cursor)
      printf(" < cursor");
    printf("\n");
  }
  printf("\n");
}

// This is more efficient that looping ll_cursor_unlink
void ll_empty(DD_LL_Leader_t *leader)
{
  leader->cursor = leader->head;
  leader->cursor_prev = NULL;
  while (leader->length)
  {
    ll_next(leader);
    free(leader->cursor_prev);
    leader->length--;
  }
  leader->cursor = leader->head = leader->cursor_prev = NULL;
}
