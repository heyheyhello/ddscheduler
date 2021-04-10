// Public API for the DDS

#include "dd_headers.h"

// From the lab manual
void create_dd_task(
    TaskHandle_t task_handle,
    DD_Task_Enum_t type,
    // I assume we're told to accept an ID as a way to overwrite a task? I'll
    // try to make this reuse memory, otherwise simply delete/re-create.
    uint32_t id,
    uint32_t absolute_deadline);

// From the lab manual
void delete_dd_task(uint32_t id);

// Modified from the lab manual. The recommended type signature is to return a
// double indirect pointer to the address of the start of the list, but I'm not
// enough of a C wizard to practice such pointer magic. Instead, dedicated list
// metadata structs are passed around. I argue this improves maintainability and
// accessibility of the code.

// Wait No? This doesn't make sense as an API. The API calls are async so how am
// I supposed to return a pointer - it'll be NULL until the promise resolves.

// Either a) the DDS will need to have an outbox queue, and a caller will need
// to poll that queue (or add itself to the queue) before calling the API or b)
// the API needs to take a queue handle as a parameter so the DDS knows the
// caller to return the message to; this means the API returns void. Option a)
// is awful because it relies on being at the right at the right time.

// Hmm... It's possible that F-Task priorities will actually make it possible.
// When a caller uses get*dd_task_list() they send a message to a queue and then
// they _suspend_. It's not like a JS promises which use a microtask queue; it's
// immediately suspended as soon as the message arrives in the DDS' queue. This
// means the get_*_dd call can have its own queue for the message payload, and
// the DDS will return to _that_ instead of directly to the caller. The DDS then
// yields and get_*_dd resumes (as part of the caller F-Task), reads its queue,
// resolves the pointer, and gives it back to the caller.

// OK. Needed to realize that this isn't an event loop microtask queue
// architecture haha

DD_LL_Leader_t *get_active_dd_task_list(void);
DD_LL_Leader_t *get_overdue_dd_task_list(void);
DD_LL_Leader_t *get_complete_dd_task_list(void);
