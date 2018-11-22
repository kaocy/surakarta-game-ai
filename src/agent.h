#pragma once
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <random>
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
    Random_agent() : Agent() {}

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
