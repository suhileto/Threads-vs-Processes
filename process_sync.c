#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
sem_t * semaphore;
int count = 0;
void printArray(int A[], int size) 
{ 
    int i; 
    for (i=0; i < size; i++) 
        printf("%d ", A[i]); 
    printf("\n"); 
} 
void insertionSort(int arr[], int n) 
{ 
    int i, key, j; 
    for (i = 1; i < n; i++) { 
        key = arr[i]; 
        j = i - 1; 
        while (j >= 0 && arr[j] > key) { 
            arr[j + 1] = arr[j]; 
            j = j - 1; 
        } 
        arr[j + 1] = key; 
    } 
} 

void operation(char * input, const int k)
{
	int num;
	FILE * fp;
	fp = fopen(input, "r");
	const char* name = "OS"; 
	const int SIZE = k;
	int shm_fd;   
	int* ptr; 
	shm_fd = shm_open(name, O_RDWR, 0666);//paylaşılan bellek alanı oluşturuyoruz
    	ptr = mmap(0,SIZE,PROT_WRITE|PROT_READ,MAP_SHARED,shm_fd, 0);//shared memoryi nesnesini adresliyoruz
	while(fscanf(fp,"%d",&num) != EOF)//dosya sonuna kadar okuma işlemi
	{	

		sem_wait(semaphore);//semaphore değeri 0 olana kadar azaltma işlemi gerçekleşir 0 olduğunda ise semaphore artana kadar beklemeye alır
		if(count < k)
		{			
			ptr[count++]=num;
						 
		}
		else
		{					
			insertionSort((int *)ptr,k);
			if(ptr[0]<num)
			{		
				ptr[0]=num;
				count++;				
			}
		}
		sem_post(semaphore);//Semafor değerini arttırıp yeni processin ilgili alana eşimine izin verir(uyandırır)
	} 
	fclose(fp);
}

int main(int argc, char *argv[]) {
	struct timeval startTime, endTime;    	
	long elapsedTime;
	gettimeofday(&startTime, NULL);
	int k, n;

	k = atoi(argv[1]);
	n = atoi(argv[2]);
	if(k<100 || k>10000){
		printf("100 ile 10000 arası k değeri giriniz  \n");
		exit(1);	
	}
	if(n<1 || n>10){
		printf("1 ile 10 arası n değeri giriniz\n");
		exit(1);	
	}
	semaphore = sem_open("/sem",O_RDWR | O_CREAT,0660,1);//semaforu açıyoruz
	if (semaphore < 0){
		perror("Semafor açılamadı\n");
		exit(1);
	}
	int fd;	
	void * sptr;
	const char* name = "OS"; 
	const int SIZE = k*4; 
	fd = shm_open(name, O_RDWR | O_CREAT, 0660);//paylaşılan bellek alanı oluşturuyoruz
	struct stat sbuf;
	fstat(fd, &sbuf);//shared memory alanını kontrol ediyoruz
	ftruncate(fd, SIZE);//memory mappingden önce shared memory boyutunu ayarlıyoruz
	sptr = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);//paylaşılan belleği o anki adrese atıyoruz
	
	int status = 0;
	pid_t pid, wpid;
	for(int i=0;i<n;i++){
		char* infile = argv[(3+i)];
		pid = fork();
		if(pid == 0){
			//printf("child%d \n",i);		
			operation(infile,k);
			//printf("sonrası %d \n", i);
			exit(0);
		}	
	}
	while((wpid = wait(&status))>0);
	close(fd);
	//printf("Parent\n");
	int * p = (int *)sptr;
	insertionSort(p,k);
	//printArray(p,k);
	
	FILE * fpw = fopen(argv[(3+n)], "w");	
	for(int j=k-1; j>=0;j--)
	{
		fprintf(fpw,"%d\n",p[j]);
	}
	fclose(fpw);	
	shm_unlink (name);//name bellek alanına olan bağlantıyı kaldırıyoruz
	sem_unlink("/sem");
	gettimeofday(&endTime, NULL);
	elapsedTime = (endTime.tv_sec - startTime.tv_sec) * 1000000 +
	endTime.tv_usec - startTime.tv_usec;
	printf("sure: %ld\n ms",  elapsedTime);
	return(0);
}



