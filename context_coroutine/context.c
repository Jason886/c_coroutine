#include <stdio.h>
#include <sys/ucontext.h>
#include <unistd.h>

int main(int argc, const char *argv[]) {
    ucontext_t context;

    getcontext(&context);
    puts("Hello wrold");
    sleep(1);
    setcontext(&context);
    return 0;
}
