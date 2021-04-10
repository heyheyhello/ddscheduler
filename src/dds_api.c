// Public API for the DDS

#include "dd_headers.h"

// From the lab manual
void create_dd_task(
    TaskHandle_t task_handle,
    DD_Task_Enum_t type,
    // I assume we're told to accept an ID as a way to overwrite a task? I'll
    // try to make this reuse memory, otherwise simply delete/re-create.
    uint32_t id,
    uint32_t absolute_deadline){};

// From the lab manual
void delete_dd_task(uint32_t id){};

// Modified from the lab manual. The recommended type signature is to return a
// double indirect pointer to the address of the start of the list, but I'm not
// enough of a C wizard to practice such pointer magic. Instead, dedicated list
// metadata structs are passed around. I argue this improves maintainability and
// accessibility of the code.

DD_LL_Leader_t *get_active_dd_task_list(void){};
DD_LL_Leader_t *get_overdue_dd_task_list(void){};
DD_LL_Leader_t *get_complete_dd_task_list(void){};
