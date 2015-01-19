#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include "iostream"

using namespace std;

#define maxbuf 1024

int ffd[2];
bool wt = true;
int *pid,shmid;
void sigusr2_handler(int signum)
{
	
	wt=false;
}

void sigusr1_handler(int signo,siginfo_t *siginfo, void *context)
{
	cout<<"Server: Select a group to join.\n\t1\n\t2\n";
	int g;
	cin>>g;
	char buf[2];
	memset(buf,'\0',2);
	buf[0]= (char)(((int)'0')+g);
	buf[1]='\0';
	write(ffd[1],buf,2);
	kill(getpid(),SIGUSR2);
	//signal(SIGUSR2,sigusr2_handler);
}

int main(int argc, char const *argv[])
{
	signal(SIGUSR2,sigusr2_handler);

	struct sigaction sa;
	sa.sa_sigaction = &sigusr1_handler;
    	sa.sa_flags = SA_SIGINFO;
    	sigemptyset(&sa.sa_mask);
    	
	sigaction(SIGUSR1,&sa,NULL);

	int p = getpid();
	
	int key_r=p;
	int key_w=p*7;
	
	mkfifo(to_string(key_r).c_str(),0666);
	mkfifo(to_string(key_w).c_str(),0666);

	pid=(int*)malloc(1*sizeof(int));

    int key=ftok("server.cpp",0);
    shmid=shmget(key,30,0666|IPC_CREAT);

    pid=(int*)shmat(shmid,(void*)0,0);
    

    kill(pid[0],SIGUSR2);

    ffd[0]=open(to_string(key_r).c_str(),O_RDONLY);
	ffd[1]=open(to_string(key_w).c_str(),O_WRONLY);	

	while(wt);

	int c=fork();
	
	if(c>0)
	{
		char buffer[maxbuf];
		while(1)
 		{
 			memset(buffer,'\0',maxbuf);
 			
 			scanf("%s",buffer);

 			if(buffer[0]!='\0')
 			{
 				write(ffd[1],buffer,maxbuf);
 				kill(pid[0],SIGUSR1);
 			}
 			if(strcmp(buffer,"exit")==0)
 			{
 				
 				exit(1);
 			}
 		}

	}else if(c==0)
	{
		char buffer[maxbuf];
		while(1)
 		{
 			memset(buffer,'\0',maxbuf);
 			read(ffd[0],buffer,maxbuf);
 			
 			if(buffer[0]!='\0')
 				cout<<"incoming:..."<<buffer<<"\n";
 		}

	}else
	{
		perror("fork()");
		exit(1);
	}

	return 0;
}
