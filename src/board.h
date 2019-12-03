#pragma once
#include <array>
#include <iostream>
#include <vector>
#include "utilities.h"

#define F_LAYER 0x0055005500550055ULL
#define S_LAYER 0x0000333300003333ULL
#define T_LAYER 0x000000000F0F0F0FULL
#define BORDER  0xFF818181818181FFULL

/**
 * bitboard for Surakarta
 *  (0)  (1)  (2)  (3)  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11) (12) (13) (14) (15)
 * (16) (17) (18) (19) (20) (21) (22) (23)
 * (24) (25) (26) (27) (28) (29) (30) (31)
 * (32) (33) (34) (35) (36) (37) (38) (39)
 * (40) (41) (42) (43) (44) (45) (46) (47)
 * (48) (49) (50) (51) (52) (53) (54) (55)
 * (56) (57) (58) (59) (60) (61) (62) (63)
 */

class Board {
public:
    typedef uint64_t data;

public:
    Board() : board_white(0x007E7E0000000000ULL), board_black(0x00000000007E7E00ULL) {}
    Board(data black, data white) : board_white(white), board_black(black) {}
    Board(const Board& b) = default;
    Board& operator =(const Board& b) = default;
    bool operator !=(const Board& b) {
        return !((board_white == b.board_white) &&
                 (board_black == b.board_black));
    }

    void set_black(data black)  { board_black = black; }
    void set_white(data white)  { board_white = white; }

    data& get_board(unsigned int i) {
        return (i) ? board_white : board_black;
    }
    const data& get_board(unsigned int i) const {
        return (i) ? board_white : board_black;
    }

    const bool game_over() const {
        return !board_white || !board_black;
    }

public:
    void get_possible_eat(std::vector<unsigned> &eats, int color) const {
        Board::data mine = color ? board_white : board_black;
        Board::data theirs = (color ^ 1) ? board_white : board_black;
        Board::data occupied = mine | theirs | BORDER;
        Board::data empty = ~occupied;

        eats.clear();
        for (auto &cc : CIRCLE) {
            unsigned cc_cir[4][2] = {{ 0 }};
            // choose the eater position (initial section in 1 of 4)
            for (auto *i = &cc[21]; i != &cc[24]; i++) {
                if (empty & (1ULL << *i)) continue;
                // which square are eater and it's belonging (add the 7th bit, 1 mine or 0 theirs)
                cc_cir[3][0] = *i | ((mine & (1ULL << *i)) >> (*i - 7));
            }
            for (unsigned j = 0; j < 4; j++) {
                unsigned &old_eater = cc_cir[(j + 3) & 3][0]; // same as cc_cir[(j-1+4) % 4][0]
                unsigned &eatee = cc_cir[j][1];
                unsigned &eater = cc_cir[j][0];
                for (auto *i = &cc[j * 6]; i != &cc[(j + 1) * 6]; i++) {
                    if (empty & (1ULL << *i)) continue;
                    eater = *i | ((mine & (1ULL << *i)) >> (*i - 7));
                    // check the eatee and take the square at cross into consideration
                    if (eatee == 0 && *i != (old_eater & 0b111111))
                        eatee = eater;
                }
            }
            // select eater
            for (unsigned i = 0; i < 4; i++) {
                if ((cc_cir[i][0] & (1 << 7)) == 0) continue; // only check if mine piece is eater
                // select eatee
                for (unsigned j = 1; j < 4; j++) {
                    unsigned row = (i + j) & 3;
                    if(cc_cir[row][1] == 0) continue;     // the line is consider empty
                    if(cc_cir[row][1] & (1 << 7)) break;  // the eatee is mine
                    unsigned code = (cc_cir[i][0] & 0b111111) | ((cc_cir[row][1] & 0b111111) << 6);
                    eats.push_back(code);
                    break;
                }
            }
        }
    }

    void get_possible_move(std::vector<unsigned> &moves, int color) const {
        Board::data mine = color ? board_white : board_black;
        Board::data theirs = (color ^ 1) ? board_white : board_black;
        Board::data occupied = mine | theirs | BORDER;
        Board::data empty = ~occupied;

        moves.clear();
        for (unsigned i = 9; i < 55; i++) {
            if (!(mine & (1ULL << i)))   continue;
            for (const unsigned &j : NEIGHBOR) {
                if (empty & (1ULL << (i - j))) moves.push_back(((i - j) << 6) | i);
                if (empty & (1ULL << (i + j))) moves.push_back(((i + j) << 6) | i);
            }
        }
    }

public:
    int eat(unsigned origin, unsigned destination) {
        board_white ^= 1ULL << destination;
        board_black ^= 1ULL << destination;
        board_white &= ~(1ULL << origin);
        board_black &= ~(1ULL << origin);
        return 1;
    }

    int move(unsigned origin, unsigned destination) {
        board_white |= ((board_white >> origin) & 1) << destination;
        board_black |= ((board_black >> origin) & 1) << destination;
        board_white &= ~(1ULL << origin);
        board_black &= ~(1ULL << origin);
        return 1;
    }

public:
    void rotate(int r = 1) {
        switch (((r % 4) + 4) % 4) {
            default:
            case 0: break;
            case 1: board_operation(1, 8); break; // rotate right
            case 2: board_operation(9, 7); break; // reverse
            case 3: board_operation(8, -1);break; // rotate left
        }
    }

    void transpose() {
        board_operation(0, 7);
    }

