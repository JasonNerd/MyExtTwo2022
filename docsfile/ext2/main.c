#include<stdio.h>
#include<string.h>
#include"disk.h"
#include"filsys.h"

int getcmd(char* cmdline){
	memset(cmdline, 0, MAXCMDLINE);
	fgets(cmdline, MAXCMDLINE, stdin);
	if(buf[0]==0)return -1;
	return 0;
}

int parsecmd(char* cmdline){
	int i, other_ind;
	char cmd[MAXCOMMAND];
	char otherinfo[MAXCMDLINE - MAXCOMMAND];
	char* p;
	//get the command
	for(i=0; i<strlen(cmdline); i++){
		if(cmdline[i]==' ' || cmdline[i] == '\n'){
			memset(cmd, 0, MAXCOMMAND);
			memcpy(cmd, cmdline, i);
			cmd[i]=0;
			other_ind = i+1;
		}
	}
	//other info(like path etc.)
	memset(otherinfo, 0, MAXCMDLINE - MAXCOMMAND);
	memcpy(otherinfo, cmdline+other_ind, strlen(cmdline)-other_ind);
	otherinfo[strlen(cmdline)-other_ind] = 0;
	//run cmd
	if(strcmp(cmd, "ls") == 0){
		return 1;
	}else if(strcmp(cmd, "mkdir") == 0){
		return 1;
	}else if(strcmp(cmd, "touch") == 0){
		return 1;
	}else if(strcmp(cmd, "cp") == 0){
		return 1;
	}else if(strcmp(cmd, "shutdown") == 0){
		return 0;
	}else{
		return -1;
	}
}

int main(){
	char cmdline[MAXCMDLINE];
	while(1){
		printf("[ext2] ");
		if(getcmd(cmdline) == 0){
			int status = parsecmd(cmdline);
			if(status == 0) //shutdown
				break;
			else if(status == -1){
				printf("error command!\n");
			}
		}
	}
	return 0;
}