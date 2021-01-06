#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <unistd.h> 
pthread_t threadid[10];  
struct node 
{ 
    int key; 
    struct node *left, *right; 
}; 
   
struct node *newNode(int newKey) 
{ 
    struct node *t =  (struct node *)malloc(sizeof(struct node)); 
    t->key = newKey; 
    t->left = NULL;
    t->right = NULL; 
    return t; 
} 
void ordernode(struct node *root) 
{ 
    if (root != NULL) 
    { 
        ordernode(root->left); 
        printf("%d \n", root->key); 
        ordernode(root->right); 
    } 
}
int cntIndex=0;
void reversePath(struct node *root, int *arr)
{
	if (root != NULL) 
	{ 
		reversePath(root->left,arr);
		arr[cntIndex++]=root->key; 
		reversePath(root->right,arr); 
	} 
}   
struct node* insert(struct node* node, int key) 
{ 
    if (node == NULL) return newNode(key); //agacin bos olup olmama durumunu denetliyoruz
    if (key <= node->key) 
        node->left  = insert(node->left, key);  // her durum icin saga ve sola node eklemesi yapiyor
    else if (key > node->key) 
        node->right = insert(node->right, key);    
    return node; 
} 
struct node * leftMostNode(struct node* node) // en soldaki node'u bulmamizi sagliyor
{ 

    struct node* current = node; 
  
    while (current && current->left != NULL) 
        current = current->left; 
  
    return current; 
} 
  
struct node* deleteNode(struct node* root, int key) // node silme islemi
{ 

    if (root == NULL) return root; 
  
    if (key < root->key) 
        root->left = deleteNode(root->left, key); 
    else if (key > root->key) 
        root->right = deleteNode(root->right, key); 
    else
    { 
        if (root->left == NULL) 
        { 
            struct node *t = root->right; 
            free(root); 
            return t; 
        } 
        else if (root->right == NULL) 
        { 
            struct node *t = root->left; 
            free(root); 
            return t; 
        } 
  
        struct node* t = leftMostNode(root->right); 
        root->key = t->key; 
        root->right = deleteNode(root->right, t->key); 
    } 
    return root; 
} 
struct node *root = NULL; 
typedef struct{
char * input;
int arr[1]; //arr[0]=k
}params;
pthread_mutex_t lock; 
int counter;

void* operation(void * args)
{	//dosyadan okuma işlemleri//
	params * pr= args;
	int num;
	FILE * fp;
	fp = fopen(pr->input, "r");// read modunda dosyayı açıyoruz
	while(fscanf(fp,"%d",&num) != EOF)//dosyanın sonuna kadar
	{
		pthread_mutex_lock(&lock);//yeni thread gelmemesi için işlem bitene kadar kitliyoruz
		if(counter < pr->arr[0])
		{
			if(counter == 0)
				root = insert(root,num);
			else
				insert(root,num);			
			counter++;
		}
		else
		{
			if(leftMostNode(root)->key < num)
			{
				struct node *min = leftMostNode(root);
				root = deleteNode(root,min->key);
				insert(root,num);		
			}
		}
	
		pthread_mutex_unlock(&lock);//işlem bitince sırayı serbest bırakıyoruz ve yeni thread almasına izin veriyoruz
	} 
	fclose(fp);
	free(pr);
	pthread_exit(NULL);	
}


int main(int argc, char *argv[]) 
{ 	//istenilen analizde süreyi bulmak için
   	struct timeval startTime, endTime;    	
	long elapsedTime;
	int k, n;
	gettimeofday(&startTime, NULL);
	k = atoi(argv[1]);
	n = atoi(argv[2]);
	if(k<100 || k>10000){
		printf("100 ile 10000 arası k değeri giriniz \n");
		exit(1);	
	}
	if(n<1 || n>10){
		printf("1 ile 10 arası n değeri giriniz \n");
		exit(1);	
	}
	if (pthread_mutex_init(&lock, NULL) != 0) { //mutex oluşturuyoruz
        printf("mutex_init başarısız oldu\n"); 
        return 1; 
    	} 
	counter =0;
	
	for(int i=0;i<n;i++){
		char* infile = argv[(3+i)];//1 ve 2 k ve n değerleri olduğundan 3+0 dan başlıyoruz
		params * pr = malloc(sizeof(*pr));//parametrelere dizinde yer açıyoruz	
		pr->input = infile;
		pr->arr[0]= k;
		int r = pthread_create(&(threadid[i]),NULL,operation,pr);//threadleri oluşturuyoruz
		if(r != 0){
			printf("thread oluşturulamadı\n");
			exit(1);
		}				
	}
	for(int l =0; l<n;l++)	pthread_join(threadid[l],NULL);//
	pthread_mutex_destroy(&lock);//kodun devamında kullanılmayacağı için kapatıyoruz
	int * t = (int *)malloc(k*sizeof(int));
	FILE * fp2;
	reversePath(root,t);
	fp2 = fopen(argv[(3+n)], "w");//verilen dosyaları write modunda açıyoruz	
	for(int j=k-1; j>=0;j--) fprintf(fp2,"%d\n",t[j]);
	fclose(fp2);	
	free(t);
	gettimeofday(&endTime, NULL);
	elapsedTime = (endTime.tv_sec - startTime.tv_sec) * 1000000 +endTime.tv_usec - startTime.tv_usec;
	printf("süre: %ldms\n",  elapsedTime);
    return 0; 
} 
