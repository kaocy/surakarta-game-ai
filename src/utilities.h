#pragma once

unsigned bsf_table[64];

void bsf_table_init() {
    for (unsigned i = 0; i < 64; i++)
        bsf_table[((1ULL << i) * 0x218A392CD3D5DBFULL) >> 58] = i;
}

uint64_t lsb(uint64_t x) {
    return x & (-x);
}
/**
 *Built-in Function: int __builtin_popcount (unsigned int x)

    Returns the number of 1-bits in x. 
 * 
 * Built-in Function: int __builtin_clz (unsigned int x)

    Returns the number of leading 0-bits in x, starting at the most significant bit position. If x is 0, the result is undefined. 
 *  
 * Built-in Function: int __builtin_ffs (int x)

    Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero. 
 */
inline int lsb_index(uint64_t x) {
    return __builtin_ffsll(x)-1;
    /*
    return bsf_table[(lsb(x) * 0x218A392CD3D5DBFULL) >> 58];
    */
}

inline int msb_index(uint64_t x) {
    if(x == 0) return -1;
    else return 64-__builtin_clzll(x);
    /*
    for (unsigned i = 54; i >= 9; i--) {
        if (x & (1ULL << i))    return i;
    }
    return 0;
    */
}

inline int Bitcount(uint64_t b) {
    return __builtin_popcountll(b);
    /*
    b = (b & 0x5555555555555555) + ((b >> 1) & 0x5555555555555555);
    b = (b & 0x3333333333333333) + ((b >> 2) & 0x3333333333333333);
    return (((b + (b >> 4)) & 0x0f0f0f0f0f0f0f0f) * 0x0101010101010101) >> 56;
    */
}