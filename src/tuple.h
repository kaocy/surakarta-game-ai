#pragma once
#include "board.h"
#include "weight.h"

class Tuples {
public:
    typedef uint32_t rest;
    void board_to_tuples(const Board &b, float &o_, float &m_, float &i_) {
        Board::data white = b.get_board(1);
        Board::data black = b.get_board(0);
        rest outer_r = 0, mider_r = 0,inner_r = 0;
        uint64_t ohead = ((white & 0x42000000004200ULL) * 0x40800000000120ULL >> 57) & 0x55
                       | ((black & 0x42000000004200ULL) * 0x40800000000120ULL >> 56) & 0xAA;
        Board oboard = b;
        convert81(ohead,oboard);
        Board::data owhite = oboard.get_board(1);
        Board::data oblack = oboard.get_board(0);
        for(auto &bit : outer_rest) {
            outer_r *= 3;
            outer_r += (owhite >> (bit-1)) & 2ULL;
            outer_r += (oblack >> bit) & 1ULL;
        }
        uint64_t mhead = ((white & 0x240000240000ULL) * 0x21000000C000ULL >> 57) & 0x55 
                       | ((black & 0x240000240000ULL) * 0x21000000C000ULL >> 56) & 0xAA;
        Board mboard = b;
        convert81(mhead,mboard);
        Board::data mwhite = mboard.get_board(1);
        Board::data mblack = mboard.get_board(0);
        for(auto &bit : mider_rest) {
            mider_r *= 3;
            mider_r += (mwhite >> (bit-1)) & 2ULL;
            mider_r += (mblack >> bit) & 1ULL;
        }
        uint64_t ihead = (white >> 27 & 1ULL) | (white >> 26 & 4ULL) | (white >> 32 & 16ULL) | (white >> 29 & 64ULL)
                       | (black >> 26 & 2ULL) | (black >> 25 & 8ULL) | (black >> 31 & 32ULL) | (black >> 28 & 128ULL);
        Board iboard = b;
        convert81(ihead,iboard);
        Board::data iwhite = iboard.get_board(1);
        Board::data iblack = iboard.get_board(0);
        for(auto &bit : inner_rest) {
            inner_r *= 3;
            inner_r += (iwhite >> (bit-1)) & 2ULL;
            inner_r += (iblack >> bit) & 1ULL;
        }
        o_ = outer[ohead & 15][outer_r] * ((ohead & 16)? -1.f: 1.f);
        m_ = mider[mhead & 15][mider_r] * ((mhead & 16)? -1.f: 1.f);
        i_ = inner[ihead & 15][inner_r] * ((ihead & 16)? -1.f: 1.f);
    }
private:
    
/*  const unsigned outer_head[4] = {011,016,061,066};
    const unsigned mider_head[4] = {022,025,052,055};
    const unsigned inner_head[4] = {033,034,043,044};*/
    const unsigned outer_rest[16] = {012,013,014,015,021,026,031,036,041,046,051,056,062,063,064,065};
    const unsigned mider_rest[16] = {012,015,021,023,024,026,032,035,042,045,051,053,054,056,062,065};
    const unsigned inner_rest[16] = {013,014,023,024,031,032,035,036,041,042,045,046,053,054,063,064};

    void convert81(uint64_t &head, Board &b);
private:
    std::vector<weight> outer, mider, inner;

};

void Tuples::convert81(uint64_t &head, Board &b) {
        
    switch (head)
    {
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
        default:
            break;
    }
}