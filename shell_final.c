//--------------------- INTERACTIVE SHELL PROGRAM
// Simran Kedia
//201201024

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>

typedef struct back
{
	char name[100];
	int pi;
	int state;
}back;
char *in,*out;
back background[100]; //contains the list of background processes
int back_c=0;
pid_t ppid;
char HOME[200];
int len;
//-------------------------------------------Creating a new process to execute a command--------------------------------------
void profork(char* w,char **wor,int mode)
{
	int status;
	pid_t pid;
	pid=fork();
	if (pid<0)
	{
		perror("Fork");
		exit(-1);
	}
	else if(pid==0)
	{
//----------------------------------------- INPUT - OUTPUT REDIRECTION-----------------------------------------		
		if(mode==33)
		{
		
			int fi,fo;
			fi=open(in,O_RDONLY);
			dup2(fi,0);
			fo=open(out,O_RDWR|O_CREAT,S_IRWXU);
			//			printf("input-- %d output-- %d\n",fi,fo);
			dup2(fo,1);
		}
//--------------------------------INPUT REDIRECTION----------------------------------------------------------
		if(mode==22)
		{
			int fi;
			fi=open(in,O_RDONLY);
			//			printf("mode--%d file descriptor--%d",mode,fi);
			dup2(fi,0);
			close(fi);
		}
//---------------------------------------OUTPUT REDIRECTION-----------------------------------------------------
		if (mode==11)
		{
			//			printf("!!");
			int fo;
			fo=open(out,O_RDWR|O_CREAT,S_IRWXU);
			//			printf("File descriptor %d\n",fo);
			dup2(fo,1);
			close(fo);
		}
//-------------------------------------------------------------------------------------------------------------------
		int r=execvp(w,wor);
		if(r<0)
			perror("Execvp ");

	}
	else
	{
//--------------------------------------BACKGROUND PROCESSES------------------------------------------------
		if (mode==44)	
		{
			background[back_c].pi=pid;
			background[back_c].state=1;
			int i,j=0;
			for(i=1;wor[i]!=NULL;i++)
			{
				strcat(wor[j]," ");
				strcat(wor[j],wor[i]);
			}
			strcpy(background[back_c].name,wor[0]);
			back_c++;
		}
		else
		{
			ppid=pid;
			if(waitpid(pid, &status, WUNTRACED)<0)
				perror("Wait ");
			if(WIFSTOPPED(status))
			{
				background[back_c].pi=pid;
				background[back_c].state=1;
				int i,j=0;
				for(i=1;wor[i]!=NULL;i++)
				{
					strcat(wor[j]," ");
					strcat(wor[j],wor[i]);
				}
				strcpy(background[back_c].name,wor[0]);
				back_c++;
			}
		}

	}
}
//----------------------------------------USER PROMPT--------------------------------------------------------
void prompt()
{
	char *path=malloc(100);
	getcwd(path,100);
	if(path==NULL)
		perror("cwd: ");
	char *point;
	char pa[100];
	point=strstr(path,HOME);
	if(point)
	{
		path=path+len;
		strcpy(pa,path);
		strcpy(path,"~");
		strcat(path,pa);
	}	
	char buf[100];
	gethostname(buf,100);
	char *user=malloc(100);
	user=getlogin();
	printf("%s@%s:%s> ",user,buf,path);
}

//------------------------------------SIGNAL HANDLER--------------------------------------------------------------
void sig_handle(int sign)
{
	if (sign==2 ||sign==3)
	{
		fflush(stdout);
		printf("\n");
		prompt();
		fflush(stdout);
		signal(SIGQUIT,sig_handle);
		signal(SIGINT,sig_handle);
	}
	if(sign==20)
	{
		kill(ppid,SIGTSTP);
		signal(SIGTSTP,sig_handle);
	}
	return;
}

