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
    virtual ~Agent() {}
    virtual void open_episode() {}
    virtual void close_episode() {}
    virtual Action take_action(const Board& b) { return Action(); }
    virtual bool check_for_win(const Board& b) { return b.game_over(); }
};

class RandomAgent : public Agent {
public:
    RandomAgent() : Agent(), dis(0.0, 1.0) { engine.seed(197); }
    virtual ~RandomAgent() {}

protected:
    std::default_random_engine engine;
    std::uniform_real_distribution<> dis;
};

/**
 * dummy player
 * select a legal random action
 */
class TrainingPlayer : public RandomAgent {
public:
    TrainingPlayer(unsigned color, Tuple *tuple) : RandomAgent(), color(color), tuple(tuple) {}
    std::string role() { return color ? "White" : "Black"; }

    virtual void open_episode() {
        record.clear();
        epsilon = 0.7;
    }

    virtual void close_episode() {
        Board last = record.back();
        int black_bitcount = Bitcount(last.get_board(0));
        int white_bitcount = Bitcount(last.get_board(1));

        float result;
        //std::cout<<black_bitcount<<" "<<white_bitcount<<std::endl;
        if      (black_bitcount < white_bitcount)   result = -1.0f;
        else if (black_bitcount == white_bitcount)  result = 0.0f;
        else                                        result = 1.0f;

        for (int i = record.size() - 1; i >= 0; i--) {
            tuple->train_weight(record[i], result);
        }
    }

public:
    virtual Action take_action(const Board& before) {
        std::vector<unsigned> moves;
        std::vector<unsigned> eats;
        before.get_possible_eat(eats, color);
        before.get_possible_move(moves, color);

        // exploitation - choose best action with highest value
        if (dis(engine) < epsilon) {
            float best_value = -1e9;
            unsigned best_code = 0;
            Board best_state;
            int best_action_type;

            for (unsigned code : eats) {
                Board tmp = Board(before);
                tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
                float value = tuple->get_board_value(tmp, color);
                if (value > best_value) {
                    best_value = value;
                    best_code = code;
                    best_state = tmp;
                    best_action_type = 0;
                }
            }
            for (unsigned code : moves) {
                Board tmp = Board(before);
                tmp.move(code & 0b111111, (code >> 6) & 0b111111);
                float value = tuple->get_board_value(tmp, color);
                if (value > best_value) {
                    best_value = value;
                    best_code = code;
                    best_state = tmp;
                    best_action_type = 1;
                }
            }
            if (best_code != 0) {
                record.emplace_back(best_state);
                if (!best_action_type)  return Action::Eat(best_code);
                else                    return Action::Move(best_code);
            }       
        }
        // exploration - random play
        else {
            Board tmp = Board(before);
            std::shuffle(eats.begin(), eats.end(), engine);
            std::shuffle(moves.begin(), moves.end(), engine);

            int size1 = eats.size(), size2 = moves.size();
            if (dis(engine) * (size1 + size2) < size1) {
                if (eats.size() > 0) {
                    tmp.eat(eats[0] & 0b111111, (eats[0] >> 6) & 0b111111);
                    record.emplace_back(tmp);
                    return Action::Eat(eats[0]);
                }
            }
            else {
                if (moves.size() > 0) {
                    tmp.move(moves[0] & 0b111111, (moves[0] >> 6) & 0b111111);
                    record.emplace_back(tmp);
                    return Action::Move(moves[0]);
                }
            }
        }

        return Action();
    }

private:
    unsigned color; // 0 for black or 1 for white
    float epsilon;
    std::vector<Board> record;
    Tuple *tuple;
};
