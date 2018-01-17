#ifndef CANDLEBROT_STACK_H
#define CANDLEBROT_STACK_H

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

typedef struct
{
    int x;
    int y;

    void *prev;
} stack_element_t;

typedef struct
{
    stack_element_t *current;
    size_t size;
} stack_t;

static stack_t *new_stack()
{
    stack_t *stack = malloc(sizeof(stack_t));
    stack->size = 0;

    return stack;
}

static void stack_push(stack_t *stack, stack_element_t *element)
{
    if(stack->size > 0)
    {
        element->prev = stack->current;
    }

    stack->current = element;
    stack->size++;
}

static stack_element_t *stack_pop(stack_t *stack)
{
    if(stack->current == NULL)
    {
        return NULL;
    }
    else
    {
        stack_element_t *result = stack->current;

        stack->current = result->prev;
        stack->size--;

        result->prev = NULL;
        return result;
    }
}

static void free_stack(stack_t *stack)
{
    while(stack->size > 0)
    {
        free(stack_pop(stack));
    }

    free(stack);
}

#endif //CANDLEBROT_STACK_H