void child_sig(int signo)
{
	int pid;
	int temp;
	pid=waitpid(WAIT_ANY, &temp, WNOHANG);
	int i;
	for(i=0;i<back_c;i++)
	{
		if(background[i].pi==pid && background[i].state==1)
		{
			background[i].state=0;
			printf("\n%s %d exited normally\n",background[i].name,background[i].pi);
			prompt();
			fflush(stdout);
		}
	}
	signal(SIGCHLD, child_sig);
	return;
}
//----------------------------------------------------------------------------------------------------------------------------
void pipes(char *n)
{
	int infile;
	int outfile;
	int oldpipe[2],newpipe[2];
	int stdin_c=dup(STDIN_FILENO);
	int stdout_c=dup(STDOUT_FILENO);

	int mode=00;
	char *pipec[100];
	char **command=malloc(sizeof(char *)*1);
	command[0]=malloc(sizeof(char **)*1000);
	char *t;
	int i=0;
//-----------------------------PARSING- TO SEPARATE PIPE COMMANDS---------------------------------------------------------
	t=strtok(n,"|");
	while(t!=NULL)
	{
		//printf("t---%s\n ",t);
		pipec[i++]=t;
		pipec[i-1][strlen(pipec[i-1])]='\0';
		t=strtok(NULL,"|");
	}
	pipec[i]=NULL;
//--------------------------------------------------PARSING TO SPLIT INTO COMMANDS-------------------------------------------------------
	int j=0;
	for(j=0;j<i;j++)
	{
		char *t1,*t2,*co;
		char *command[100];
		char *inp;
		int in=0;
		//printf("pipe 1--%s",pipec[j]);
		char pi[1000];
		strcpy(pi,pipec[j]);
		t1=strtok(pipec[j],"<");
		char t11[1000];
		strcpy(t11,t1);
		//printf("t1---%s\n",t1);
		if(strcmp(t1,pi)!=0)
		{
			mode=22;
			inp=strtok(NULL," ");
			//printf("file---%s\n",inp);
		}
		char pip[1000];
		strcpy(pip,pi);
		char t22[1000];

		t2=strtok(pi,">");
		strcpy(t22,t2);
		if(strcmp(t2,pip)!=0)
		{
			mode=11;
			out=strtok(NULL," ");
			//printf("file---%s\n",out);
		}
		if(mode==22)
		{
			//printf("INput Redirection\n");
			co=strtok(t11," ");
			while(co!=NULL)
			{
				command[in++]=co;
				co=strtok(NULL," ");
			}
			command[in]=NULL;
			infile=open(inp,O_RDONLY,777);
			if(infile<0)
				perror("File not created\n");
			else
			{
				dup2(infile,0);
				//printf("input redirected");
			}
		}
		else if(mode==11)
		{
			co=strtok(t22," ");
			while(co!=NULL)
			{
				command[in++]=co;
				co=strtok(NULL," ");
			}
			command[in]=NULL;
			outfile=open(out,O_RDWR|O_CREAT,777);
			if(outfile<0)
				perror("File not created\n");
			else
				dup2(outfile,1);

		}
		else
		{
			co=strtok(pipec[j]," ");
			//printf("pipie---%s\n",pipec[j]);
			while(co!=NULL)
			{
				command[in++]=co;
				co=strtok(NULL," ");
			}
			command[in]=NULL;
		}
//---------------------------------------------------PIPE IMPLEMENTATION----------------------------------------------------------------------------
		if(j<i-1)
			pipe(newpipe);
		pid_t pid=fork();

		if(j>0 && j<=i-1)
		{
			dup2(oldpipe[0],0);
			close(oldpipe[0]);
			close(oldpipe[1]);
		}

		if (pid==0)
		{
			if(j<i-1)
			{
				dup2(newpipe[1],1);
				close(newpipe[1]);
				close(newpipe[0]);
			}

			//	if(mode!=22 && mode!=11)

			int r=execvp(command[0],command);
			if(r<0)
			{
				perror("connamd not found");
				exit(1);
			}
		}
		else
		{
			int r;
			waitpid(pid,&r,0);

			if(j<i-1)
			{
				oldpipe[0]=newpipe[0];
				oldpipe[1]=newpipe[1];
			}
		}
	}
	close(oldpipe[0]);
	close(newpipe[0]);
	close(oldpipe[1]);
	close(newpipe[1]);
	// restore stdin, stdout
	dup2(stdin_c, 0);
	dup2(stdout_c, 1);
	close(stdin_c);
	close(stdout_c);
}	

