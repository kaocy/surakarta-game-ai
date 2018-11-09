#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#define F_LAYER 0x0055005500550055ULL
#define S_LAYER 0x0000333300003333ULL
#define T_LAYER 0x000000000F0F0F0FULL
/**
 * 
 * bitboard for Surakarta
 *  (0)  (1)  (2)  (3)  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11) (12) (13) (14) (15)
 * (16) (17) (18) (19) (20) (21) (22) (23)
 * (24) (25) (26) (27) (28) (29) (30) (31)
 * (32) (33) (34) (35) (36) (37) (38) (39)
 * (40) (41) (42) (43) (44) (45) (46) (47)
 * (48) (49) (50) (51) (52) (53) (54) (55)
 * (56) (57) (58) (59) (60) (61) (62) (63)
 * 
 */
 
class board {
public:
    typedef uint64_t data;
public:
    board() : {}

    
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
     *    Second, move the position of four 2*2 clockwise in every 4*4 boards. 
     *      (24) (16) |  (8)  (0)
     *      (25) (17) |  (9)  (1)
     *      --------------------
     *      (26) (18) | (10)  (2)
     *      (27) (19) | (11)  (3)
     *    Last, move the four 4*4 clockwise in the 8*8 board
     *      -The 4*4 above is now at top-right of the 8*8 board
     * 
     * C.On bitboard, the first action could be done by the union of four cells bit shifting.
     * D.Furthermore, if we view a 2*2 as a cell, the second action is similar to the first with
     *   two times the shifting digits at the corresponding position.As well as the third with 
     *   4*4 taken. That is, the position of the cell (no matter it is a 1*1, 2*2, or 4*4) is the  
     *   multiplicand of the digits to shift, while the size is the multiplier.
     * E.The digits shift of four positions are required parameters. However, the cell diagonal
     *   with will always be the same, so two parameters are enough.
     * F.The operation of transposing, rotating several times, and the merge of them are similar.
     *   So all of them can be done in the three actions (three bit-operations) with two parameters.
     */
    void board_operation (const int& a,const int& b) {
        if(b >= 0) {
            board_ = ((board_ &  F_LAYER     )<<a | (board_ & (F_LAYER<< 1))<<b | 
                      (board_ & (F_LAYER<< 8))>>b | (board_ & (F_LAYER<< 9))>>a);
            board_ = ((board_ &  S_LAYER     )<<(a<<1) | (board_ & (S_LAYER<< 2))<<(b<<1) |
                      (board_ & (S_LAYER<<16))>>(b<<1) | (board_ & (S_LAYER<<18))>>(a<<1));
            board_ = ((board_ &  T_LAYER     )<<(a<<2) | (board_ & (T_LAYER<< 4))<<(b<<2) |
                      (board_ & (T_LAYER<<32))>>(b<<2) | (board_ & (T_LAYER<<36))>>(a<<2));
        }else {
            board_ = ((board_ &  F_LAYER     )<< a | (board_ & (F_LAYER<< 1))>>-b | 
                      (board_ & (F_LAYER<< 8))<<-b | (board_ & (F_LAYER<< 9))>> a);
            board_ = ((board_ &  S_LAYER     )<<( a<<1) | (board_ & (S_LAYER<< 2))>>(-b<<1) |
                      (board_ & (S_LAYER<<16))<<(-b<<1) | (board_ & (S_LAYER<<18))>>( a<<1));
            board_ = ((board_ &  T_LAYER     )<<( a<<2) | (board_ & (T_LAYER<< 4))>>(-b<<2) |
                      (board_ & (T_LAYER<<32))<<(-b<<2) | (board_ & (T_LAYER<<36))>>( a<<2));
        }
    }
    void rotate (int r = 1) {
        switch (((r % 4) + 4) % 4) {
            default:
            case 0: break;
            case 1: board_operation(1,8); break;
            case 2: board_operation(9,7); break;
            case 3: board_operation(8,-1);break;
        }
    }

    void transpose (void) {
        board_operation(0,7);
    }
    //rotate then transpose
    void rotate_tran(int r = 1) {
        switch (((r % 4) + 4) % 4) {
            default:
            case 0: board_operation(0,7); break;  //same as transpose
            case 1: board_operation(8,8); break;
            case 2: board_operation(9,0); break;
            case 3: board_operation(1,-1);break;
        }
    }
private:
    data board_;

}
