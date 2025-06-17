#include "cpp_func.h"
#include <stdio.h> 

void cpp_function() {
    static int count = 0;
    count++;
    printf("C++ function called: %d\n", count); 
}