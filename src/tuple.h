#pragma once
#include "board.h"
#include "weight.h"

class Tuples {
public:
    // 
    void board_to_tuples(const Board &b, float &outer_v, float &small_v, float &large_v) {
        Board::data white = b.get_board(1);
        Board::data black = b.get_board(0);
        uint32_t outer_index = 0, small_index = 0, large_index = 0;

        /**
         * map board white bit [49, 42, 35, 28, 21, 14] to [49, 14, 42, 21, 35, 28] (0x888888)
         * map board black bit [49, 42, 35, 28, 21, 14] to [49, 14, 42, 21, 35, 28] (0x444444)
         * map board white bit [54, 45, 36, 27, 18,  9] to [54,  9, 45, 18, 36, 27] (0x222222)
         * map board black bit [54, 45, 36, 27, 18,  9] to [54,  9, 45, 18, 36, 27] (0x111111)
         */
        uint64_t head = ((white & 0x0002040810204000ULL) * 0x020004000F000ULL >> 40 & 0x888888ULL) | 
                        ((black & 0x0002040810204000ULL) * 0x020004000F000ULL >> 41 & 0x444444ULL) |
                        ((white & 0x0040201008040200ULL) * 0x4000200010E00ULL >> 42 & 0x222222ULL) |
                        ((black & 0x0040201008040200ULL) * 0x4000200010E00ULL >> 43 & 0x111111ULL);

        uint64_t outer_head = head >> 16 & 0xFF;
        Board outer_board = b;
        convert81(outer_head, outer_board);
        Board::data outer_white = outer_board.get_board(1);
        Board::data outer_black = outer_board.get_board(0);
        for(auto bit : outer_rest) {
            outer_index *= 3;
            outer_index += (outer_white >> (bit-1)) & 2;
            outer_index += (outer_black >> bit) & 1;
        }

        uint64_t small_head = (head >> 8) & 0xFF;
        Board small_board = b;
        convert81(small_head, small_board);
        Board::data small_white = small_board.get_board(1);
        Board::data small_black = small_board.get_board(0);
        for(auto bit : small_rest) {
            small_index *= 3;
            small_index += (small_white >> (bit-1)) & 2;
            small_index += (small_black >> bit) & 1;
        }

        uint64_t large_head = head & 0xFF;
        Board large_board = b;
        convert81(large_head, large_board);
        Board::data large_white = large_board.get_board(1);
        Board::data large_black = large_board.get_board(0);
        for(auto bit : large_rest) {
            large_index *= 3;
            large_index += (large_white >> (bit-1)) & 2;
            large_index += (large_black >> bit) & 1;
        }

        outer_v = outer[outer_head & 0xF][outer_index] * ((outer_head & 16) ? -1.0f : 1.0f);
        small_v = small[small_head & 0xF][small_index] * ((small_head & 16) ? -1.0f : 1.0f);
        large_v = large[large_head & 0xF][large_index] * ((large_head & 16) ? -1.0f : 1.0f);
    }

private:
    const unsigned outer_rest[16] = { 012, 013, 014, 015, 021, 026, 031, 036, 041, 046, 051, 056, 062, 063, 064, 065 };
    const unsigned small_rest[16] = { 012, 015, 021, 023, 024, 026, 032, 035, 042, 045, 051, 053, 054, 056, 062, 065 };
    const unsigned large_rest[16] = { 013, 014, 023, 024, 031, 032, 035, 036, 041, 042, 045, 046, 053, 054, 063, 064 };

    void convert81(uint64_t &head, Board &b);

private:
    std::vector<weight> outer, small, large;
};

