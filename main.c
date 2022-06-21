#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

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
    	
    	struct dirent *dirpmemoria;
    	DIR *dpmemoria;
    	
  	    char* memoria = (char*) malloc(sizeof(proc)+(6*sizeof(char))+sizeof(dirp->d_name));
  	    strcat(memoria,proc); 
  	    strcat(memoria,"/");
  	    printf("%s\n",memoria);
  	    strcat(memoria,dirp->d_name);
    	strcat(memoria,"/stat");
    	
    	if ((dpmemoria = opendir(memoria)) == NULL) {
        	switch (errno) {
        	    case EACCES: printf("Permission denied\n"); break;
        	    case ENOENT: printf("Directory does not exist\n"); break;
        	    case ENOTDIR: printf("'%s' is not a directory\n", memoria); break;
        	}
        	exit(EXIT_FAILURE);
    	}
    	
    	while ((dirpmemoria = readdir(dpmemoria)) != NULL)
    		printf(" : %s\n", dirpmemoria->d_name);
    		
    	
    	if (closedir(dpmemoria) == -1)
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
