#include <stdio.h>
int main(int argc, char* argv[]) {
    printf("Hello, World!\n");
    if (argc > 1) {
        for (int i = 1; i < argc; i++)
            printf("Arg[%d]: %s", i, argv[i]);
    }
}