#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>

#define INTERVAL 3

struct pstat {
    long unsigned int utime_ticks;
    long int cutime_ticks;
    long unsigned int stime_ticks;
    long int cstime_ticks;
    long unsigned int vsize; // virtual memory size in bytes
    long unsigned int rss; //Resident  Set  Size in bytes
};

int get_usage(const pid_t pid, struct pstat* result) {

    //convert  pid to string
    char pid_s[20];
    snprintf(pid_s, sizeof(pid_s), "%d", pid);

    char stat_filepath[30] = "/proc/"; strncat(stat_filepath, pid_s,
            sizeof(stat_filepath) - strlen(stat_filepath) -1);
    strncat(stat_filepath, "/stat", sizeof(stat_filepath) -
            strlen(stat_filepath) -1);

    FILE *fpstat = fopen(stat_filepath, "r");
    if (fpstat == NULL) {
        perror("FOPEN ERROR ");
        return -1;
    }

    //read values from /proc/pid/stat
    bzero(result, sizeof(struct pstat));
    long int rss;
    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu"
                "%lu %ld %ld %*d %*d %*d %*d %*u %lu %ld",
                &result->utime_ticks, &result->stime_ticks,
                &result->cutime_ticks, &result->cstime_ticks, &result->vsize,
                &rss) == EOF) {
        fclose(fpstat);
        return -1;
    }
    fclose(fpstat);
    result->rss = rss * getpagesize();

    return 0;
}

void calc_cpu_usage_pct(const struct pstat* cur_usage, const struct pstat* last_usage, double* usage){
    const long unsigned int pid_diff =
        ( cur_usage->utime_ticks + cur_usage->stime_ticks ) -
        ( last_usage->utime_ticks + last_usage->stime_ticks );
    *usage = 1/(float)INTERVAL * pid_diff;
}

double getCpu(int pid){
		struct pstat prev,curr;
		double cpu;
    	struct tms t;
    	times( &t );
    	
    	if( get_usage(pid, &prev) == -1 ) {
            printf( "error\n" );
        }
        
        sleep(0.5); //se metto a 3 funziona ma è lento, devo trovare un modo per velocizzare
        //potrei salvare tutti i prev in un array e dopo 3 secondi stampare tutti
        //così facendo potrei anche stampare i processi in ordine di cpu usata

        if( get_usage(pid, &curr) == -1 ) {
            printf( "error\n" );
        }
    	calc_cpu_usage_pct(&curr, &prev, &cpu);
    	return cpu;
}

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
L:
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
    	double cpu = getCpu(atoi(dirp->d_name));
    	
    	//stampo i valori di memoria e CPU
    	printf("	Memoria: %d		%%CPU: %.02f\n",memoria,cpu);
    		
    	if (closedir(dppid) == -1)
        	perror("closedir");
        	
        free(pid);
        
        
        
    }
    //gestione dei comandi 
    char* comando = (char*)malloc(4*sizeof(char));
    printf("Inserisci un comando: ");
    scanf("%s",comando);
    
    //aggiornamento dei processi
    if(strcmp(comando,"u\n")){ //funziona con qualsiasi lettera metto, devo cambiare
    printf("\nAGGIORNO I PROCESSI\n");
    	goto L;
    }

	//chiusura cartella proc
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
