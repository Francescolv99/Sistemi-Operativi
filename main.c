#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

char* creaDirectory(char* proc, struct dirent* dirp){
	char* pid2 = malloc(strlen(proc));
  	strcpy(pid2,proc); 
  	strcat(pid2,"/");
  	strcat(pid2,dirp->d_name);
  	return pid2;
}

//DA COMPLETARE
int getMemoria(char* pid){
	
	int memoria = 0;
	//creo le due stringhe che rappresentano i percorsi dei file
	char* clear_refs = (char*) malloc(strlen(pid));
	char* smaps = (char*) malloc(strlen(pid));
	strcpy(clear_refs,pid);
	strcat(clear_refs,"/clear_refs");
	strcpy(smaps,pid);
	strcat(smaps,"/smaps");
    
    errno = 0;
    
    /*FILE *fptr;
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
  
    fclose(fptr);*/
    
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
    	printf("%s", dirp->d_name);
    	
    	//accedo ad ogni directory pid una alla volta
    	
    	DIR *dppid; 
    	
  	    char* pid;
  	    pid = creaDirectory(proc,dirp);
    	
    	errno = 0;
    	
    	if ((dppid = opendir(pid)) == NULL) {
        	switch (errno) {
        	    case EACCES: printf("Permission denied\n"); break;
        	    case ENOENT: printf("Directory does not exist\n"); break;
        	    case ENOTDIR: printf("'%s' is not a directory\n", pid); break;
        	}
        	exit(EXIT_FAILURE);
    	}
    	
    	errno = 0;
    	
    	
    	//calcolo la memoria usata da ogni processo
    	int memoria = getMemoria(pid);
    	//stampo i valori di memoria e CPU
    	printf("	Memoria: %d		CPU: %d\n",memoria,0);
    		
    	if (closedir(dppid) == -1)
        	perror("closedir");
        
        
        
    }

    if (errno != 0) {
        if (errno == EBADF)
            printf("Invalid directory stream descriptor\n");
        else
            perror("readdir");
    }

    if (closedir(dpproc) == -1)
        perror("closedir");

    exit(EXIT_SUCCESS);
}
