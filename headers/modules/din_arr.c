#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../din_arr.h"

din_arr * new_din_arr(int cap){
    din_arr * da = (din_arr*)malloc(sizeof(din_arr));

    da->arr = (int*)malloc(sizeof(int) * cap);
    da->len = 0;
    da->cap = cap;

    return da;
}

void free_din_arr(din_arr * da) {
    free(da->arr);
    free(da);
}

void append(din_arr * da, int item) {
    if (da->len == da->cap) {
        da->cap *= 1.5;
        da->arr = (int*)realloc(da->arr, da->cap);
    }
    da->arr[da->len] = item;
    da->len += 1;
}

void pop(din_arr * da, int idx) {
    for (int i = idx; i < da->len - 1; i++){
        int temp = da->arr[i];
        da->arr[i] = da->arr[i+1];
        da->arr[i+1] = temp; 
    }
    da->len--;
}

void print_dinarr(din_arr da) {
    printf("[");
    for (int i = 0; i < da.len; i++) {
        printf("%d, ", da.arr[i]);
    }
    printf("]\n");
}

str_din_arr * new_str_din_arr(int cap){
    str_din_arr * da = (str_din_arr*)malloc(sizeof(str_din_arr));

    da->arr = (char**)malloc(sizeof(char*) * cap);
    for (int i = 0; i < cap; i++){
        da->arr[i] = (char*)malloc(sizeof(char) * 20);
    }
    da->len = 0;
    da->cap = cap;

    return da;
}

void free_str_din_arr(str_din_arr * da) {
    free(da->arr);
    free(da);
}

void append_str(str_din_arr * da, char * str, int len) {
    if (da->len == da->cap) {
        da->cap *= 1.5;
        da->arr = (char**)realloc(da->arr, da->cap * sizeof(char*));
        for (int i = da->len; i < da->cap; i++){
            da->arr[i] = (char*)malloc(sizeof(char) * 20);
        }
    }

    if (len > 20) {
        len = 20;
    }
    
    memset(da->arr[da->len], 0, 20);

    int i;
    for (i = 0; i < len; i++){
        da->arr[da->len][i] = str[i];
    }
    da->len += 1;
}

void pop_str(str_din_arr * da, int idx) {
    for (int i = idx; i < da->len - 1; i++){
        char * temp = da->arr[i];
        da->arr[i] = da->arr[i+1];
        da->arr[i+1] = temp; 
    }
    da->len--;
}

void print_strdinarr(str_din_arr da) {
    printf("[");
    for (int i = 0; i < da.len; i++) {
        printf("%s, ", da.arr[i]);
    }
    printf("]\n");
}

int index_of_str(str_din_arr * da, const char * str) {
    int idx = -1;
    for (int i = 0; i < da->len; i++) {
        if (!strncmp(da->arr[i], str, strlen(str))) {
            idx = i;
            break;
        }
    }    
    
    return idx;
}
