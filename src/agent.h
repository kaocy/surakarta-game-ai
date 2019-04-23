#pragma once
#include <algorithm>
#include <vector>
#include <random>
#include "board.h"
#include "action.h"
#include "utilities.h"
#include "tuple.h"
#include "mcts.h"

class Agent {
public:
    Agent() {}
    virtual ~Agent() {}
    virtual void open_episode() {}
    virtual void close_episode(const std::string& flag = "") {}
    virtual Action take_action(const Board& b) { return Action(); }
    virtual bool check_for_win(const Board& b) { return b.game_over(); }
};

class RandomAgent : public Agent {
public:
    RandomAgent() : Agent(), dis(0.0, 1.0) { engine.seed(17); }
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
        epsilon = 0.9f;
    }

    virtual void close_episode(const std::string& flag = "") {
        float result = (flag == "Black") ? 1.0f : -1.0f;
        for (int i = record.size() - 1; i >= 0; i--) {
            tuple->train_weight(record[i], result);
        }
    }

public:
    // virtual Action take_action(const Board& before) { // for training
    //     std::vector<unsigned> moves, eats;
    //     before.get_possible_eat(eats, color);
    //     before.get_possible_move(moves, color);

    //     std::shuffle(eats.begin(), eats.end(), engine);
    //     std::shuffle(moves.begin(), moves.end(), engine);

    //     // exploitation - choose best action with highest value
    //     if (dis(engine) < epsilon) {
    //         float best_value = -1e9;
    //         unsigned best_code = 0;
    //         Board best_state;
    //         int best_action_type;
    //         float alp = -1e9, bet = 1e9;
    //         for (unsigned code : eats) {
    //             Board tmp = Board(before);
    //             tmp.eat(code & 0b111111, (code >> 6) & 0b111111);

    //             float value = tuple->minimax_search(tmp, color, 1, -bet, -alp);
    //             if (value > best_value) {
    //                 best_value = value;
    //                 best_code = code;
    //                 best_state = tmp;
    //                 best_action_type = 0;
    //             }
    //             if (best_value > alp) alp = best_value;
    //         }
    //         for (unsigned code : moves) {
    //             Board tmp = Board(before);
    //             tmp.move(code & 0b111111, (code >> 6) & 0b111111);

    //             float value = tuple->minimax_search(tmp, color, 1, -bet, -alp);
    //             if (value > best_value) {
    //                 best_value = value;
    //                 best_code = code;
    //                 best_state = tmp;
    //                 best_action_type = 1;
    //             }
    //             if (best_value > alp) alp = best_value;
    //         }
    //         if (best_code != 0) {
    //             record.emplace_back(best_state);
    //             if (!best_action_type)  return Action::Eat(best_code);
    //             else                    return Action::Move(best_code);
    //         }       
    //     }
    //     // exploration - random play
    //     else {
    //         Board tmp = Board(before);
    //         int size1 = eats.size(), size2 = moves.size();
    //         if (dis(engine) * (size1 + size2) < size1 * 5) {  // eat seems to be TOO important
    //             if (eats.size() > 0) {
    //                 tmp.eat(eats[0] & 0b111111, (eats[0] >> 6) & 0b111111);
    //                 record.emplace_back(tmp);
    //                 return Action::Eat(eats[0]);
    //             }
    //         }
    //         else {
    //             if (moves.size() > 0) {
    //                 tmp.move(moves[0] & 0b111111, (moves[0] >> 6) & 0b111111);
    //                 record.emplace_back(tmp);
    //                 return Action::Move(moves[0]);
    //             }
    //         }
    //     }

    //     return Action();
    // }

    // use MCTS in training
    virtual Action take_action(const Board& before) {
        MCTS mcts(tuple, true);
        Board tmp = Board(before);
        std::pair<std::string, unsigned> prev_action = mcts.training(tmp, color, 1);

        std::string type = prev_action.first;
        unsigned code = prev_action.second;
        if (type == "eat")   return Action::Eat(code);
        if (type == "move")  return Action::Move(code);

        // cannot find valid action
        return Action();
    }

private:
    unsigned color; // 0 for black or 1 for white
    float epsilon;
    std::vector<Board> record;
    Tuple *tuple;
};

class TuplePlayer : public RandomAgent {
public:
    TuplePlayer(Tuple *tuple) : RandomAgent(), tuple(tuple) {}

public:
    void playing(Board &board, int player) {
        // choose best action with tuple value
        std::vector<unsigned> eats, moves;
        eats.clear(); moves.clear();
        board.get_possible_eat(eats, player);
        board.get_possible_move(moves, player);

        float best_value = -1e9;
        unsigned best_code = 0;
        int best_action_type;
        float alp = -1e9, bet = 1e9;
        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
            float value = tuple->minimax_search(tmp, player, 1, -bet, -alp);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 0;
            }
            if (best_value > alp) alp = best_value;
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);
            float value = tuple->minimax_search(tmp, player, 1, -bet, -alp);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 1;
            }
            if (best_value > alp) alp = best_value;
        }
        if (best_code != 0) {
            if (!best_action_type)  board.eat(best_code & 0b111111, (best_code >> 6) & 0b111111);
            else                    board.move(best_code & 0b111111, (best_code >> 6) & 0b111111);
        }
    }

private:
    Tuple *tuple;
};

class RandomPlayer : public RandomAgent {
public:
    RandomPlayer() : RandomAgent() {}

public:
    void playing(Board &board, int player) {
        // random play with eat first
        std::vector<unsigned> eats, moves;
        eats.clear(); moves.clear();
        board.get_possible_eat(eats, player);
        board.get_possible_move(moves, player);

        std::shuffle(eats.begin(), eats.end(), engine);
        std::shuffle(moves.begin(), moves.end(), engine);

        if (eats.size() > 0) {
            board.eat(eats[0] & 0b111111, (eats[0] >> 6) & 0b111111);
        }
        else if (moves.size() > 0) {
            board.move(moves[0] & 0b111111, (moves[0] >> 6) & 0b111111);
        }
    }
};
