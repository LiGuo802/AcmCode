#include <stdio.h>

int main() {
    int i;
    while(scanf("%d", &i) == 1) {
        if(i % 2 == 0) {
            printf("%d\n\n", i / 2 * (i + 1) );
        } else {
            printf("%d\n\n", (i + 1) / 2 * i );
        }
    }
    return 0;
}