    //rotate then transpose
    void rotate_tran(int r = 1) {
        switch (((r % 4) + 4) % 4) {
            default:
            case 0: board_operation(0, 7); break; // same as transpose
            case 1: board_operation(8, 8); break;
            case 2: board_operation(9, 0); break;
            case 3: board_operation(1, -1);break;
        }
    }

protected:
    /**
     * Board Operation
     *
     * A.The whole 8*8 board could be devided into four 4*4 boards, 
     *   and each of them contains four 2*2 boards. Show as below.
     * 
     *     (0)  (1) |  (2)  (3)  
     *     (8)  (9) | (10) (11)    <- top-left 4*4 board
     *    --------------------
     *    (16) (17) | (18) (19)
     *    (24) (25) | (26) (27)
     * 
     * B.The 8*8 board clockwise rotation could be seen as a total of the three actions below:
     *    First, rotate the cells in every 2*2 board clockwise.
     *       (8)  (0) | (10)  (2)  
     *       (9)  (1) | (11)  (3)
     *      --------------------
     *      (24) (16) | (26) (18)
     *      (25) (17) | (27) (19)
     *
     *    Second, move the position of four 2*2 clockwise in every 4*4 boards. 
     *      (24) (16) |  (8)  (0)
     *      (25) (17) |  (9)  (1)
     *      --------------------
     *      (26) (18) | (10)  (2)
     *      (27) (19) | (11)  (3)
     *
     *    Last, move the four 4*4 clockwise in the 8*8 board
     *      -The 4*4 above is now at top-right of the 8*8 board
     * 
     * C.On bitboard, the first action could be done by the union of four cells bit shifting.
     *
     * D.Furthermore, if we view a 2*2 as a cell, the second action is similar to the first with
     *   two times the shifting digits at the corresponding position.As well as the third with 
     *   4*4 taken. That is, the position of the cell (no matter it is a 1*1, 2*2, or 4*4) is the  
     *   multiplicand of the digits to shift, while the size is the multiplier.
     *
     * E.The digits shift of four positions are required parameters. However, the cell diagonal
     *   with will always be the same, so two parameters are enough.
     *
     * F.The operation of transposing, rotating several times, and the merge of them are similar.
     *   So all of them can be done in the three actions (three bit-operations) with two parameters.
     */

    void board_operation(const int& a, const int& b) {
        if (b >= 0) {
            board_white = ((board_white &  F_LAYER       ) << a |
                           (board_white & (F_LAYER <<  1)) << b |
                           (board_white & (F_LAYER <<  8)) >> b |
                           (board_white & (F_LAYER <<  9)) >> a);

            board_white = ((board_white &  S_LAYER       ) << (a << 1) |
                           (board_white & (S_LAYER <<  2)) << (b << 1) |
                           (board_white & (S_LAYER << 16)) >> (b << 1) |
                           (board_white & (S_LAYER << 18)) >> (a << 1));

            board_white = ((board_white &  T_LAYER       ) << (a << 2) |
                           (board_white & (T_LAYER <<  4)) << (b << 2) |
                           (board_white & (T_LAYER << 32)) >> (b << 2) |
                           (board_white & (T_LAYER << 36)) >> (a << 2));

            board_black = ((board_black &  F_LAYER       ) << a |
                           (board_black & (F_LAYER <<  1)) << b |
                           (board_black & (F_LAYER <<  8)) >> b |
                           (board_black & (F_LAYER <<  9)) >> a);

            board_black = ((board_black &  S_LAYER       ) << (a << 1) |
                           (board_black & (S_LAYER <<  2)) << (b << 1) |
                           (board_black & (S_LAYER << 16)) >> (b << 1) |
                           (board_black & (S_LAYER << 18)) >> (a << 1));

            board_black = ((board_black &  T_LAYER       ) << (a << 2) |
                           (board_black & (T_LAYER <<  4)) << (b << 2) |
                           (board_black & (T_LAYER << 32)) >> (b << 2) |
                           (board_black & (T_LAYER << 36)) >> (a << 2));
        }
        else {
            board_white = ((board_white &  F_LAYER       ) <<  a |
                           (board_white & (F_LAYER <<  1)) >> -b |
                           (board_white & (F_LAYER <<  8)) << -b |
                           (board_white & (F_LAYER <<  9)) >> a);

            board_white = ((board_white &  S_LAYER       ) << ( a << 1) |
                           (board_white & (S_LAYER <<  2)) >> (-b << 1) |
                           (board_white & (S_LAYER << 16)) << (-b << 1) |
                           (board_white & (S_LAYER << 18)) >> ( a << 1));

            board_white = ((board_white &  T_LAYER       ) << ( a << 2) |
                           (board_white & (T_LAYER <<  4)) >> (-b << 2) |
                           (board_white & (T_LAYER << 32)) << (-b << 2) |
                           (board_white & (T_LAYER << 36)) >> ( a << 2));

            board_black = ((board_black &  F_LAYER       ) <<  a |
                           (board_black & (F_LAYER <<  1)) >> -b |
                           (board_black & (F_LAYER <<  8)) << -b |
                           (board_black & (F_LAYER <<  9)) >> a);

            board_black = ((board_black &  S_LAYER       ) << ( a << 1) |
                           (board_black & (S_LAYER <<  2)) >> (-b << 1) |
                           (board_black & (S_LAYER << 16)) << (-b << 1) |
                           (board_black & (S_LAYER << 18)) >> ( a << 1));

            board_black = ((board_black &  T_LAYER       ) << ( a << 2) |
                           (board_black & (T_LAYER <<  4)) >> (-b << 2) |
                           (board_black & (T_LAYER << 32)) << (-b << 2) |
                           (board_black & (T_LAYER << 36)) >> ( a << 2));
        }
    }

private:
    data board_white;
    data board_black;
};
