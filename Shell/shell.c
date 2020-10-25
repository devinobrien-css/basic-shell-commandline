/* Functions with basic shell terminal commands
 * author: Devin O'Brien
 * version: 1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>

int tokenize(char * buff, char ** cmd);
int packager(char * cmd[30], char list[20][20][30]);
void execute(int count,char list[20][20][30]);
int size(char list[20][20][30], int index);


/*TOKENIZE FUNCTION
 * Tokenizes line in from user
 *  • tokenizes line by spaces
 *  • list is then packaged by scan function
 *  • NOTE:
 *     i at the end of this loop is equal to the argc
 */
int tokenize(char * buff, char ** cmd)
{
	char * tok = strtok(buff," ");
	int i = 0;
	for(i = 0; tok != NULL; i++)
	{
		
		cmd[i] = tok;
		tok = strtok(NULL," "); 
	}
	cmd[i] = NULL; //NULL TERMINATES THIS ARRAY 
	return i;
}
/*SCAN FUNCTION
 * Searches for keywords and assembles packages 
 *	• all packages terminated by String NULL \0
 *  • returns number of packages
 *  • NOTE:
 *   cmd is list of tokenized arguments from user;
 *    this function sorts them into null 
 *     terminated lists  and packages them
 */
int packager(char * cmd[30], char list[20][20][30])
{
	int index = 0, i = 0;
	while(cmd[i] != NULL)
	{
		if(strcmp(cmd[i],"cat") == 0) //TAKES TWO FILES
		{
			strcpy(list[index][0],cmd[i]); 
			strcpy(list[index][1],cmd[i+1]); 
			strcpy(list[index][2],cmd[i+2]);
			strcpy(list[index][3],"\0");
			index++; i+=2;
		}	
		else if(strcmp(cmd[i],"egrep") == 0) //TAKES ONE CHAR
		{
			if(strstr(cmd[i+2],".txt") != NULL)
			{
				strcpy(list[index][0],cmd[i]); 
				strcpy(list[index][1],cmd[i+1]); 
				strcpy(list[index][2],cmd[i+2]);
				strcpy(list[index][3],"\0");
				i+=2;
			}
			else 
			{
				strcpy(list[index][0],cmd[i]); 
				strcpy(list[index][1],cmd[i+1]); 
				strcpy(list[index][2],"\0");
				i++;
			}
			index++; 
		}	
		else if(strcmp(cmd[i],"more") == 0)
		{
			strcpy(list[index][0],cmd[i]);  
			strcpy(list[index][1],"\0"); 
			index++; 
		}	
		else if(strcmp(cmd[i],"less") == 0)
		{
			strcpy(list[index][0],cmd[i]);  
			strcpy(list[index][1],"\0"); 
			index++; 
		}	
		else if(strcmp(cmd[i],"wc") == 0)
		{
			if(strstr(cmd[i+1],".txt") != NULL)
			{
				strcpy(list[index][0],cmd[i]); 
				strcpy(list[index][1],cmd[i+1]);
				strcpy(list[index][2],"\0");
				i++;
			}	
			else if(strstr(cmd[i+1],"-") != NULL)
			{
				strcpy(list[index][0],cmd[i]); 
				strcpy(list[index][1],cmd[i+1]);
				if(strstr(cmd[i+2],".txt") != NULL)
				{
					strcpy(list[index][2],cmd[i+2]);
					strcpy(list[index][3],"\0");
					i+=2;
				}
				else
				{
					strcpy(list[index][2],"\0");
					i++;
				}
				
				
			}
			else
			{
				strcpy(list[index][0],cmd[i]);  
				strcpy(list[index][1],"\0");
			} 
			index++; 
		}	
		else if(strcmp(cmd[i],"sort") == 0)
		{
			if(strstr(cmd[i+1],".txt") != NULL)
			{
				strcpy(list[index][0],cmd[i]); 
				strcpy(list[index][1],cmd[i+1]);  
				strcpy(list[index][2],"\0");
				i++;
			}
			else
			{
				strcpy(list[index][0],cmd[i]);  
				strcpy(list[index][1],"\0");
			} 
			index++; 
		}	
		else if(strcmp(cmd[i],"|") == 0)
		{
			strcpy(list[index][0],cmd[i]);  
			strcpy(list[index][1],"\0"); 
			index++; 
		}	
		else if(strcmp(cmd[i],">") == 0) //TAKES ONE FILE
		{
			strcpy(list[index][0],cmd[i]);  
			strcpy(list[index][1],cmd[i+1]);
			strcpy(list[index][2],"\0"); 
			index++; i++;
		}
		i++;
	}
	strcpy(list[index][0],"\0");
	return index; //SIZE OF LIST
}
/*EXECUTE FUNCTION
 * Accepts list of commands and executes 
 *  • will pipe two or more processes
 *  • accepts count - number of packages
 *  • accepts list of commands and executes 
 *  • will pipe two or more processes
 */
