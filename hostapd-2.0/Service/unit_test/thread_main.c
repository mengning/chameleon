#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct foo {
    int f_count;
    pthread_mutex f_lock;
    int f_id;
};

struct foo * foo_alloc(int id){
    struct foo *fp;
    if((fp = malloc(sizeof(struct foo)))!=NULL) {
        fp->f_count =1;
        fp->f_id =id;
        if (pthread_mutex_init(&fp->f_lock,NULL)!=0) {
            free(fp);
            return (NULL);
        }
    }
    return (fp);
}    

void foo_hold(struct foo *fp) //add
{
    pthread_mutex_lock(&fp->f_lock);
    fp->f_count++;
    pthread_mutex_unlock(&fp->f_lock);
}
void foo_rele(struct foo *fp) //release
{
    pthread_mutex_lock(&fp->f_lock);
    if(--fp->f_count==0) {
        pthread_mutex_unlock(&fp->f_lock);
        pthread_mutex_destroy(&fp->f_lock);
        free(fp);
    }else {
        pthread_mutex_unlock(&fp->f_lock);
    }
}


void *thr_fn1(void *arg)
{
    printf("thread 1 returning\n");
    return ((void *)1);
}
void *thr_fn2(void *arg)
{
    printf("thread 2 exiting\n");
    pthread_exit((void *)2);
}

int main()
{
    int err;
    pthread_t tid1,tid2;
    void *tret;
    err = pthread_create(&tid1,NULL,thr_fn1,NULL);
    if(err!=0)
        printf("can not create thread 1\n");

    err = pthread_create(&tid2,NULL,thr_fn2,NULL);
    if(err!=0)
        printf("can not create thread 2\n");

    err = pthread_join(tid1,&tret);
    if(err!=0)
        printf("can not join with thread1\n");
    printf("thread 1 exit %ld\n",(long)tret);
    err = pthread_join(tid2,&tret);
    if(err!=0)
        printf("can not join with thread2\n");
    printf("thread 2 exit %ld\n",(long)tret);
    exit(0);
}
