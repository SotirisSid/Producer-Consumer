#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#define BUFFER_SIZE 5
#define MAXNUM 100
//two thread solution

typedef struct{
    int num;
    char str[5]

}item;

item buffer[BUFFER_SIZE];
int in=0;
int out=0;
sem_t empty;
sem_t full;
pthread_mutex_t mutex;



// initialize buffer
void init_buffer() {
    item it;
    for(int i= 0; i< BUFFER_SIZE; i++) {
        it.num= -1;sprintf(it.str, "%3d", it.num);
        buffer[i] = it;
    }
}
void do_sleep(int n){
    sleep((rand()% n)+1);
}
//produce item
item produce_item(){
    item it;
    it.num=rand()% MAXNUM;
    sprintf(it.str,"%3d",it.num);
    return it;
}
void set_item(int pos,item it){
    buffer[pos].num=it.num;
    do_sleep(4);
    strcpy(buffer[pos].str,it.str);

}
item get_item(int pos){
    item it;
    it.num=buffer[pos].num;
    //do_sleep(4);
    strcpy(it.str,buffer[pos].str);
    return it;

}

void *producer(void* rank){
    while(1){
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);
    item it;
    //produce next item
    it=produce_item();
    printf("produced --> item %3d,%s\n",it.num,it.str);
    set_item(in,it);
    in=(in+1)%BUFFER_SIZE;
    pthread_mutex_unlock(&mutex);
    sem_post(&full);

    }
}

void *consumer(void *rank){
    while(1){
    sem_wait(&full);
    pthread_mutex_lock(&mutex);
    //get and consume item
    do_sleep(2);
    item it =get_item(out);
    out=(out+1)% BUFFER_SIZE;
    printf("consumed --> item %3d,%s\n",it.num,it.str);
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);


    }

}





int main()
{
pthread_t t1_prod,t2_prod,t1_cons,t2_cons;
pthread_mutex_init(&mutex, NULL);
sem_init(&empty,0,BUFFER_SIZE);
sem_init(&full,0,0);

srand(time(0));
init_buffer();

//create threads
pthread_create(&t1_prod,NULL,producer,NULL);
pthread_create(&t2_prod,NULL,producer,NULL);
pthread_create(&t1_cons,NULL,consumer,NULL);
pthread_create(&t2_cons,NULL,consumer,NULL);



//wait till threads are completed
pthread_join(t1_prod,NULL);
pthread_join(t2_prod,NULL);
pthread_join(t1_cons,NULL);
pthread_join(t2_cons,NULL);




printf("done");
pthread_mutex_destroy(&mutex);
sem_destroy(&empty);
sem_destroy(&full);



return 0;
}
