#include "dd_headers.h"

// Wrote this linked list in SENG265...

DD_LL_Leader_t *ll_leader() {
  DD_LL_Leader_t *leader =
      (DD_LL_Leader_t *)pvPortMalloc(sizeof(DD_LL_Leader_t));
  leader->head = NULL;
  leader->cursor = NULL;
  leader->cursor_prev = NULL;
  leader->length = 0;
  return leader;
}

DD_LL_Node_t *ll_node(DD_Task_t *dd_task) {
  DD_LL_Node_t *item = (DD_LL_Node_t *)pvPortMalloc(sizeof(DD_LL_Node_t));
  item->task = dd_task;
  item->next = NULL;
  return item;
}

void ll_cur_head(DD_LL_Leader_t *leader) {
  leader->cursor = leader->head;
  leader->cursor_prev = NULL;
}

void ll_cur_next(DD_LL_Leader_t *leader) {
  if (leader->cursor == NULL)
    return;
  leader->cursor_prev = leader->cursor;
  leader->cursor = leader->cursor->next;
}

void ll_cur_tail(DD_LL_Leader_t *leader) {
  for (ll_cur_head(leader); leader->cursor->next; ll_cur_next(leader))
    ;
}

void ll_cur_prepend(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add) {
  if (leader->cursor == NULL) {
    printf("ll_cur_prepend cursor is NULL\n");
    return;
  }
  to_add->next = leader->cursor;
  if (leader->cursor == leader->head) {
    // leader->cursor_prev would be null so ->next would crash
    leader->head = to_add;
  } else {
    if (leader->cursor_prev->next != leader->cursor)
      printf("bug: ll_cur_prepend cursor_prev isn't previous of cursor");
    leader->cursor_prev->next = to_add;
  }
  leader->cursor_prev = to_add;
  leader->length++;
}

void ll_cur_append(DD_LL_Leader_t *leader, DD_LL_Node_t *to_add) {
  if (leader->cursor == NULL) {
    printf("bug: ll_cur_append cursor is NULL\n");
    return;
  }
  to_add->next = leader->cursor->next;
  leader->cursor->next = to_add;
  leader->length++;
}

DD_LL_Node_t *ll_cur_unlink(DD_LL_Leader_t *leader) {
  if (leader->cursor == NULL) {
    printf("bug: ll_cur_unlink cursor is NULL\n");
    return NULL;
  }
  if (leader->cursor == leader->head) {
    // leader->cursor_prev would be null so ->next would crash
    leader->head = leader->cursor->next;
  } else {
    if (leader->cursor_prev->next != leader->cursor)
      printf("bug: ll_cur_unlink cursor_prev isn't previous of cursor");
    leader->cursor_prev->next = leader->cursor->next;
  }
  leader->cursor->next = NULL;
  DD_LL_Node_t *item = leader->cursor;
  // Don't make an assumption of what the cursor should point at next. They have
  // the prev_cursor to reset the cursor if they need to unlink in a loop.
  leader->cursor = NULL;
  leader->length--;
  return item;
}

void ll_print(DD_LL_Leader_t *leader) {
  if (leader == NULL) {
    printf("bug: ll_print leader is NULL\n");
    return;
  }
  DD_LL_Node_t *runner = leader->head;
  DD_Task_t *task;

  printf("ll_print items: %d\n", leader->length);
  for (; runner; runner = runner->next) {
    task = runner->task;
    printf("ll_print task: '%s'", task->id);
    if (runner == leader->cursor)
      printf(" < cursor");
    printf("\n");
  }
  printf("\n");
}

// This is more efficient that looping ll_cur_unlink
void ll_empty(DD_LL_Leader_t *leader) {
  ll_cur_head(leader);
  while (leader->length) {
    ll_cur_next(leader);
    vPortFree(leader->cursor_prev);
    leader->length--;
  }
  leader->cursor = leader->head = leader->cursor_prev = NULL;
}
