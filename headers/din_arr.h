#ifndef DIN_ARR 
#define DIN_ARR 

// genral purpose dinamic array to store ints
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


// genral purpose dinamic array to store strs
typedef struct {
    char ** arr;
    int len;
    int cap;
}str_din_arr;

str_din_arr * new_str_din_arr(int cap);

void free_str_din_arr(str_din_arr * da);

void append_str(str_din_arr * da, char * str, int len);

void pop_str(str_din_arr * da, int idx);

int index_of_str(str_din_arr * da, const char * str);

void print_strdinarr(str_din_arr da);

#endif
