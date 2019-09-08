#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/times.h>
#include <sys/vtimes.h>
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



typedef struct { int i, j; } pair;
#define MAX 1000
int a[MAX];
pthread_attr_t attr;

void *threaded_merge_sort(void *param) {
    pair range = *(pair *)param, lrange, rrange;
    pthread_t lside, rside;
    int p = range.i, q, r = range.j, n1, n2, n = r-p+1, i, j, k;
    int *aleft, *aright;
    if(p < r) {
        q = (p + r) >> 1;
        lrange.i = p, lrange.j = q, rrange.i = q + 1, rrange.j = r;
        pthread_create(&lside, &attr, threaded_merge_sort, (void *)&lrange);
        pthread_create(&rside, &attr, threaded_merge_sort, (void *)&rrange);
        pthread_join(lside, NULL);
        pthread_join(rside, NULL);
        n1 = q - p + 1, n2 = r - q;
        aleft = (int *)malloc(sizeof(int) * n1);
        aright = (int *)malloc(sizeof(int) * n2);
        for(i = 0; i < n1; i++) aleft[i] = a[p+i];
        for(i = 0; i < n2; i++) aright[i] = a[q+1+i];
        for(k = i = j = 0; k < n; k++) {
            if(i >= n1 || (j < n2 && aleft[i] > aright[j])) a[k+p] = aright[j++];
            else a[k+p] = aleft[i++];
        }
        free(aleft);
        free(aright);
    }
    return NULL;
}

int main() {
    clock_t start =clock();
    int n, i;
    pthread_t sorter;
    pair range;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

         srand(time(NULL));
        for(i = 0; i <  MAX; i++){
        a[i] = rand();
        }

        range.i = 0, range.j = MAX-1;
        pthread_create(&sorter, &attr, threaded_merge_sort, (void *)&range);
        pthread_join(sorter, NULL);
        for(i = 0; i < MAX; i++)
        printf("%d%c", a[i], (i==MAX-1? '\n' : ' '));
        pthread_attr_destroy(&attr);
            clock_t end =clock();
     double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("duratio time is :%f\n",time_spent);

    return 0;
}

