#ifndef _C_COROUTINE_H_
#define _C_COROUTINE_H_

#ifndef __cplusplus
extern "C" {
#endif

#define DEFAULT_STACK_SIZE 1024*128
typedef enum {FREE, RUNNABLE, RUNNING, SUSPEND} TreadState;

typedef void (*Fun)(void *arg);

typedef struct uthread_t {
    ucontext_t ctx;
    Fun func;
    void *arg;
    enum TreadState state;
    char stack[DEFAULT_STACK_SIZE];
} uthread_t;







#ifndef __cplusplus
}
#endif

#endif
