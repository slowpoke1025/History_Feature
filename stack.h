#ifndef STACK_H
#define STACK_H
#endif

#define BUFFER_SIZE 1024

typedef struct Stack
{
    char value[BUFFER_SIZE + 1];
    int top;
} Stack;
void init_stack(Stack *stack);
char pop_stack(Stack *stack);
char push_stack(Stack *stack, char val);
void close_stack(Stack *stack);
int empty_stack(Stack *stack);

typedef struct RStack
{
    char value[BUFFER_SIZE + 1];
    int top;
} RStack;
void init_rstack(RStack *rstack);
char pop_rstack(RStack *rstack);
char push_rstack(RStack *rstack, char val);
int empty_rstack(RStack *rstack);
