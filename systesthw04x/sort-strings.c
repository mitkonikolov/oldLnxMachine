#include<stdlib.h>
#include<stdio.h>
#include<alloca.h>
#include<string.h>

// Usend strcmp this function compares two strings.
// Because string compare works with void types,
// strcmpimproved first ensures that both of the args
// are strings
int strcmpimproved(const void* a, const void* b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2);
}

int main(int argc, char *argv[]) {
    int temp2 = 4;
    char **array=malloc(temp2*sizeof(char*));
    int i=0;
    
    while(!feof(stdin)) {
        array[i]=alloca(80*sizeof(char));
        if(fgets(array[i], 80, stdin)!=NULL) {
            if(i==(temp2-1)) {
                temp2=temp2+1;
                array=realloc(array, temp2*sizeof(char*));
            }
            i=i+1;
        }
        else {
            break;
        }
    }
    
    qsort(array, i, sizeof(char *), strcmpimproved);
    
    for(int p=0; p <= i; p++) {
        printf("%s", (char *)array[p]);
    }
    
    free(array);
    
    return 0;
}
