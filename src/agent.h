#pragma once
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <random>
#include "board.h"
#include "action.h"
#include "utilities.h"
#include "tuple.h"

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
    Player(unsigned color) : Random_agent(), color(color) {}
    std::string role() { return color ? "White" : "Black"; }

public:
    virtual Action take_action(const Board& before) {
        Board::data mine = before.get_board(color);
        Board::data theirs = before.get_board(color ^ 1);
        Board::data occupied = mine | theirs | Board::BORDER;
        Board::data empty = ~occupied;

        eats.clear();
        moves.clear();

        // eatable
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

        // movable
        for (unsigned i = 9; i < 55; i++) {
            if (!(mine & (1ULL << i)))   continue;
            for (const unsigned &j : NEIGHBOR) {
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
