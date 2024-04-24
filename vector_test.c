#include<stdio.h>
#include"vector.h"

int main(){
    struct Vector vec;
    vectorInit(&vec, 4);
    vectorPush(&vec, (void*)(123));
    printf("%d\n", (int)vec.data[0]);
    return 0;
}