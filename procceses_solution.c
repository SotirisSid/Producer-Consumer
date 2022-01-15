#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define BUFFER_SIZE 6
#define MAXNUM 100

#define SHM_BUFFER "pro-con-buffer"
#define SHM_IN "pro-con-in"
#define SHM_OUT "pro-con-out"
#define N_CONSUMER 2
//sem_t *mutex;
const char *semmut_n = "mutex";
const char *sememp_n = "empty";
const char *semfull_n = "full";


//sem_t *empty;
//sem_t *full;
//pthread_mutex_t *mutex;


typedef struct{
    int num;
    char str[5]

}item;

item buffer[BUFFER_SIZE];
int in=0;
int out=0;




// initialize buffer
void init_buffer() {
    item it;
    for(int i= 0; i< BUFFER_SIZE; i++) {
        it.num= -1;
        sprintf(it.str, "%3d", it.num);
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
void set_item(item *buffer,int pos,item it){
    buffer[pos].num=it.num;
    do_sleep(4);
    strcpy(buffer[pos].str,it.str);

}
item get_item(item *buffer,int pos){
    item it;
    it.num=buffer[pos].num;
    do_sleep(4);
    strcpy(it.str,buffer[pos].str);
    return it;

}

void producer(int n){
// open shared memory
 sem_t *sem_mut = sem_open(semmut_n, O_CREAT);
  sem_t *sem_em = sem_open(sememp_n,O_CREAT);
   sem_t *sem_full = sem_open(semfull_n, O_CREAT);

int sval,fullval;
    int pro_fd_buffer= shm_open(SHM_BUFFER, O_RDWR, 0640);
    int pro_fd_in= shm_open(SHM_IN, O_RDWR, 0640);
    int pro_fd_out= shm_open(SHM_OUT, O_RDWR, 0640);



    item *pro_buffer=mmap(NULL,BUFFER_SIZE*sizeof(item),PROT_READ | PROT_WRITE,MAP_SHARED,pro_fd_buffer,0);
    int *pro_in=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,pro_fd_in,0);
    int *pro_out=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,pro_fd_out,0);
    while(1){

   //printf("%d",pempty);
    do_sleep(3);


    item it=produce_item(n);
    sem_wait(sem_em);
    sem_wait(sem_mut);

    printf("produced --> item %3d,%s \n",it.num,it.str);
    set_item(pro_buffer,*pro_in,it);
    *pro_in=(*pro_in+1)%BUFFER_SIZE;

//    pthread_mutex_unlock(&mutex);

   sem_post(sem_mut);
   sem_post(sem_full);

   sem_getvalue(sem_full, &fullval);

    }
    //unmap
    munmap(pro_buffer,BUFFER_SIZE*sizeof(item));
    munmap(pro_in,sizeof(int));
    munmap(pro_out,sizeof(int));

   sem_unlink(sememp_n);
   sem_unlink(semfull_n);
    sem_unlink(semmut_n);
    exit(0);
}

void consumer(){
    int sval;
    sem_t *sem_mut = sem_open(semmut_n, O_CREAT);
    sem_t *sem_em = sem_open(sememp_n, O_CREAT);
    sem_t *sem_full = sem_open(semfull_n,O_CREAT);



    int con_fd_buffer= shm_open(SHM_BUFFER, O_RDWR, 0640);
    int con_fd_in= shm_open(SHM_IN, O_RDWR, 0640);
    int con_fd_out= shm_open(SHM_OUT, O_RDWR, 0640);


    //memory map the shared memory obj


    item *con_buffer=mmap(NULL,BUFFER_SIZE*sizeof(item),PROT_READ | PROT_WRITE,MAP_SHARED,con_fd_buffer,0);
    int *con_in=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,con_fd_in,0);
    int *con_out=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,con_fd_out,0);

    while(1){

    do_sleep(3);
    sem_wait(sem_full); //sem
    sem_wait(sem_mut);


    item it=get_item(con_buffer,*con_out);
    *con_out=(*con_out+1)%BUFFER_SIZE;

    printf("------consumed--> item %3d,%s\n",it.num,it.str);


    sem_post(sem_mut);
    sem_post(sem_em);

    }

    munmap(con_buffer,BUFFER_SIZE*sizeof(item));
    munmap(con_in,sizeof(int));
    munmap(con_out,sizeof(int));
     sem_unlink(sememp_n);
   sem_unlink(semfull_n);
    sem_unlink(semmut_n);
    exit(0);

}





