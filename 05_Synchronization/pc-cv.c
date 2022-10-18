#include "thread.h"
#include "thread-sync.h"

int n, count = 0;
mutex_t lk = MUTEX_INIT();
cond_t cv = COND_INIT();

void Tproduce()
{
    while (1)
    {
        mutex_lock(&lk);
        while (!(count != n))
        {
            cond_wait(&cv, &lk);
        }
        assert(count != n);
        printf("(");
        count++;
        // prevent other thread contending for the lock
        cond_broadcast(&cv);
        mutex_unlock(&lk);
    }
}

void Tconsume()
{
    while (1)
    {
        mutex_lock(&lk);
        while (!(count != 0))
        {
            pthread_cond_wait(&cv, &lk);
        }
        // and lock is held
        assert(count != 0);
        printf(")");
        count--;
        // prevent other thread contending for the lock
        cond_broadcast(&cv);
        mutex_unlock(&lk);
    }
}

int main(int argc, char *argv[])
{
    assert(argc == 2);

    // ascii to integer
    n = atoi(argv[1]);

    setbuf(stdout, NULL);
    
    for (int i = 0; i < 8; i++)
    {
        create(Tproduce);
        create(Tconsume);
    }
}
