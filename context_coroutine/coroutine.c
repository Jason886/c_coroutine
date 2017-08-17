#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <sys/ucontext.h>
#include "coroutine.h"

#define _STACK_SIZE (1024*128) 

struct coroutine {
    ucontext_t ctx;
    char stack[_STACK_SIZE];
    coroutine_func func;
    void *ud;
    int status;
    struct coroutine * next;
};

struct schedule {
    ucontext_t main;
    /*char stack[_STACK_SIZE]; */
    struct coroutine * running;
    struct coroutine * head;
};

static struct coroutine * 
_co_new(struct schedule *S, coroutine_func func, void *ud) {
    struct coroutine * co = malloc(sizeof(*co));
    if(co) {
        memset(co, 0x00, sizeof(*co));
        co->func = func;
        co->ud = ud;
        co->status = COROUTINE_READY;
    }
    return co;
}

static void
_co_delete(struct coroutine *C) {
    assert(C);
    free(C);
}

struct schedule * 
coroutine_open() {
    struct schedule * S = 0;
    S = malloc(sizeof(*S));
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
        _co_delete(q);
    }
    free(S);
}

struct coroutine * 
coroutine_new(struct schedule *S, coroutine_func func, void *ud) {
    struct coroutine *co = 0, *p;
    assert(S);
    co = _co_new(S, func, ud);
    if(co) {
        p = S->head;
        while(p && p->next) {
            p = p->next;
        }
        if(p) {
            p->next = co;
        }
        else {
            S->head = co;
        }
    }
    return co;
}

static void
mainfunc(uint32_t low32, uint32_t hi32) {
    struct coroutine *p, *q;
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    struct schedule *S = (struct schedule *)ptr;
    struct coroutine *C = S->running;

    C->func(S, C->ud);

    if(S->head == C) {
        q = S->head;
        S->head = q->next;
        _co_delete(q);
    }
    else {
        p = S->head;
        while(p && p->next != C) {
            p = p->next;
        }
        if(p) {
            q = p->next;
            p->next = q->next;
            _co_delete(q);
        }
    }
    S->running = 0;
}

void
coroutine_resume(struct schedule *S, struct coroutine * C) {
    struct coroutine *p;
    uintptr_t ptr = 0;
    int status;

    assert(S);
    assert(C);
    assert(!S->running);

    p = S->head;
    while(p && p!=C) {
        p = p->next;
    }
    if(!p) {
        assert(0);
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
        ptr = (uintptr_t)S;
        makecontext(&C->ctx, (void (*)(void))mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        C->status = COROUTINE_RUNNING;
        S->running = C;
        swapcontext(&S->main, &C->ctx);
        break;
    case COROUTINE_SUSPEND:
        C->status = COROUTINE_RUNNING;
        S->running = C;
        swapcontext(&S->main, &C->ctx);
        break;
    default:
        assert(0);
    }
}

void
coroutine_yield(struct schedule *S) {
    struct coroutine * C = 0;
    assert(S);
    assert(S->running);
    C = S->running;
    S->running = NULL;
    C->status = COROUTINE_SUSPEND;
    swapcontext(&C->ctx, &S->main);
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
