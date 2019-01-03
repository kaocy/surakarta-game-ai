#pragma once

// moving position offset from current position(only half)
static const unsigned NEIGHBOR[4] = { 1, 7, 8, 9 };

// position in circles(Octal)
static const unsigned CIRCLE[4][25] = {
    // 0: small_circle
    { 021, 022, 023, 024, 025, 026,
      015, 025, 035, 045, 055, 065,
      056, 055, 054, 053, 052, 051,
      062, 052, 042, 032, 022, 012, 0 },
    // 1: small_circle_re
    { 012, 022, 032, 042, 052, 062,
      051, 052, 053, 054, 055, 056,
      065, 055, 045, 035, 025, 015,
      026, 025, 024, 023, 022, 021, 0 },
    // 2: large_circle
    { 031, 032, 033, 034, 035, 036,
      014, 024, 034, 044, 054, 064,
      046, 045, 044, 043, 042, 041,
      063, 053, 043, 033, 023, 013, 0 },
    // 3: large_circle_re
    { 013, 023, 033, 043, 053, 063,
      041, 042, 043, 044, 045, 046,
      064, 054, 044, 034, 024, 014,
      036, 035, 034, 033, 032, 031, 0 }
};

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
    return __builtin_ffsll(x) - 1;
    // return bsf_table[(lsb(x) * 0x218A392CD3D5DBFULL) >> 58];
}

inline int msb_index(uint64_t x) {
    if(x == 0) return -1;
    else return 63 - __builtin_clzll(x);
    // for (unsigned i = 54; i >= 9; i--) {
    //     if (x & (1ULL << i))    return i;
    // }
    // return 0;
}

inline int Bitcount(uint64_t b) {
    return __builtin_popcountll(b);
    // b = (b & 0x5555555555555555) + ((b >> 1) & 0x5555555555555555);
    // b = (b & 0x3333333333333333) + ((b >> 2) & 0x3333333333333333);
    // return (((b + (b >> 4)) & 0x0f0f0f0f0f0f0f0f) * 0x0101010101010101) >> 56;
}
