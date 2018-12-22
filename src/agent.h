#pragma once
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <random>
#include <bitset>
#include "board.h"
#include "action.h"
#include "utilities.h"

class Agent {
public:
    Agent() {}
    virtual Action take_action(const Board& b) { return Action(); }
    virtual bool check_for_win(const Board& b) { return b.game_over(); }

};

class Random_agent : public Agent {
public:
    Random_agent() : Agent() {
        engine.seed(11);
    }

protected:
    std::default_random_engine engine;
};

/**
 * dummy player
 * select a legal random action
 */
class Player : public Random_agent {
public:
    static constexpr unsigned neighbor[4] = {1, 7, 8, 9};
    static constexpr bool clockwise[4] = {true, false, false, true};

public:
    Player(unsigned color) : Random_agent(), color(color) {}
    std::string role() { return color ? "White" : "Black"; }

public:
    virtual Action take_action(const Board& before) {
        Board::data mine = before.get_board(color);
        Board::data theirs = before.get_board(color ^ 1);
        Board::data occupied = mine | theirs | Board::BORDER;
        Board::data empty = ~occupied;

        std::unordered_set<unsigned> record;
        Board::data small_circle[4], big_circle[4];

        small_circle[0] = occupied & 0x00000000007E0000ULL;
        small_circle[1] = occupied & 0x0004040404040400ULL;
        small_circle[2] = occupied & 0x00007E0000000000ULL;
        small_circle[3] = occupied & 0x0020202020202000ULL;
        big_circle[0]   = occupied & 0x000000007E000000ULL;
        big_circle[1]   = occupied & 0x0008080808080800ULL;
        big_circle[2]   = occupied & 0x0000007E00000000ULL;
        big_circle[3]   = occupied & 0x0010101010101000ULL;

        eats.clear();
        moves.clear();

        // eatable
        /*
        for (unsigned i = 9; i < 55; i++) {
            Board::data piece = 1ULL << i;
            if (!(mine & piece))   continue;

            for (int j = 0; j < 4; j++) {
                if (piece & small_circle[j]) {
                    // direction that smaller than piece
                    if (lsb(small_circle[j]) == piece) {
                        for (int k = 1; k <= 4; k++) {
                            int next_index = ((Player::clockwise[j] ? k : -k) + j + 4) % 4;
                            Board::data remove_piece = small_circle[next_index] - (small_circle[next_index] & piece);
                            if (!remove_piece)  continue;

                            if ((Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                (lsb(remove_piece) & theirs)) {
                                unsigned dest = lsb_index(remove_piece);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            if (!(Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                ((remove_piece & theirs) > (remove_piece & mine))) {
                                unsigned dest = msb_index(remove_piece & theirs);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            break;
                        }
                    }

                    // direction that larger than piece
                    if (small_circle[j] < (piece << 1)) {
                        for (int k = 1; k <= 4; k++) {
                            int next_index = ((Player::clockwise[j] ? -k : k) + j + 4) % 4;
                            Board::data remove_piece = small_circle[next_index] - (small_circle[next_index] & piece);
                            if (!remove_piece)  continue;

                            if (!(Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                (lsb(remove_piece) & theirs)) {
                                unsigned dest = lsb_index(remove_piece);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            if ((Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                ((remove_piece & theirs) > (remove_piece & mine))) {
                                unsigned dest = msb_index(remove_piece & theirs);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            break;
                        }
                    }
                }
                if (piece & big_circle[j]) {
                    // direction that smaller than piece
                    if (lsb(big_circle[j]) == piece) {
                        for (int k = 1; k <= 4; k++) {
                            int next_index = ((Player::clockwise[j] ? k : -k) + j + 4) % 4;
                            Board::data remove_piece = big_circle[next_index] - (big_circle[next_index] & piece);
                            if (!remove_piece)  continue;

                            if ((Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                (lsb(remove_piece) & theirs)) {
                                unsigned dest = lsb_index(remove_piece);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            if (!(Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                ((remove_piece & theirs) > (remove_piece & mine))) {
                                unsigned dest = msb_index(remove_piece & theirs);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            break;
                        }
                    }

                    // direction that larger than piece
                    if (big_circle[j] < (piece << 1)) {
                        for (int k = 1; k <= 4; k++) {
                            int next_index = ((Player::clockwise[j] ? -k : k) + j + 4) % 4;
                            Board::data remove_piece = big_circle[next_index] - (big_circle[next_index] & piece);
                            if (!remove_piece)  continue;

                            if (!(Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                (lsb(remove_piece) & theirs)) {
                                unsigned dest = lsb_index(remove_piece);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            if ((Player::clockwise[next_index] ^ Player::clockwise[j]) &&
                                ((remove_piece & theirs) > (remove_piece & mine))) {
                                unsigned dest = msb_index(remove_piece & theirs);
                                unsigned code = (dest << 6) | i;
                                if (!record.count(code)) {
                                    record.insert(code);
                                    eats.push_back(code);
                                }
                            }

                            break;
                        }
                    }
                }
            }
        }
        */
        //eatable 2
        unsigned cir_bit[4][25] = {
            // 0: mider
            {021,022,023,024,025,026 , 015,025,035,045,055,065,
             056,055,054,053,052,051 , 062,052,042,032,022,012,0},
            // 1: mider_re
            {012,022,032,042,052,062 , 051,052,053,054,055,056,
             065,055,045,035,025,015 , 026,025,024,023,022,021,0},
            // 2: inner
            {031,032,033,034,035,036 , 014,024,034,044,054,064,
             046,045,044,043,042,041 , 063,053,043,033,023,013,0},
            // 3: inner_re
            {013,023,033,043,053,063 , 041,042,043,044,045,046,
             064,054,044,034,024,014 , 036,035,034,033,032,031,0}
        };
        for (auto &cc : cir_bit) {
            unsigned cc_cir[4][2] = {0};
            for (unsigned *i = &cc[21]; i != &cc[24]; i++) {
                if (empty & (1ULL << *i)) continue;
                cc_cir[3][0] = *i | ((mine & (1ULL << *i)) >> (*i - 7));  //which square are eater and it's belonging (at the 7th bit, 1 mine & 0 theirs)
            }
            for (unsigned j = 0; j < 4; j++) {
                unsigned &old_eater = cc_cir[(j + 3) & 3][0];
                unsigned &t_eatee = cc_cir[j][1];
                unsigned &t_eater = cc_cir[j][0];
                for (unsigned *i = &cc[j*6]; i != &cc[(j + 1) * 6]; i++) {
                    if (empty & (1ULL << *i)) continue;
                    t_eater = *i | ((mine & (1ULL << *i)) >> (*i - 7));
                    if (t_eatee == 0 && *i != (old_eater & 0b111111)) //check the eatee and take the square at cross into consideration
                        t_eatee = t_eater;
                }
            }
            for (unsigned i = 0; i < 4; i++) {
                if ((cc_cir[i][0] & (1 << 7)) == 0) continue;  //only check if mine piece is eater
                for (unsigned j = 1; j < 4; j++) {
                    unsigned row = (i + j) % 4;
                    if(cc_cir[row][1] == 0) continue;  //the line is consider empty 
                    if(cc_cir[row][1] & (1 << 7)) break;  //the eatee is mine
                    unsigned code = (cc_cir[i][0] & 0b111111) | (cc_cir[row][1] & 0b111111) << 6;
                    eats.push_back(code);
                    std::cout<<std::bitset<12>(code)<<std::endl;
                    break;
                }
            }
        }

        // movable
        for (unsigned i = 9; i < 55; i++) {
            if (!(mine & (1ULL << i)))   continue;
            for (unsigned j : Player::neighbor) {
                if (empty & (1ULL << (i - j))) moves.push_back(((i - j) << 6) | i);
                if (empty & (1ULL << (i + j))) moves.push_back(((i + j) << 6) | i);
            }
        }

        std::shuffle(eats.begin(), eats.end(), engine);
        std::shuffle(moves.begin(), moves.end(), engine);
        if (eats.size() > 0)     return Action::Eat(eats[0]);
        if (moves.size() > 0)    return Action::Move(moves[0]);

        return Action();
    }

private:
    unsigned color; // 0 for white or 1 for black
    std::vector<unsigned> moves;
    std::vector<unsigned> eats;
};
