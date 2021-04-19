// Public API for the DDS

#include "dd_headers.h"

// -----------------------------------------------------------------------------

void create_dd_task(
    // If they want us to accept a TaskHandle_t then I'm not calling xTaskCreate
    // from in the DDS - the caller must have done xTaskCreate and xTaskSuspend
    TaskHandle_t task_handle,
    //
    DD_Task_Enum_t type,
    // I assume we're told to accept an ID as a way to overwrite a task? I'll
    // try to make this reuse memory, otherwise simply delete/re-create. Is this
    // to see if we're overdue for periodic tasks? High absolute deadlines would
    // allow multiple of the same ID to be scheduled, so, error?
    uint32_t id,
    //
    uint32_t absolute_deadline);

// From the lab manual. Still, I worry accepting a task handle is bad API design
// since it's too easy for callers to hand either a running or suspended task.
// I'll have to suspend either way, and this also wastes time on the FreeRTOS
// scheduler. I think xTaskFunction would be better.

// -----------------------------------------------------------------------------

void delete_dd_task(uint32_t id);

// From the lab manual. No comment.

// -----------------------------------------------------------------------------

DD_LL_Leader_t *get_active_dd_task_list(void);
DD_LL_Leader_t *get_overdue_dd_task_list(void);
DD_LL_Leader_t *get_complete_dd_task_list(void);

// These aren't from the lab manual. The recommended type signature is to return
// a double indirect pointer to the address of the start of the list, but I'm
// not enough of a C wizard to practice that pointer magic. Instead, I pass
// dedicated list metadata structs around. This is easier to understand.

// Notes:

// First looking at this, it doesn't make sense as an API since the calls are
// async so how am I supposed to return a pointer? It'd be NULL until the
// promise resolves.

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

// OK... Needed to realize that this isn't an event loop microtask queue
// architecture; all good.
