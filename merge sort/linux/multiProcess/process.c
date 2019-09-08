// C program to implement concurrent merge sort
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <sys/vtimes.h>
 


static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
//get CPU percent used by process
void init(){
    FILE* file;
    struct tms timeSample;
    char line[128];

    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    file = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while(fgets(line, 128, file) != NULL){
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file);
}

double getCurrentValue(){
    struct tms timeSample;
    clock_t now;
    double percent;

    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        percent = (timeSample.tms_stime - lastSysCPU) +
            (timeSample.tms_utime - lastUserCPU);
        percent /= (now - lastCPU);
        percent /= numProcessors;
        percent *= 100;
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    return percent;
}

//get RAM used by process
int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

void merge(int a[], int l1, int h1, int h2);
 
void mergeSort(int a[], int l, int h)
{
    int i, len=(h-l+1);
 
    // Using insertion sort for small sized array
    if(l<h){
 
    pid_t lpid,rpid;
    lpid = fork();
    if (lpid<0)
    {
        // Lchild proc not created
        perror("Left Child Proc. not created\n");
        _exit(-1);
    }
    else if (lpid==0)
    {
        mergeSort(a,l,l+len/2-1);
        _exit(0);
    }
    else
    {
        rpid = fork();
        if (rpid<0)
        {
            // Rchild proc not created
            perror("Right Child Proc. not created\n");
            _exit(-1);
        }
        else if(rpid==0)
        {
            mergeSort(a,l+len/2,h);
            _exit(0);
        }
    }
 
    int status;
 
    // Wait for child processes to finish
    waitpid(lpid, &status, 0);
    waitpid(rpid, &status, 0);
 
    // Merge the sorted subarrays
    merge(a, l, l+len/2-1, h);
}
}
 
/* Function to sort an array using insertion sort*/
void insertionSort(int arr[], int n)
{
   int i, key, j;
   for (i = 1; i < n; i++)
   {
       key = arr[i];
       j = i-1;
 
       /* Move elements of arr[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
       while (j >= 0 && arr[j] > key)
       {
           arr[j+1] = arr[j];
           j = j-1;
       }
       arr[j+1] = key;
   }
}
 
// Method to merge sorted subarrays
void merge(int a[], int l1, int h1, int h2)
{
    // We can directly copy  the sorted elements
    // in the final array, no need for a temporary
    // sorted array.
    int count=h2-l1+1;
    int sorted[count];
    int i=l1, k=h1+1, m=0;
    while (i<=h1 && k<=h2)
    {
        if (a[i]<a[k])
            sorted[m++]=a[i++];
        else if (a[k]<a[i])
            sorted[m++]=a[k++];
        else if (a[i]==a[k])
        {
            sorted[m++]=a[i++];
            sorted[m++]=a[k++];
        }
    }
 
    while (i<=h1)
        sorted[m++]=a[i++];
 
    while (k<=h2)
        sorted[m++]=a[k++];
 
    int arr_count = l1;
    for (i=0; i<count; i++,l1++)
        a[l1] = sorted[i];
}
 
// To check if array is actually sorted or not
void isSorted(int arr[], int len)
{
    if (len==1)
    {

    int j=0;
    printf("sorted array is:\n");
    for(int j=0;j<len ;j++){
    printf("%d\t",arr[j]);
         }
        printf("Sorting Done Successfully\n");
        return;
    }
 
    int i;
    for (i=1; i<len; i++)
    {
        if (arr[i]<arr[i-1])
        {
            printf("Sorting Not Done\n");
            return;
        }
    }
    
     int j=0;
    printf("sorted array is:\n");
    for(int j=0;j<len ;j++){
    printf("%d\t",arr[j]);
         }
   
    printf("Sorting Done Successfully\n");
    return;
}
 
// To fill randome values in array for testing
// purpise
void fillData(int a[], int len)
{
    // Create random arrays
    int i;
    printf("\n");
    printf("random array is");
    for (i=0; i<len; i++)
       { a[i] = rand();
    printf("%d\t",a[i]);
    }  
     printf("\n");
    return;
}
 
// Driver code
int main()
{   clock_t begin =clock();
    int shmid;
    key_t key = IPC_PRIVATE;
    int *shm_array;
 
 
    
    int length = 1000000;

    // Calculate segment length
    size_t SHM_SIZE = sizeof(int)*length;
 
    // Create the segment.
    if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        _exit(1);
    }
 
    // Now we attach the segment to our data space.
    if ((shm_array = shmat(shmid, NULL, 0)) == (int *) -1)
    {
        perror("shmat");
        _exit(1);
    }
 
    // Create a random array of given length
    srand(time(NULL));
    fillData(shm_array, length);
 
    // Sort the created array
    mergeSort(shm_array, 0, length-1);
 
    // Check if array is sorted or not
    isSorted(shm_array, length);
 
    /* Detach from the shared memory now that we are
       done using it. */
    if (shmdt(shm_array) == -1)
    {
        perror("shmdt");
        _exit(1);
    }

    /* Delete the shared memory segment. */
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {  
        perror("shmctl");
        _exit(1);
    }
     clock_t end =clock();
     double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("duratio time is :%f\n",time_spent);
    printf("Virtual Memory currently used by current process: %f\n ",getValue());
    printf("CPU currently used by current process: %lf\n",getCurrentValue());
    return 0;
}
