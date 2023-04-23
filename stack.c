#include "stack.h"

char pop_stack(Stack *stack)
{
    if (stack->top < 0)
        return '\0';
    char val = stack->value[stack->top];
    stack->value[stack->top--] = '\0';
    return val;
}

char push_stack(Stack *stack, char val)
{
    stack->value[++stack->top] = val;
    return val;
}

void close_stack(Stack *stack)
{
    stack->value[stack->top + 1] = '\0';
}

int empty_stack(Stack *stack)
{
    return stack->top < 0;
}

/***R STACK*/
char push_rstack(RStack *rstack, char val)
{
    rstack->value[--rstack->top] = val;
    return val;
}

char pop_rstack(RStack *rstack)
{
    if (rstack->top == BUFFER_SIZE)
        return '\0';
    char val = rstack->value[rstack->top++];
    return val;
}
int empty_rstack(RStack *rstack)
{
    return rstack->top == BUFFER_SIZE;
}

void init_stack(Stack *stack)
{
    stack->top = -1;
}
void init_rstack(RStack *rstack)
{
    rstack->value[BUFFER_SIZE] = '\0';
    rstack->top = BUFFER_SIZE;
}