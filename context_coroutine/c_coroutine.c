#include "c_coroutine.h"

#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

#define _STACK_SIZE (1024*128) /* 1M bytes */

struct coroutine {
    struct coroutine * next;
    char stack[_STACK_SIZE];
    ucontext_t ctx;
    coroutine_func func;
    void *ud;
    int status;
};

struct schedule {
    char stack[_STACK_SIZE];
    ucontext_t main;
    struct coroutine * running;
    struct coroutine * head;
};


static struct coroutine * 
_co_new(struct schedule *S, coroutine_func func, void *ud) {
    struct coroutine * co = 0;
    co = malloc(sizeof(*co));
    if(co) {
        memset(co, 0x00, sizeof(*co));
        co->func = func;
        co->ud = ud;
        co->status = COROUTINE_READY;
    }
    return co
}

struct schedule * 
coroutine_open(void) {
    struct schedule * S = 0;
    s = malloc(sizeof(*S));
    memset(S, 0x00, sizeof(*S));
    return S;
}

void 
coroutine_close(struct schedule * S) {
    struct coroutine *p, *q;
    assert(S);
    p = S->head;
    while(p) {
        q = p;
        p = p->next;
        free(q);
    }
    free(S);
}

struct coroutine * 
coroutine_new(struct schedule *S, coroutine_func func, void *ud) {
    struct coroutine *co = 0, *p;
    assert(S);
    co = _co_new(S, func, ud);
    if(co) {
        if(!S->head) {
            S->head = co;
        }
        else {
            p = S->head;
            while(p->next) {
                p = p->next;
            }
            p->next = co;
        }
    }
    return co
}

static void
mainfunc(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr)hi32 << 32);
    struct schedule *S = (struct schedule *)ptr;
    struct coroutine_t * C = S->running;
    C->func(S, C->ud);
    struct schedule *p, *q;
    if(S->head == C) {
        free(S->head);
        S->head = NULL;
    }
    else {
        p = S->head;
        while(p && p->next != C) {
            p = p->next;
        }
        if(p) {
            q = p->next;
            p->next = q->next;
            free(q);
        }
    }
    S->running = 0;
}

void
coroutine_resume(struct schedule *S, struct coroutine * C) {
    struct coroutine *p;
    int status;

    assert(S);
    assert(C);
    assert(!S->running);

    p = S->head;
    while(p) {
        if(p == C) {
            break;
        }
        p = p->next;
    }
    assert(p);
    if(!p) {
        return;
    }
    
    status = C->status;
    switch(status) {
    case COROUTINE_READY:
        getcontext(&C->ctx);
        C->ctx.uc_stack.ss_sp = C->stack;
        C->ctx.uc_stack.ss_size = _STACK_SIZE;
        C->ctx.uc_stack.ss_flags = 0;
        C->ctx.uc_link = &S->main;
        C->status = COROUTINE_RUNNING;
        S->running = C;
        uintptr_t ptr = (uintptr_t)S;
        makecontext(&C->ctx, (void (*)(void))mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        swapcontext(&S->main, &C->ctx);
        break;
    case COROUTINE_SUSPEND:
        C->status = COROUTINE_RUNNING;
        S->running = C;
        wapcontext(&S->main, &C->ctx);
        break;
    default:
        assert(0);
    }
}

void
coroutine_yield(struct schedule *S) {
    struct Coroutine * C = 0;
    assert(S);
    assert(S->running);
    C = S->running;
    if(C->status == COROUTINE_RUNNING) {
        S->running = NULL;
        C->status = COROUTINE_SUSPEND;
        wapcontext(&C->ctx, &S->main);
    }
    else {
        assert(0);
    }
}

struct coroutine *
coroutine_running(struct schedule *S) {
    assert(S);
    return S->running;
}

int
coroutine_status(struct schedule *S, struct coroutine *C) {
    struct coroutine *p;
    assert(S);
    assert(C);
    if(S->head) {
        p = S->head;
        while(p && p != C) {
            p = p->next;
        }
        if(p) {
            return p->status;
        }
    }
    return COROUTINE_DEAD;
}
