#ifndef MATH_H
#define MATH_H
#include "types.h"
// #include "math.c"

static uint64_t DE_BRUIJN_MAGIC = 0x37E84A99DAE458FULL;

static int DE_BRUIJN_MAGIC_TABLE [] = {
    0, 1, 17, 2, 18, 50, 3, 57,
    47, 19, 22, 51, 29, 4, 33, 58,
    15, 48, 20, 27, 25, 23, 52, 41,
    54, 30, 38, 5, 43, 34, 59, 8,
    63, 16, 49, 56, 46, 21, 28, 32,
    14, 26, 24, 40, 53, 37, 42, 7,
    62, 55, 45, 31, 13, 39, 36, 6,
    61, 44, 12, 35, 60, 11, 10, 9,
};

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static inline uint64_t set_bit(uint64_t board, int index){
    return board | (uint64_t)1 << index;
}

static inline bool check_bit(uint64_t board, int index){
    return (board & (uint64_t)1 << index);
}


// this is less efficient, also checks if the bit is 1
static inline uint64_t remove_bit(uint64_t board, int index){
    if (check_bit(board, index)){
        return board ^ ((uint64_t)1 << index);
    } else {
        return board;
    }
}

static inline uint64_t toggle_bit(uint64_t board, int index){
    return board ^ ((uint64_t)1 << index);
}

static inline int bit_count(uint64_t x) {
    return __builtin_popcountll(x);
}
static inline int bit_scan_forward(uint64_t * b){
    return __builtin_ctzll(*b);
}
static inline int bit_scan_backward(uint64_t * b){
    return __builtin_clzll(*b);
}


static inline int pop_lsb(uint64_t * b){
    // if (!b) return 0;
    uint64_t bb = *b;
    int index = __builtin_ctzll(bb);
    *b = bb & (bb - 1);
    return index;
}




#endif
