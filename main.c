#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <signal.h>

#define INTERVAL 3

struct pstat {
    long unsigned int utime_ticks;
    long int cutime_ticks;
    long unsigned int stime_ticks;
    long int cstime_ticks;
    long unsigned int vsize; // virtual memory size in bytes
    long unsigned int rss; //Resident  Set  Size in bytes
};

double getMemoryInfo()
{
    FILE *meminfo = fopen("/proc/meminfo", "r");
    double totalMemory = 0;
    if(meminfo == NULL)
    {
        exit(-1);
    }
    char buff[256];
    while(fgets(buff, sizeof(buff), meminfo))
    {
        int ramKB;
        if(sscanf(buff, "MemTotal: %d kB", &ramKB) == 1)
        {
            totalMemory = ramKB*1024.0;
        }
    }
    if(fclose(meminfo) != 0)
    {
        exit(-1);
    }
    return totalMemory;
}

int get_usage(const pid_t pid, struct pstat* result) {

    char pid_s[20];
    snprintf(pid_s, sizeof(pid_s), "%d", pid);

    char stat_filepath[30] = "/proc/"; strncat(stat_filepath, pid_s,
            sizeof(stat_filepath) - strlen(stat_filepath) -1);
    strncat(stat_filepath, "/stat", sizeof(stat_filepath) -
            strlen(stat_filepath) -1);

    FILE *fpstat = fopen(stat_filepath, "r");
    if (fpstat == NULL) {
        return -1;
    }
    
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

double getCpu(int pid, struct pstat prev){
		struct pstat curr;
		double cpu;

        if( get_usage(pid, &curr) == -1 ) {
            return 0;
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

double getMemoria(int mypid){

	double memoria=0;
	FILE *mypidstat = NULL;
    char filename[100] = {0};
    snprintf(filename, sizeof(filename), "/proc/%d/statm", mypid);
    
    mypidstat = fopen(filename, "r");
    if (mypidstat == NULL) {
        return memoria;
    }

    int i = 0;
    int ret = 0;
    unsigned long long val = 0;
    char strval1[100] = {0};
    char strval2[100] = {0};

    // il parametro di interesse si trova alla riga 6
    for (i = 0; i < 6; i++) {
        ret = fscanf(mypidstat, "%lld ", &val);
        if (i == 5) {
            memoria+=val;
        }
    }
    fclose(mypidstat);
    //trasformo la memoria da pagine a bytes
    memoria*=getpagesize();
    //trasformo la memoria in percentuale
    memoria= (memoria/getMemoryInfo()) * 100;
	
	return memoria;
}

int getprocSize(){
	int size = 0;
	DIR *dpproc;
    struct dirent *dirp;

    errno = 0;
    
    char* proc = "/proc";
    if ((dpproc = opendir(proc)) == NULL) {
        switch (errno) {
            case EACCES: printf("Permesso negato\n"); break;
            case ENOENT: printf("La directory non esiste\n"); break;
            case ENOTDIR: printf("'%s' non ?? una directory\n", proc); break;
        }
        exit(EXIT_FAILURE);
    }

    errno = 0;
    
    //scorro nella directory /proc fino al primo processo
    while ((dirp = readdir(dpproc)) != NULL){
    	if(strcmp(dirp->d_name,"thread-self")==0) break;
    }
    
    while ((dirp = readdir(dpproc)) != NULL){
    	size++;
    }
    
    if (errno != 0) {
        if (errno == EBADF)
            printf("Descrittore della directory non valido\n");
        else
            perror("readdir");
    }

    if (closedir(dpproc) == -1)
        perror("closedir");
    
    return size;
}

int main(int argc, char *argv[]){
L:
    DIR *dpproc;
    struct dirent *dirp;

    errno = 0;
    
    char* proc = "/proc";
    if ((dpproc = opendir(proc)) == NULL) {
        switch (errno) {
            case EACCES: printf("Permesso negato\n"); break;
            case ENOENT: printf("La directory non esiste\n"); break;
            case ENOTDIR: printf("'%s' non ?? una directory\n", proc); break;
        }
        exit(EXIT_FAILURE);
    }

    errno = 0;
    
    int procsize = getprocSize();
    
    struct pstat* arrayprev = (struct pstat*) malloc(procsize*sizeof(struct pstat));
    int array_size = 0;
    int* arraypid = (int*) malloc(procsize*sizeof(int));
    int pid_size = 0;
    
    printf("STAMPO I VALORI DI MEMORIA E CPU DEI PROCESSI NELLA CARTELLA /PROC\n\n");
    
    //scorro nella directory /proc fino al primo processo
    while ((dirp = readdir(dpproc)) != NULL){
    	if(strcmp(dirp->d_name,"thread-self")==0) break;
    }
    //accedo ad ogni directory pid una alla volta
    while ((dirp = readdir(dpproc)) != NULL){
    	
    	DIR *dppid; 
    	
  	    char* pid;
  	    pid = creaDirectory(proc,dirp);
    	
    	errno = 0;
    	
    	if ((dppid = opendir(pid)) == NULL) {
        	switch (errno) {
        	    case EACCES: printf("Permesso negato\n"); break;
        	    case ENOENT: printf("La directory non esiste\n"); break;
        	    case ENOTDIR: printf("'%s' non ?? una directory\n", pid); break;
        	}
        	exit(EXIT_FAILURE);
    	}
    	
    	errno = 0;
    	
    	struct pstat prev;
    	struct tms t;
    	times( &t );
    	
    	if( get_usage(atoi(dirp->d_name), &prev) == -1 ) {
            printf( "errore\n" );
        }
        
        //inserisco tutti i valori prev e pid dentro all'array 
        arrayprev[array_size] = prev;
        array_size++;
        arraypid[pid_size] = atoi(dirp->d_name);
        pid_size++;
    		
    	if (closedir(dppid) == -1)
        	perror("closedir");
        	
        free(pid);
        
    }
    
    //chiusura cartella proc
    if (errno != 0) {
        if (errno == EBADF)
            printf("Descrittore della directory non valido\n");
        else
            perror("readdir");
    }

    if (closedir(dpproc) == -1)
        perror("closedir");
        
    //stampa valori memoria e cpu
    //attendo 3 secondi
    sleep(INTERVAL);
    
    for(int i = 0; i<array_size;i++){
    	double cpu = getCpu(arraypid[i],arrayprev[i]);
    	double memoria = getMemoria(arraypid[i]);
    	printf("%d		%%Memoria: %.01f		%%CPU: %.01f\n",arraypid[i],memoria,cpu);
    }
    free(arraypid);
    free(arrayprev);
    printf("\n");
    //gestione dei comandi
    while(1){
    	char* comando = (char*)malloc(10*sizeof(char));
    	int x;
    	printf("Inserisci un comando: ");
    	scanf("%s",comando);
    	if(strcmp(comando,"update")==0){
    		printf("\nAGGIORNO I PROCESSI\n");
    		free(comando);
    		goto L;
    	}
    	else if(strcmp(comando,"h")==0){
    		printf("\nupdate - aggiorna i processi\nterminate - termina il processo inserito da input\nkill - uccide il processo inserito da input\nsuspend - sospende il processo inserito da input\nresume - riattiva il processo inserito da input\n\n");
    		free(comando);
    	}
    	else if(strcmp(comando,"q")==0){
    		printf("TERMINO IL PROGRAMMA...\n");
    		free(comando);
    		break;
    	}
    	else if(strcmp(comando,"terminate")==0){
    		int ret;
    		printf("Inserisci il numero del processo da terminare: ");
    		scanf("%d",&x);
    		ret = kill(x, SIGTERM);
    		if(ret==-1){ 
    			printf("il processo %d non esiste, ?? gi?? stato terminato o non pu?? essere terminato\n\n",x);
    			free(comando);
    			continue;
    		}
    		free(comando);
    		printf("il processo %d ?? stato terminato\n\n",x);
    	}
    	else if(strcmp(comando,"kill")==0){
    		int ret;
    		printf("Inserisci il numero del processo da interrompere: ");
    		scanf("%d",&x);
    		ret = kill(x, SIGKILL);
    		if(ret==-1){ 
    			printf("il processo %d non esiste, ?? gi?? stato interrotto o non pu?? essere interrotto\n\n",x);
    			free(comando);
    			continue;
    		}
    		free(comando);
    		printf("il processo %d ?? stato interrotto\n\n",x);
    	}
    	else if(strcmp(comando,"suspend")==0){
    		int ret;
    		printf("Inserisci il numero del processo da sospendere: ");
    		scanf("%d",&x);
    		ret = kill(x, SIGSTOP);
    		if(ret==-1){ 
    			printf("il processo %d non esiste, ?? gi?? stato sospeso o non pu?? essere sospeso\n\n",x);
    			free(comando);
    			continue;
    		}
    		free(comando);
    		printf("il processo %d ?? stato sospeso\n\n",x);
    	}
    	else if(strcmp(comando,"resume")==0){
    		int ret;
    		printf("Inserisci il numero del processo da riprendere: ");
    		scanf("%d",&x);
    		ret = kill(x, SIGCONT);
    		if(ret==-1){ 
    			printf("il processo %d non esiste, ?? gi?? stato ripreso o non pu?? essere ripreso\n\n",x);
    			free(comando);
    			continue;
    		}
    		printf("il processo %d ?? stato ripreso\n\n",x);
    		free(comando);
    	}
    	else{
    		printf("comando non riconosciuto, premi 'h' per visualizzare i comandi disponibili\n");
    		free(comando);
    	}
    }
    exit(EXIT_SUCCESS);
}
