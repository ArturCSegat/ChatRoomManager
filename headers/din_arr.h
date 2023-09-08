#ifndef DIN_ARR 
#define DIN_ARR 

typedef struct {
    int * arr;
    int len;
    int cap;
}din_arr;

din_arr * new_din_arr(int cap);

void free_din_arr(din_arr * da);

void append(din_arr * da, int item);

void pop(din_arr * da, int idx);

void print_dinarr(din_arr da);

#endif
