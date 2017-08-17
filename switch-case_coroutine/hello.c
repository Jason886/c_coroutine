#include <stdio.h>
#define crBegin static int state = 0; switch(state) {case 0:
#define crReturn(x) do {state=__LINE__;return x; case __LINE__:;}while(0)
#define crFinish }

int function(void) {
    crBegin;
    crReturn(1);
    crReturn(1);
    crReturn(0);
    crFinish;
    return 0;
}



int main() {
    for(int j=0; j< 30; j++) {
        int val = function();
        printf("val = %d\n", val);
    }
}