int main()
{

int sval;


  sem_t *sem_mut = sem_open(semmut_n, O_CREAT,  0640, 1);
  sem_t *sem_em = sem_open(sememp_n, O_CREAT,  0640, BUFFER_SIZE);
  sem_t *sem_full = sem_open(semfull_n, O_CREAT,  0640, 0);
sem_init(sem_em,1,BUFFER_SIZE);
sem_init(sem_mut,1,1);
sem_init(sem_full,1,0);

      sem_getvalue(sem_full, &sval);
    printf("full;=%d\n", sval);
    sem_getvalue(sem_em, &sval);
    printf("empty=%d\n", sval);
    sem_getvalue(sem_mut, &sval);
    printf("mutex=%d\n", sval);


    int shm_buffer = shm_open(SHM_BUFFER, O_CREAT | O_RDWR, 0640);
    int shm_in = shm_open(SHM_IN, O_CREAT | O_RDWR, 0640);
    int shm_out = shm_open(SHM_OUT, O_CREAT | O_RDWR, 0640);



    // configure the size of the shared memory object
    ftruncate(shm_buffer, BUFFER_SIZE*sizeof(item));
    ftruncate(shm_in, sizeof(int));
    ftruncate(shm_out, sizeof(int));
    // memory map the shared memory object
    item *ptr_buffer = mmap(NULL, BUFFER_SIZE*sizeof(item), PROT_READ | PROT_WRITE,
    MAP_SHARED, shm_buffer, 0);
    int *ptr_in = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_in, 0);
    int *ptr_out = mmap(NULL, sizeof(int), PROT_READ| PROT_WRITE, MAP_SHARED, shm_out, 0);
    // write to the shared memory object
    // init buffer, in, out
    item it;
    it.num = -1;
    sprintf(it.str, "%3d", it.num);
    for (int i = 0; i < BUFFER_SIZE; i++) {
    ptr_buffer[i].num = it.num;
    strcpy(ptr_buffer[i].str, it.str);
    }
    *ptr_in = 0;
    *ptr_out = 0;
int n1=fork();  //fork1
/*
if(n1==0){
producer(getpid());}
else{
consumer();}
*/


int n2=fork();  //fork2

if (n1 > 0 && n2 > 0) {
    printf(" my id is %d \n", getpid());
    srand(time(NULL));
        producer(getpid());


    }

    else if (n1 == 0 && n2 > 0)
    {
    do_sleep(1);
        printf(" my id is %d \n", getpid());
        srand(2);
        producer(getpid());

    }
    else if (n1 > 0 && n2 == 0)
    {
      //  srand(time(NULL));
        printf(" my id is %d \n", getpid());
        consumer();

    }
    else {
    do_sleep(1);
    //srand(2);
    printf(" my id is %d \n", getpid());
        consumer();

    }


   wait(NULL);
   wait(NULL);




    munmap(ptr_buffer, BUFFER_SIZE*sizeof(item));
    munmap(ptr_in, sizeof(int));
    munmap(ptr_out, sizeof(int));
    shm_unlink(SHM_BUFFER);
    shm_unlink(SHM_IN);
    shm_unlink(SHM_OUT);
    sem_unlink(semmut_n);
    sem_unlink(semfull_n);
    sem_unlink(semmut_n);
    printf("done\n");

return 0;
}
