#include <stdio.h>
#include <stdlib.h>

// Define function pointer type for arithmetic operations
typedef int (*operation_t)(int, int);

// Arithmetic functions
int add(int a, int b) { 
    printf("Adding 'a' and 'b'\n"); 
    return a + b; 
}

int subtract(int a, int b) { 
    printf("Subtracting 'b' from 'a'\n"); 
    return a - b; 
}

int multiply(int a, int b) { 
    printf("Multiplying 'a' and 'b'\n"); 
    return a * b; 
}

int divide(int a, int b) { 
    printf("Dividing 'a' by 'b'\n"); 
    if (b == 0) {
        printf("Error: Division by zero\n");
        return 0;
    }
    return a / b; 
}

int exit_program(int a, int b) {
    printf("Exiting program...\n");
    exit(0);
    return 0;
}

int main(void) {
    int a = 6;
    int b = 3;
    char input[10];

    // Array of function pointers mapping to choices 0, 1, 2, 3, 4
    operation_t operations[] = {add, subtract, multiply, divide, exit_program};

    while (1) {
        printf("Operand 'a' : %d | Operand 'b' : %d\n", a, b);
        printf("Specify the operation to perform (0 : add | 1 : subtract | 2 : Multiply | 3 : divide | 4 : exit): ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        // Convert character input (e.g., '0' -> 0) to array index
        int choice = input[0] - '0';

        // Direct call via function pointer array (no conditional statements used)
        int result = operations[choice](a, b);
        printf("x = %d\n\n", result);
    }

    return 0;
}