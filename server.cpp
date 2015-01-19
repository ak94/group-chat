#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <poll.h>
#include <sys/shm.h>
#include "bits/stdc++.h"

#define maxbuf 1024

using namespace std;

int shmid,*pid;
char buffer[maxbuf];
std::vector<pair<int,int> > v_grp[2];

std::map<int, pair<int,int> > map_pid,map_name;

void sigusr1_handler(int signo,siginfo_t *siginfo, void *context)
{
	
	int group_id=-1,index=-1;
	group_id=map_pid[(int)siginfo->si_pid].first;
	index=map_pid[(int)siginfo->si_pid].second;

	/*for (std::map<int,pair<int,int> >::iterator i = map_pid.begin(); i != map_pid.end(); ++i)
	{
		if(i->first == (int)siginfo->si_pid)
		{
			group_id=i->second.first;
			index=i->second.second;
			break;
		}
	}*/

	if(group_id!=-1 && index!=-1)
	{
		int k=0;
		read(v_grp[group_id][index].first,buffer,maxbuf);
		if(strcmp(buffer,"exit")==0)
		{
			for (std::vector<pair<int,int> >::iterator i = v_grp[group_id].begin(); i != v_grp[group_id].end(); ++i)
			{
				if(k==index)
				{
					i->first=-1;i->second=-1;
					break;
				}
				k++;
			}

			map_pid.erase((int)siginfo->si_pid);
		}else
		{
			for (std::vector<pair<int,int> >::iterator i = v_grp[group_id].begin(); i != v_grp[group_id].end(); ++i)
			{

				if(k==index || (i->first==-1 && i->second==-1))
				{
					k++;
					continue;
				}
				k++;
				//write(i->second,map_name[])
				write(i->second,buffer,maxbuf);
			}

		}
		
	}else
	{
		cout<<"map_pid error\n";
	}
}

void sigusr2_handler(int signo,siginfo_t *siginfo, void *context)
{
	
	int p=(int)siginfo->si_pid;
	int key_r=p;
	int key_w=p*7;
	
	mkfifo(to_string(key_r).c_str(),0666);
	mkfifo(to_string(key_w).c_str(),0666);
	
	int fd[2];
	fd[1]=open(to_string(key_r).c_str(),O_WRONLY);
	fd[0]=open(to_string(key_w).c_str(),O_RDONLY);
	
	kill(siginfo->si_pid,SIGUSR1);
	
	char buf[2];
	
	memset(buffer,'\0',maxbuf);
	

	while(1)
	{
		read(fd[0],buf,2);

		if(buf[0]!='\0')
			break;
	}
	
	int group_id = buf[0]-'0';
	group_id--;
	v_grp[group_id].push_back(make_pair(fd[0],fd[1]));
	map_pid[p]=make_pair(group_id,v_grp[group_id].size()-1);
	
	/*for (std::map<int,pair<int,int> >::iterator i = map_pid.begin(); i != map_pid.end(); ++i)
	{
		cout<<i->first<<" <"<<i->second.first<<","<<i->second.second<<">\n";
	}*/
}

int main(int argc, char const *argv[])
{
	struct sigaction sa;
	sa.sa_sigaction = &sigusr1_handler;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGUSR1,&sa,NULL);

    /**
     * signal handler for new client
     */
     struct sigaction sa2;
     sa2.sa_sigaction = &sigusr2_handler;
     sa2.sa_flags = SA_SIGINFO;
     sigemptyset(&sa2.sa_mask);

     sigaction(SIGUSR2,&sa2,NULL);
     map_pid.clear();
     map_name.clear();
     v_grp[0].clear();
     v_grp[1].clear();   

    /**
     * shared memory creation for server process id
     */
     pid=(int*)malloc(1*sizeof(int));

     int key=ftok("server.cpp",0);
     shmid=shmget(key,30,0666|IPC_CREAT);

     pid=(int*)shmat(shmid,(void*)0,0);

     pid[0]=getpid();
     
 	/**
 	 * signal handler for incoming data
 	 */


 	 while(1);

 	 return 0;
 	}
