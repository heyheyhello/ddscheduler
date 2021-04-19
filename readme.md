# FreeRTOS Deadline Driven Scheduler

Project 3 for UVic ECE 455 2021

This defines a scheduler for periodic tasks which is implemented as a FreeRTOS
priority-based task (F-Task). Every tasks is actually scheduled as an F-Task but
the DDS is able to re-prioritize the tasks based on their deadlines. A task is
actually a struct of metadata `DD_Task_t`.

The scheduler task (dd_ftask_scheduler.c) runs as the highest priority process
on the system. Next is the monitor task (dd_ftask_monitor.c) which periodically
uses the DDS' public API functions to get information about how many tasks are
currently active, overdue, and completed. Tasks are stored across three linked
lists (linked_list.c). Communication to/from the DDS is done by passing messages
on two queues `qh_request` and `qh_response` which I based this on webservers
that use a similar req/res model. The `dd_api_call()` private function does
message passing. Structs and headers for everything are in dd_headers.h but
the DDS's public API is dds_api.h and test benches defined in the lab report are
in dd_tests.h.

Since I create and delete (aka not reuse) FreeRTOS tasks, DD task structs, and
DD message structs, there's _a lot_ of allocating and freeing memory. I used
heap_4.c as the heap (included in main.c) to help with this and bumped the heap
size to 30kb in the FreeRTOSConfig.h.

It works 🌺