void Tuples::convert81(uint64_t &head, Board &b) {
    switch (head) {
        // [0000]
        case 0b00000000: head = 0x00; break;

        // [1000] 0100 0010 0001 2000 0200 0020 0002
        case 0b01000000: head = 0x01; break;
        case 0b00010000: b.rotate(3); head = 0x01; break;
        case 0b00000100: b.rotate(2); head = 0x01; break;
        case 0b00000001: b.rotate(1); head = 0x01; break;
        case 0b10000000: head = 0x11; break;
        case 0b00100000: b.rotate(3); head = 0x11; break;
        case 0b00001000: b.rotate(2); head = 0x11; break;
        case 0b00000010: b.rotate(1); head = 0x11; break;

        // [1100] 0110 0011 1001 2200 0220 0022 2002
        case 0b01010000: head = 0x02; break;
        case 0b00010100: b.rotate(3); head = 0x02; break;
        case 0b00000101: b.rotate(2); head = 0x02; break;
        case 0b01000001: b.rotate(1); head = 0x02; break;
        case 0b10100000: head = 0x12; break;
        case 0b00101000: b.rotate(3); head = 0x12; break;
        case 0b00001010: b.rotate(2); head = 0x12; break;
        case 0b10000010: b.rotate(1); head = 0x12; break;

        // [1010] 0101 2020 0202
        case 0b01000100: head = 0x03; break;
        case 0b00010001: b.rotate(1); head = 0x03; break;
        case 0b10001000: head = 0x13; break;
        case 0b00100010: b.rotate(1); head = 0x13; break;

        // [1200] 0120 0012 2001 2100 0210 0021 1002
        case 0b01100000: head = 0x04; break;
        case 0b00011000: b.rotate(3); head = 0x04; break;
        case 0b00000110: b.rotate(2); head = 0x04; break;
        case 0b10000001: b.rotate(1); head = 0x04; break;
        case 0b10010000: head = 0x14; break;
        case 0b00100100: b.rotate(3); head = 0x14; break;
        case 0b00001001: b.rotate(2); head = 0x14; break;
        case 0b01000010: b.rotate(1); head = 0x14; break;

        // [1020] 0102 2010 0201
        case 0b01001000: head = 0x05; break;
        case 0b00010010: b.rotate(3); head = 0x05; break;
        case 0b10000100: b.rotate(2); head = 0x05; break;
        case 0b00100001: b.rotate(1); head = 0x05; break;

        // [1110] 0111 1011 1101 2220 0222 2022 2202
        case 0b01010100: head = 0x06; break;
        case 0b00010101: b.rotate(3); head = 0x06; break;
        case 0b01000101: b.rotate(2); head = 0x06; break;
        case 0b01010001: b.rotate(1); head = 0x06; break;
        case 0b10101000: head = 0x16; break;
        case 0b00101010: b.rotate(3); head = 0x16; break;
        case 0b10001010: b.rotate(2); head = 0x16; break;
        case 0b10100010: b.rotate(1); head = 0x16; break;

        // [1120] 0112 2011 1201 1021 1102 2110 0211 2210 0221 1022 2102 2012 2201 1220 0122
        case 0b01011000: head = 0x07; break;
        case 0b00010110: b.rotate(3); head = 0x07; break;
        case 0b10000101: b.rotate(2); head = 0x07; break;
        case 0b01100001: b.rotate(1); head = 0x07; break;
        case 0b01001001: b.transpose(); head = 0x07; break;
        case 0b01010010: b.rotate_tran(3); head = 0x07; break;
        case 0b10010100: b.rotate_tran(2); head = 0x07; break;
        case 0b00100101: b.rotate_tran(1); head = 0x07; break;
        case 0b10100100: head = 0x17; break;
        case 0b00101001: b.rotate(3); head = 0x17; break;
        case 0b01001010: b.rotate(2); head = 0x17; break;
        case 0b10010010: b.rotate(1); head = 0x17; break;
        case 0b10000110: b.transpose(); head = 0x17; break;
        case 0b10100001: b.rotate_tran(3); head = 0x17; break;
        case 0b01101000: b.rotate_tran(2); head = 0x17; break;
        case 0b00011010: b.rotate_tran(1); head = 0x17; break;

        // [1210] 0121 1012 2101 2120 0212 2021 1202
        case 0b01100100: head = 0x08; break;
        case 0b00011001: b.rotate(3); head = 0x08; break;
        case 0b01000110: b.rotate(2); head = 0x08; break;
        case 0b10010001: b.rotate(1); head = 0x08; break;
        case 0b10011000: head = 0x18; break;
        case 0b00100110: b.rotate(3); head = 0x18; break;
        case 0b10001001: b.rotate(2); head = 0x18; break;
        case 0b01100010: b.rotate(1); head = 0x18; break;

        // [1111] 2222
        case 0b01010101: head = 0x09; break;
        case 0b10101010: head = 0x19; break;

        // [1112] 1121 1211 2111 2221 2212 2122 1222
        case 0b01010110: head = 0x0A; break;
        case 0b01011001: b.rotate(1); head = 0x0A; break;
        case 0b01100101: b.rotate(2); head = 0x0A; break;
        case 0b10010101: b.rotate(3); head = 0x0A; break;
        case 0b10101001: head = 0x1A; break;
        case 0b10100110: b.rotate(1); head = 0x1A; break;
        case 0b10011010: b.rotate(2); head = 0x1A; break;
        case 0b01101010: b.rotate(3); head = 0x1A; break;

        // [1122] 1221 2211 2112
        case 0b01011010: head = 0x0B; break;
        case 0b01101001: b.rotate(1); head = 0x0B; break;
        case 0b10100101: b.rotate(2); head = 0x0B; break;
        case 0b10010110: b.rotate(3); head = 0x0B; break;

        // [1212] 2121
        case 0b01100110: head = 0x0C; break;
        case 0b10011001: head = 0x1C; break;

        default: break;
    }
}
