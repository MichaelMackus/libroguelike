#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>

typedef struct TestNode {
    int priority;
    const char *name;
} TestNode;

int test_cmp(const void *d1, const void *d2)
{
    const TestNode *tn1 = (const TestNode*) d1;
    const TestNode *tn2 = (const TestNode*) d2;
    return tn1->priority < tn2->priority;
}

int main(void)
{
    int nodes_len = 4;
    TestNode node0 = (TestNode) { .priority = 100, .name = "Ohai" };
    TestNode node1 = (TestNode) { .priority = 99, .name = "Ohmm" };
    TestNode node2 = (TestNode) { .priority = 98, .name = "OWhy" };
    TestNode node3 = (TestNode) { .priority = 97, .name = "Obye" };
    RL_Heap *q = rl_heap_create(nodes_len, &test_cmp);

    rl_heap_insert(q, &node0);
    rl_heap_insert(q, &node1);
    rl_heap_insert(q, &node2);
    rl_heap_insert(q, &node3);
    rl_heap_insert(q, &node1);
    rl_heap_insert(q, &node2);

    TestNode *d;
    while ((d = (TestNode*) rl_heap_pop(q))) {
        printf("%s\n", d->name);
    }

    rl_heap_destroy(q);

    printf("Done\n\n");

    q = rl_heap_create(nodes_len, NULL);

    rl_heap_insert(q, &node0);
    rl_heap_insert(q, &node1);
    rl_heap_insert(q, &node2);
    rl_heap_insert(q, &node3);

    while ((d = (TestNode*) rl_heap_pop(q))) {
        printf("%s\n", d->name);
    }

    rl_heap_destroy(q);

    printf("Done\n");

    return 0;
}