void execute(int count,char list[20][20][30])
{

	//loop through list of packaged commands
		for(int i = 0; i < count; i++)
		{
			// count equals one. single command
			if(count == 1)
			{
				pid_t pid;

				if((pid = fork()) < 0)
				{
					fprintf(stderr,"ERROR IN FORK - loc: void execute(int)\n");
				}
				else if(pid == 0)
				{
					int pkg_size = size(list,0);

					if(pkg_size == 3)
					{
						char *pkg[] = {list[0][0],list[0][1],list[0][2],NULL};
							execvp(pkg[0],pkg);
					}
					else if(pkg_size == 2)
					{
						char *pkg[] = {list[0][0],list[0][1],NULL};
							execvp(pkg[0],pkg);
					}
					else if(pkg_size == 1)
					{
						char *pkg[] = {list[0][0],NULL};
							execvp(pkg[0],pkg);
					}
					fprintf(stderr,"ERROR IN EXEC - loc: void execute(int)\n");
				}
				else 
				{
					wait(NULL);
				}
			}
			else if((strcmp(list[i+1][0],"|") == 0 ) || (strcmp(list[i+1][0],">") == 0)) //pipe from pcs -> fd[] -> ... || pcs -> file
			{
				

				int fd[2];
				int fds = 0; // file des output to next input | STARTS AT STDIN
				int stop = 1;
				pid_t pid;

					/* Recurring Pipe Loop
					 *  -will recur if piping several commands to eachother.
					 *   •saves output from inital pipe for input pipe of next command
					 *  -if there isn't a pipe after the command:
					 *   •check if there is a push after the command
					 *   •set stop to a negative integer
					 */
					do
					{
						if((strcmp(list[i+1][0],"|") != 0)) //FOR SECOND LOOP: will continue if pipes continue
						{
							stop = -1;
						}
						else if(i > count) //ERROR CHECK: checks bounds
						{
							stop = -1;
						}
							pipe(fd);
						
						
						if((pid = fork()) == -1)
						{
							fprintf(stderr,"ERROR IN FORK - loc: void execute(int)\n");
						}
						else if(pid == 0)//CHILD : WILL PUSH TO FILE OR PIPE
						{
							//SET INPUT : FOR FIRST ITERATION (fds == 0) THIS WILL BE STDIN (CMD ARGS SENT BY EXEC)
							dup2(fds,0);

							if((strcmp(list[i+1][0],">") == 0))//SEND TO FILE
							{
								int Ffd;
								if((Ffd = open(list[i+1][1],O_WRONLY|O_APPEND|O_CREAT)) == -1)
								{
									fprintf(stderr,"ERROR IN CREATION OF FILE - loc: void execute(int)\n");
								}
								else
								{
									dup2(Ffd,1);
									int pkg_size = size(list,i);

									if(pkg_size == 3)
									{
										char *pkg[] = {list[i][0],list[i][1],list[i][2],NULL}; 
											execvp(pkg[0],pkg);
									}
									else if(pkg_size == 2)
									{
										char *pkg[] = {list[i][0],list[i][1],NULL}; 
											execvp(pkg[0],pkg);
									}
									else if(pkg_size == 1)
									{
										char *pkg[] = {list[i][0],NULL}; 
											execvp(pkg[0],pkg);
									}
									fprintf(stderr,"ERROR IN EXEC - loc: void execute(int)\n");
								}
								
							} 
							else//SEND TO PIPE
							{
								close(fd[0]);
								dup2(fd[1],1);
								int pkg_size = size(list,i);

								if(pkg_size == 3)
								{
									char *pkg[] = {list[i][0],list[i][1],list[i][2],NULL}; 
										execvp(pkg[0],pkg);
								}
								else if(pkg_size == 2)
								{
									char *pkg[] = {list[i][0],list[i][1],NULL}; 
										execvp(pkg[0],pkg);
								}
								else if(pkg_size == 1)
								{
									char *pkg[] = {list[i][0],NULL}; 
										execvp(pkg[0],pkg);
								}
								fprintf(stderr,"ERROR IN EXEC - loc: void execute(int) iteration: %d\n",i);
							}
							
							//EXECUTION OF COMMAND
							
							
						}
						else if(pid > 0)//PARENT
						{
							wait(NULL);
							if(strcmp(list[i+1][0],"|") == 0)
							{
								close(fd[1]);
								fds = fd[0];
							}
						}
						i+=2;

					}while(stop > 0);
				}
		}
}


/*
 * Returns pkg size
 */
int size(char list[20][20][30], int index)
{
	int i = 0;
	while((strcmp(list[index][i],"\0") != 0))
	{
		i++;
	}
	return i;
}

/*
 * 
 *
 */
int main(int argc, char * argv[])
{
	{
		int loop = 1;
		while(loop > 0)
		{
			char buff[150]; //FULL LINE IN FROM PROMPT
			char *cmd[30]; //TOKENIZED ARRAY OF COMMANDS
			char list[20][20][30];//LIST OF EXECUTABLE PACKAGES

			printf("usr-Macbook-Pro-2 location $> ");
			fflush(stdout);
			scanf("%[^\n]%*c",&buff); //WILL BECOME PROPT - maybe add "%*c"?

				int argC = 0;
				argC = tokenize(buff,cmd);

				int pkg_count = 0;
				pkg_count = packager(cmd,list); //count is used for execution loop
				
				
				if((strcmp(cmd[0],"exit") == 0))
				{
					loop = -1;
				}
				else if(pkg_count == 0)
				{
					printf("No args entered.\n");
				}
				else
				{
					execute(pkg_count,list);
				}
		}
	} 
}




