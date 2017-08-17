#include "coroutine.h"
#include <stdio.h>

struct args {
    int n;
};

void foo(struct schedule *S, void *ud) {
    struct args *arg = ud;
    int start = arg->n;
    int i;
    for(i=0; i<5; i++) {
        printf("coroutine %p : %d\n", coroutine_running(S), start +i);
        if(start == 100) {
            coroutine_yield(S);
            printf("coroutine %p : %d another\n", coroutine_running(S), start +i);
            coroutine_yield(S); 
        }
        else {
            coroutine_yield(S); 
        }
    }
}

void test(struct schedule *S) {
    int i = 0;
    struct args arg1 = {0};
    struct args arg2 = {100};
    struct coroutine * co1 = coroutine_new(S, foo, &arg1);
    struct coroutine * co2 = coroutine_new(S, foo, &arg2);
    printf("main start\n");
    while(coroutine_status(S, co1) && coroutine_status(S, co2)) {
        coroutine_resume(S, co1);
        coroutine_resume(S, co2);
        i++;
    }
    printf("main end\n");
}

int main() {
    struct schedule *S = coroutine_open();
    test(S);
    coroutine_close(S);
    return 0;
}
