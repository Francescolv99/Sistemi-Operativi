#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

int getMemoria(char* pid){
	
	int memoria = 0;
	//creo le due stringhe che rappresentano i percorsi dei file
	char* clear_refs = (char*) malloc(sizeof(pid)+ 10*sizeof(char));
	char* smaps = (char*) malloc(sizeof(pid)+ 5*sizeof(char));
	strcat(clear_refs,pid);
	strcat(clear_refs,"/clear_refs");
	strcat(smaps,pid);
	strcat(smaps,"/smaps");
    
    errno = 0;
    
    FILE *fptr;
    char c;
  
    // Open file
    fptr = fopen(smaps, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
  
    // Read contents from file
    c = fgetc(fptr);
    while (c != EOF)
    {
        printf ("%c", c);
        c = fgetc(fptr);
    }
  
    fclose(fptr);
    return 0;
    
	return memoria;
}

int main(int argc, char *argv[]){
    DIR *dpproc;
    struct dirent *dirp;

    errno = 0;
    
    char* proc = "/proc";
    if ((dpproc = opendir(proc)) == NULL) {
        switch (errno) {
            case EACCES: printf("Permission denied\n"); break;
            case ENOENT: printf("Directory does not exist\n"); break;
            case ENOTDIR: printf("'%s' is not a directory\n", proc); break;
        }
        exit(EXIT_FAILURE);
    }

    errno = 0;
    //scorro nella directory /proc fino al primo processo
    while ((dirp = readdir(dpproc)) != NULL){
    	if(strcmp(dirp->d_name,"thread-self")==0) break;
    }
    //stampo tutte le directory dei processi con il proprio pid
    while ((dirp = readdir(dpproc)) != NULL){
    	printf("%s\n", dirp->d_name);
    	
    	//accedo ad ogni directory pid una alla volta
    	
    	struct dirent *dirppid;
    	DIR *dppid;
    	
    	char* pid = 0;
  	    pid = (char*) malloc(sizeof(proc)+(6*sizeof(char))+sizeof(dirp->d_name));
  	    strcat(pid,proc); 
  	    strcat(pid,"/");
  	    strcat(pid,dirp->d_name);
    	
    	if ((dppid = opendir(pid)) == NULL) {
        	switch (errno) {
        	    case EACCES: printf("Permission denied\n"); break;
        	    case ENOENT: printf("Directory does not exist\n"); break;
        	    case ENOTDIR: printf("'%s' is not a directory\n", pid); break;
        	}
        	exit(EXIT_FAILURE);
    	}
    	
    	//ciclo sui file della directory pid
    	while ((dirppid = readdir(dppid)) != NULL){
    		printf("%s\n", dirppid->d_name);
    		if(dirppid->d_name!="clear_refs") continue;
    		else if (dirppid->d_name=="clear_refs"){
    			int memoria = getMemoria(pid);
    		} 
    		free(pid);
    	}
    		
    	
    	if (closedir(dppid) == -1)
        perror("closedir"); 
    }

    if (errno != 0) {
        if (errno == EBADF)
            printf("Invalid directory stream descriptor\n");
        else
            perror("readdir");
    } else {
        printf("End-of-directory reached\n");
    }

    if (closedir(dpproc) == -1)
        perror("closedir");

    exit(EXIT_SUCCESS);
}
