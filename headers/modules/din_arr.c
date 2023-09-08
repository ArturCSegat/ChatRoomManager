#include <stdio.h>
#include <stdlib.h>
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
