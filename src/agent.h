#pragma once
#include <algorithm>
#include <vector>
#include <random>
#include <bitset>
#include <unordered_map>
#include <iterator>
#include <unistd.h>
#include "board.h"
#include "action.h"
#include "utilities.h"
#include "tuple.h"
#include "mcts.h"
typedef std::bitset<256> bs256;

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
    RandomAgent() : Agent(), dis(0.0, 1.0) { engine.seed(rd()); }
    virtual ~RandomAgent() {}

protected:
    std::random_device rd;
    std::default_random_engine engine;
    std::uniform_real_distribution<> dis;
};

class TrainingPlayer : public RandomAgent {
public:
    TrainingPlayer(unsigned color, Tuple *tuple, float epsilon = 0.9) :
        RandomAgent(),
        color(color),
        tuple(tuple),
        epsilon(epsilon) { repetition.reserve(400); }

    std::string role() { return color ? "White" : "Black"; }

    virtual void open_episode() {
        record.clear();
        repetition.clear();
    }

    virtual void close_episode(const std::string& flag = "") {
        float result = std::stof(flag);
        // the first record is done by black, then alter
        for (Board i : record) {
            tuple->train_weight(i, result, 0);
            result *= -1;
        }
    }

public:
    // use MCTS in training
    virtual Action take_action(const Board& before) {
        Board tmp = Board(before);
        MCTS mcts(tuple, true, 1600, rd(), epsilon);
        std::pair<std::string, unsigned> prev_action = mcts.training(tmp, color, 1);
        record.emplace_back(tmp.get_board(0 ^ color), tmp.get_board(1 ^ color));

        std::string type = prev_action.first;
        unsigned code = prev_action.second;
        if (type == "eat") {
            return Action::Eat(code);
        }
        if (type == "move") {
            if(set_repitition(before, tmp) > 2) return Action();
            return Action::Move(code);
        }
        // cannot find valid action
        return Action();
    }

private:
    int set_repitition(const Board& before, const Board& after) {
        bs256 tmpbs = (bs256(before.get_board(1)) << 192) |
                      (bs256(before.get_board(0)) << 128) |
                      (bs256(before.get_board(1)) << 64)  |
                      bs256(before.get_board(0));

        auto iter = repetition.find(tmpbs);
        if (iter == repetition.end()) {
            repetition.insert({ tmpbs, 1 });
            return 1;
        }
        return ++(iter->second);
    }

private:
    unsigned color; // 0 for black or 1 for white
    std::vector<Board> record;
    std::unordered_map<bs256,int> repetition;
    Tuple *tuple;
    float epsilon;
};

class TuplePlayer : public RandomAgent {
public:
    TuplePlayer(Tuple *tuple) : RandomAgent(), tuple(tuple) {}

public:
    // choose best action with tuple value
    void playing(Board &board, int player) {
        std::vector<unsigned> eats, moves;
        eats.clear(); moves.clear();
        board.get_possible_eat(eats, player);
        board.get_possible_move(moves, player);

        float best_value = -1e9;
        unsigned best_code = 0;
        int best_action_type;

        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
            float value = tuple->get_board_value(tmp, player);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 0;
            }
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);
            float value = tuple->get_board_value(tmp, player);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 1;
            }
        }

        if (best_code != 0) {
            if (!best_action_type) 
                board.eat(best_code & 0b111111, (best_code >> 6) & 0b111111);
            else
                board.move(best_code & 0b111111, (best_code >> 6) & 0b111111);
        }
    }

private:
    Tuple *tuple;
};

class RandomPlayer : public RandomAgent {
public:
    RandomPlayer() : RandomAgent() {}
    RandomPlayer(int seed) : RandomAgent() { engine.seed(seed); }

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
