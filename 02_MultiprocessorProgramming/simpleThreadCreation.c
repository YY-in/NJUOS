#include "thread.h"

void *mythreadA(){
    printf("A\n");
    return NULL;
}
void *mythreadB(){
    printf("B\n");
    return NULL;
}
int main(int argc, char const *argv[])
{
    create(mythreadA);
    create(mythreadB);
    join();
    return 0;
}
