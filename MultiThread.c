/*threadWrite线程向共享内存写数据，threadRead线程从共享内存读数据*/
/*读线程必须等待写线程执行完才开始*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <errno.h>

static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
//int g_Flag=0;
void* threadRead(void*);
void* threadWrite(void*);

int shmid;
char *buf[4092];

int main(int argc,char** argv)
{
    printf("Enter main\n");
    
    printf("Create shared memory id\n");
    shmid=shmget(IPC_PRIVATE,sizeof(char)*4092,IPC_CREAT|0666);
    printf("shmid:%d\n",shmid);
    
    int rt=0,wt=0;
    pthread_t rtid,wtid;
    wt=pthread_create(&wtid,NULL,threadWrite,NULL);
    if(wt!=0)
    {
        printf("create Write Thread error:%s,errno:%d\n",strerror(errno),errno);
        exit(0);
    }
    rt=pthread_create(&rtid,NULL,threadRead,&wtid);
    if(rt!=0)
    {
        printf("create Read Thread error:%s,errno:%d\n",strerror(errno),errno);
        exit(0);
    }
    pthread_cond_wait(&cond,&mutex);
    printf("Leave main\n");
    exit(0);
}

void* threadRead(void* arg)
{
    printf("Enter Read thread,start reading after Write thread finished!\n");
    
    char *shmptr;
    int i=0;
    char word;
    if((int)(shmptr=shmat(shmid,NULL,0))==-1)
    {
        printf("The Read thread attach the shared memory failed!\n");
        exit(0);
    }
    printf("threadRead Attached successfully!\n");
    
    pthread_join(*(pthread_t*)arg,NULL);   //阻塞read thread  ，arg为threadWrite线程标示符
    printf("this is Read thread,thread id is %u,g_Flag:%d\n",(unsigned int)pthread_self(),g_Flag);
    pthread_mutex_lock(&mutex);
    //g_Flag=1;
    printf("Reading shared memory...\n");
    printf("Content:%s\n",shmptr);
    shmdt(shmptr);
    pthread_mutex_unlock(&mutex);
    printf("Read over,thread id is %u,g_Flag:%d\n",(unsigned int)pthread_self(),g_Flag);
    printf("Leave Read thread!\n");
    pthread_cond_signal(&cond);
    pthread_exit(0);
}

void* threadWrite(void* arg)
{
    printf("Enter Write thread,we'll write something\n");

    
    char *shmptr;
    int i=0;
    char word;
    if((int)(shmptr=shmat(shmid,NULL,0))==-1)
    {
        printf("The Write thread attach the shared memory failed!\n");
        exit(0);
    }
    printf("threadWrite Attached successfully!\n");
    
    printf("this is Write thread,thread id is %u,g_Flag:%d\n",(unsigned int)pthread_self(),g_Flag); 
    pthread_mutex_lock(&mutex);
    //g_Flag=2;
    
    printf("write a string into shared memory\n");
    while((word=getchar())!='\n')
        shmptr[i++]=word;
    pthread_mutex_unlock(&mutex);
    printf("Write over,thread id is %u,g_Flag:%d\n",(unsigned int)pthread_self(),g_Flag);
    printf("Leave Write thread!\n");
    pthread_exit(0);
}

