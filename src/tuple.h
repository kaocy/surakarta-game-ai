#pragma once
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <vector>
#include "board.h"
#include "weight.h"

class Tuple {
public:
    Tuple(const std::string& args = "") : learning_rate(0.003f) {
        std::stringstream ss(args);
        for (std::string pair; ss >> pair; ) {
            std::string key = pair.substr(0, pair.find('='));
            std::string value = pair.substr(pair.find('=') + 1);
            meta[key] = { value };
        }
        if (meta.find("alpha") != meta.end())
            learning_rate = float(meta["alpha"]);
        if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
            load_weights(meta["load"]);
        else
            init_weight();
    }
    ~Tuple() {
        if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
            save_weights(meta["save"]);
    }

private:
    typedef std::string key;
    struct value {
        std::string value;
        operator std::string() const { return value; }
        template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
        operator numeric() const { return numeric(std::stod(value)); }
    };
    std::map<key, value> meta;

private:
    void init_weight() {
        // 3^16 = 43046721
        square.emplace_back(43046721);
        small.emplace_back(43046721);
        large.emplace_back(43046721);
    }

    void load_weights(const std::string& path) {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in.is_open()) std::exit(-1);
        uint32_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));

        square.resize(size / 3); 
        for (Weight& w : square) in >> w;
        small.resize(size / 3);
        for (Weight& w : small) in >> w;
        large.resize(size / 3);
        for (Weight& w : large) in >> w;
        in.close();
    }

    void save_weights(const std::string& path) {
        std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) std::exit(-1);
        uint32_t size = square.size() * 3;
        out.write(reinterpret_cast<char*>(&size), sizeof(size));

        for (Weight& w : square) out << w;
        for (Weight& w : small) out << w;
        for (Weight& w : large) out << w;
        out.close();
    }

public:
    /**
     * 0: states in one game
     * 1: states from MCTS nodes
     */
    void train_weight(const Board &b, float result, int source = 0) {
        if (source == 0)      set_board_value(b, result, learning_rate);
        else if (source == 1) set_board_value(b, result, learning_rate * 0.05f);
    }

public:
    float minimax_search(const Board &board, int player, int level, float alp, float bet) {
        std::vector<unsigned> eats, moves;
        eats.clear(); moves.clear();
        board.get_possible_eat(eats, player ^ 1);
        board.get_possible_move(moves, player ^ 1);

        float value;
        for (unsigned code : eats) {  
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, (code >> 6) & 0b111111);

            if (level <= 1) value = get_board_value(tmp, player ^ 1);
            else            value = minimax_search(tmp, player ^ 1, level - 1, -bet, -alp);
            alp = std::max(alp, value);
            if (alp >= bet) return -alp;
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);

            if (level <= 1) value = get_board_value(tmp, player ^ 1);
            else            value = minimax_search(tmp, player ^ 1, level - 1, -bet, -alp);
            alp = std::max(alp, value);
            if (alp >= bet) return -alp;
        }
        return -alp;
    }

    float get_board_value(const Board &board, const int player) {  // 0 black 1 white
        Board b(board.get_board(0 ^ player), board.get_board(1 ^ player));
        uint32_t o, s, l;
        float square_v = 0.0f;
        float small_v = 0.0f;
        float large_v = 0.0f;

        for (int i = 4; i > 0; i--){
            board_to_tuple_index(b, o, s, l);
            square_v += square[0][o];
            small_v += small[0][s];
            large_v += large[0][l];
            b.rotate(1);
        }

        b.transpose();
        for (int i = 4; i > 0; i--) {
            board_to_tuple_index(b, o, s, l);
            square_v += square[0][o];
            small_v += small[0][s];
            large_v += large[0][l];
            b.rotate(1);
        }
        return (square_v + small_v + large_v) / 24.0f;
    }

    void set_board_value(const Board &board, float value, float alpha) {
        Board b(board);
        uint32_t o, s, l;

        for (int i = 4; i > 0; i--) {
            board_to_tuple_index(b, o, s, l);
            square[0][o] += alpha * (value - square[0][o]);
            small[0][s] += alpha * (value - small[0][s]);
            large[0][l] += alpha * (value - large[0][l]);
            b.rotate(1);
        }

        b.transpose();
        for (int i = 4; i > 0; i--) {
            board_to_tuple_index(b, o, s, l);
            square[0][o] += alpha * (value - square[0][o]);
            small[0][s] += alpha * (value - small[0][s]);
            large[0][l] += alpha * (value - large[0][l]);
            b.rotate(1);
        }
    }

private:
    void board_to_tuple_index(const Board &b, uint32_t &square_bit, uint32_t &small_bit, uint32_t &large_bit) {
        const Board::data white = b.get_board(1);
        const Board::data black = b.get_board(0);
        square_bit = 0;
        small_bit = 0;
        large_bit = 0;
        
        for(size_t i = 0; i < 16; i++){
            square_bit *= 3;
            square_bit += (white >> (square_rest[i]-1)) & 2;
            square_bit += (black >> square_rest[i]) & 1;
            small_bit *= 3;
            small_bit += (white >> (small_rest[i]-1)) & 2;
            small_bit += (black >> small_rest[i]) & 1;
            large_bit *= 3;
            large_bit += (white >> (large_rest[i]-1)) & 2;
            large_bit += (black >> large_rest[i]) & 1;
        }
    }

private:
    const unsigned square_rest[16] = { 011, 012, 013, 014, 021, 022, 023, 024,
                                       031, 032, 033, 034, 041, 042, 043, 044 };
    const unsigned small_rest[16] = { 012, 021, 022, 023, 024, 025, 026, 032,
                                      042, 051, 052, 053, 054, 055, 056, 062 };
    const unsigned large_rest[16] = { 013, 023, 031, 032, 033, 034, 035, 036,
                                      041, 042, 043, 044, 045, 046, 053, 063 };

    std::vector<Weight> square, small, large;
    float learning_rate;
};