//---------------------------------------------------------------------------------------------------------------------------------
int main()
{
	getcwd(HOME,200);
	if(HOME==NULL)
		perror("cwd ");
	len=strlen(HOME);
	signal(SIGINT,SIG_IGN);
	signal(SIGINT,sig_handle);
	signal(SIGCHLD,SIG_IGN);
	signal(SIGCHLD,child_sig);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTSTP,sig_handle);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGQUIT,sig_handle);
	while(1)
	{
		int mode=00;
		char *pch;char *p,*pa;
		//gethostname(pa,100);
		char *name=malloc(1000);
		char *n=malloc(1000);
		char **wo=malloc(sizeof(char *)*1);
		int j;
		for(j=0;j<1;j++)
			wo[j]=malloc(sizeof(char **)*100);
		prompt();
		int c;
		if((c=getchar())==EOF)
		{
			printf("\n");
			continue;
		}
		else
			ungetc(c,stdin);
		scanf ("%[^\n]s", name);
		strcpy(n,name);

		getchar();
		int index=0;
//---------------------------------------------PARSING- SPLITTING INTO COMMANDS----------------------------------------------
		pch=strtok(name," ");
		while(pch!=NULL)
		{
			if (strcmp(pch,"&")==0)
			{
				mode=44;
			}
			else if (strcmp(pch,">")==0 || strcmp(pch,"<")==0 || strcmp(pch,"|")==0)
			{
				if(strcmp(pch,">")==0)		// OUTPUT REDIRECTION
				{
					if(mode==22)
						mode=33;
					else mode=11;
					out=strtok(NULL," ");
					if(out==NULL)
					{
						printf("Incorrect Syntax\n");
						break;
					}
				}
				if(strcmp(pch,"<")==0)		//INPUT REDIRECTION
				{
					if (mode == 11)
						mode=33;
					else mode=22;
					in=strtok(NULL," ");
					if(in==NULL)
					{
						printf("Incorrect syntax\n");
						break;
					}
				}
				if(strcmp(pch,"|")==0)		//PIPES
				{
					mode=55;
					pipes(n);
					break;
				}
			}

			else
				wo[index++]=pch;
			pch=strtok(NULL," ");
		}
		wo[index]=pch;
		if(mode==55)
			continue;
		//int p=0;
		//for(p=0;p<index;p++)
		//	printf("%s ",wo[p]);
		//printf("\n");
		if(wo[0]==NULL)
			continue;
//-------------------------------------------USER DEFINED COMMANDS-----------------------------------------------------------
		else if(strcmp(wo[0],"quit")==0)
			exit(0);
		else if(strcmp(wo[0],"cd")==0)
		{
			int r;
			if(index==1)
			{	r=chdir(HOME);
				if(r<0)
					perror("error in changing directory\n");
			}
			else if(strcmp(wo[1],"~")==0 || strcmp(wo[1],"~/")==0)
			{
				r=chdir(HOME);
				if(r<0)
					perror("chdir ");
			}
			else
			{
				r=chdir(wo[1]);
				if(r<0)
					perror("chdir ");
			}
		}
		else if(strcmp(wo[0],"jobs")==0)
		{
			int i=0;int j=0;
			for(i=0;i<back_c;i++)
			{
				if(background[i].state==1)
				{
					j++;
					printf("[%d] %s [%d]\n",j,background[i].name,background[i].pi);
				}
			}
		}
		else if(strcmp(wo[0],"kjob")==0)
		{
			int pno=atoi(wo[1]);
			int i,j=0;
			for(i=0;i<back_c;i++)
			{
				if(background[i].state==1)
					j++;
				if(j==pno)
					break;
			}
			if(i==back_c)
				printf("No such job exists\n");
			else
				if(kill(background[i].pi,atoi(wo[2]))<0)
					perror("kill ");
		}
		else if(strcmp(wo[0],"overkill")==0)
		{
			if(wo[1]!=NULL)
				printf("overkill takes no argument\n");
			else
			{
				int i;
				for(i=0;i<back_c;i++)
				{
					if(background[i].state==1)
						kill(background[i].pi,9);
				}
				back_c=0;
				printf("All processes killed\n");
			}
		}
		else if(strcmp(wo[0],"fg")==0)
		{
			if(wo[1]==NULL || wo[2]!=NULL)
				printf("invalid command\n");
			else
			{
				int pno=atoi(wo[1]);
				int s;
				int i,j=0;
				for(i=0;i<back_c;i++)
				{
					if(background[i].state==1)
						j++;
					if(j==pno)
						break;
				}
				if(i==back_c)
					printf("No such process exists\n");
				else
				{
					if(waitpid(background[i].pi,&s,0)<0)
						perror("wait ");
					background[i].state=0;
				}
			}
		}
		else if(strcmp(wo[0],"pinfo")==0)
		{
			char filename1[100],filename2[100];
			char buf[256],buf1[256];
			char *arr[100];
			char *ptr;
			int currentpid;
			if(wo[1]==NULL)
				currentpid=getpid();
			else
				currentpid=atoi(wo[1]);
			if(wo[2]!=NULL)
				printf("Invalid no of argguments\n");
			else
			{
				sprintf(filename1,"/proc/%d/stat",currentpid);
				int fd1,fd2;
				fd1=open(filename1,O_RDONLY);
				if(fd1<0)
					perror("File cannot be opened\n");
				else
				{
					if(read(fd1,buf,256)<0)
						perror("read ");
					//printf("%s\n",buf);
					int count=0,j=0;
					ptr=strtok(buf," ");
					while(ptr!=NULL)
					{
						if(count==0||count==2||count==22)
						{
							arr[j]=malloc(sizeof(char)*100);
							strcpy(arr[j++],ptr);
						}
						count++;
						ptr=strtok(NULL," ");
					}
					printf("pid-- %s\n",arr[0]);
					printf("Process Status-- %s\n",arr[1]);
					printf("Memory-- %s\n",arr[2]);
					sprintf(filename2,"/proc/%d/exe",currentpid);
					char dest[100];
					if(readlink(filename2,dest,100)==-1)
						perror("readLink");
					else
						printf("Executable path--%s\n",dest);

				}	
			}
		}
//-------------------------------------------------------------------------------------------------------------------------------
		else
			profork(wo[0],wo,mode);
	}
	return 0;
}


