#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#include "../threadpool.h"

#define Ntestcase1 10
pthread_spinlock_t spin;

void test_drain_buffer(void *arg) {
    int *count = (int *) arg;
    pthread_spin_lock(&spin);
    (*count)+=1;
    pthread_spin_unlock(&spin);
}

int main(void) {
    threadpool_t *pool;
    pthread_spin_init(&spin, 0);

    /* test case 1: wait queue drain */
    int count = 0;
    pool = threadpool_create(5, Ntestcase1, 1);
    // printf("pool is created\n");
    for (int i = 0; i < Ntestcase1; i++)
    {
        assert(threadpool_add_task(pool, test_drain_buffer, &count));
    }
    assert(threadpool_destroy(pool));
    printf("count:%d\n", count);
    // assert(count == Ntestcase1);
    pthread_spin_destroy(&spin);
    return 0;
}

