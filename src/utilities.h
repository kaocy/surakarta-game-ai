#pragma once

unsigned bsf_table[64];

void bsf_table_init() {
    for (unsigned i = 0; i < 64; i++)
        bsf_table[((1ULL << i) * 0x218A392CD3D5DBFULL) >> 58] = i;
}

uint64_t lsb(uint64_t x) {
    return x & (-x);
}

unsigned lsb_index(uint64_t x) {
    return bsf_table[(lsb(x) * 0x218A392CD3D5DBFULL) >> 58];
}

unsigned msb_index(uint64_t x) {
    for (unsigned i = 54; i >= 9; i--) {
        if (x & (1ULL << i))    return i;
    }
    return 0;
}

int Bitcount(uint64_t b) {
    b = (b & 0x5555555555555555) + ((b >> 1) & 0x5555555555555555);
    b = (b & 0x3333333333333333) + ((b >> 2) & 0x3333333333333333);
    return (((b + (b >> 4)) & 0x0f0f0f0f0f0f0f0f) * 0x0101010101010101) >> 56;
}