#ifndef _C_COROUTINE_H_
#define _C_COROUTINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

struct schedule;
struct coroutine;
typedef void (*coroutine_func)(struct schedule *S, void *ud);

struct schedule * coroutine_open();
void coroutine_close(struct schedule * S);
struct coroutine * coroutine_new(struct schedule *S, coroutine_func func, void *ud);
void coroutine_resume(struct schedule *S, struct coroutine * C);
void coroutine_yield(struct schedule *S);
struct coroutine *coroutine_running(struct schedule *S);
int coroutine_status(struct schedule *S, struct coroutine *C);


#ifdef __cplusplus
}
#endif

#endif
