#ifndef _C_COROUTINE_H_
#define _C_COROUTINE_H_

#ifndef __cplusplus
extern "C" {
#endif

struct schedule;
struct coroutine;

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

typedef void (*coroutine_func)(struct schedule *, void *ud);

struct schedule * coroutine_open(void);
void coroutine_close(struct schedule * S);
struct coroutine * coroutine_new(struct schedule *S, coroutine_func func, void *ud);
void coroutine_resume(struct schedule *S, struct coroutine * C);
void coroutine_yield(struct schedule *S);
struct coroutine *coroutine_running(struct schedule *S);
int coroutine_status(struct schedule *S, struct coroutine *C);

#ifndef __cplusplus
}
#endif

#endif
