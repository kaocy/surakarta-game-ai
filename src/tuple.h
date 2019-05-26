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
        for (int i = 0; i < 21; i++) {
            // 3^16 = 43046721;
            outer.emplace_back(43046721);
            small.emplace_back(43046721);
            large.emplace_back(43046721);
        }
    }

    void load_weights(const std::string& path) {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in.is_open()) std::exit(-1);
        uint32_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));

        outer.resize(size / 3); 
        for (Weight& w : outer) in >> w;
        small.resize(size / 3);
        for (Weight& w : small) in >> w;
        large.resize(size / 3);
        for (Weight& w : large) in >> w;

        in.close();
    }

    void save_weights(const std::string& path) {
        std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) std::exit(-1);
        uint32_t size = outer.size() * 3;
        out.write(reinterpret_cast<char*>(&size), sizeof(size));
        for (Weight& w : outer) out << w;
        for (Weight& w : small) out << w;
        for (Weight& w : large) out << w;
        out.close();
    }

public:
    void train_weight(const Board &b, float result, int source = 0) {
        // result: 1 black win -1 white win
        if (source == 0) set_board_value(b, result, learning_rate);
        else if (source == 1) set_board_value(b, result, learning_rate * 0.05f);
    }

public:
    float minimax_search(const Board &board, int player, int level, float alp, float bet) {
        std::vector<unsigned> eats, moves;
        eats.clear(); moves.clear();
        board.get_possible_eat(eats, player ^ 1);
        board.get_possible_move(moves, player ^ 1);

        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
            float value;
            if (level <= 1) value = get_board_value(tmp, player ^ 1);
            else value = minimax_search(tmp, player ^ 1, level - 1, -bet, -alp);
            alp = std::max(alp, value);
            if (alp >= bet) return -alp;
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);
            float value;
            if (level <= 1) value = get_board_value(tmp, player ^ 1);
            else value = minimax_search(tmp, player ^ 1, level - 1, -bet, -alp);
            alp = std::max(alp, value);
            if (alp >= bet) return -alp;
        }
        return -alp;
    }

    float get_board_value(const Board &board, const int player) {  // 0 black 1 white
        Board b(board.get_board(0 ^ player), board.get_board(1 ^ player));
        uint32_t o, s, l;
        board_to_tuple(b, o, s, l);
        unsigned outer_head = o >> 27, outer_index = o & ((1 << 27) - 1);
        unsigned small_head = s >> 27, small_index = s & ((1 << 27) - 1);
        unsigned large_head = l >> 27, large_index = l & ((1 << 27) - 1);

        float outer_v = outer[outer_head][outer_index];
        float small_v = small[small_head][small_index];
        float large_v = large[large_head][large_index];
        return (outer_v + small_v + large_v) / 3.0f;
    }

    void set_board_value(const Board &b, float value, float alpha) {
        uint32_t o, s, l;
        board_to_tuple(b, o, s, l);
        unsigned outer_head = o >> 27, outer_index = o & ((1 << 27) - 1);
        unsigned small_head = s >> 27, small_index = s & ((1 << 27) - 1);
        unsigned large_head = l >> 27, large_index = l & ((1 << 27) - 1);

        outer[outer_head][outer_index] += alpha * (value - outer[outer_head][outer_index]);
        small[small_head][small_index] += alpha * (value - small[small_head][small_index]);
        large[large_head][large_index] += alpha * (value - large[large_head][large_index]);
    }

private:
    // 
    void board_to_tuple(const Board &b, uint32_t &outer_bit, uint32_t &small_bit, uint32_t &large_bit) {
        Board::data white = b.get_board(1);
        Board::data black = b.get_board(0);
        uint32_t outer_index = 0, small_index = 0, large_index = 0;

        /**
         * map board black bit [49, 42, 35, 28, 21, 14] to (0x111111) with the order [49, 14, 42, 21, 35, 28] 
         * map board white bit [49, 42, 35, 28, 21, 14] to (0x222222) with the order [49, 14, 42, 21, 35, 28] 
         * map board white bit [54, 45, 36, 27, 18,  9] to (0x888888) with the order [54,  9, 45, 18, 36, 27] 
         * map board black bit [54, 45, 36, 27, 18,  9] to (0x444444) with the order [54,  9, 45, 18, 36, 27] 
         */
        uint64_t head = ((white & 0x0002040810204000ULL) * 0x020004000F000ULL >> 42 & 0x222222ULL) |
                        ((black & 0x0002040810204000ULL) * 0x020004000F000ULL >> 43 & 0x111111ULL) |
                        ((white & 0x0040201008040200ULL) * 0x4000200010E00ULL >> 40 & 0x888888ULL) |
                        ((black & 0x0040201008040200ULL) * 0x4000200010E00ULL >> 41 & 0x444444ULL);

        uint64_t outer_head = (head >> 16) & 0xFF;
        Board outer_board = b;
        convert81(outer_head, outer_board);
        Board::data outer_white = outer_board.get_board(1);
        Board::data outer_black = outer_board.get_board(0);
        for(auto bit : outer_rest) {
            outer_index *= 3;
            outer_index += (outer_white >> (bit-1)) & 2;
            outer_index += (outer_black >> bit) & 1;
        }

        uint64_t small_head = (head >> 8) & 0xFF;
        Board small_board = b;
        convert81(small_head, small_board);
        Board::data small_white = small_board.get_board(1);
        Board::data small_black = small_board.get_board(0);
        for(auto bit : small_rest) {
            small_index *= 3;
            small_index += (small_white >> (bit-1)) & 2;
            small_index += (small_black >> bit) & 1;
        }

        uint64_t large_head = head & 0xFF;
        Board large_board = b;
        convert81(large_head, large_board);
        Board::data large_white = large_board.get_board(1);
        Board::data large_black = large_board.get_board(0);
        for(auto bit : large_rest) {
            large_index *= 3;
            large_index += (large_white >> (bit-1)) & 2;
            large_index += (large_black >> bit) & 1;
        }

        outer_bit = (outer_head << 27) | outer_index;
        small_bit = (small_head << 27) | small_index;
        large_bit = (large_head << 27) | large_index;
    }

private:
    const unsigned outer_rest[16] = { 012, 013, 014, 015, 021, 026, 031, 036, 041, 046, 051, 056, 062, 063, 064, 065 };
    const unsigned small_rest[16] = { 012, 015, 021, 023, 024, 026, 032, 035, 042, 045, 051, 053, 054, 056, 062, 065 };
    const unsigned large_rest[16] = { 013, 014, 023, 024, 031, 032, 035, 036, 041, 042, 045, 046, 053, 054, 063, 064 };

    void convert81(uint64_t &head, Board &b);

private:
    std::vector<Weight> outer, small, large;
    float learning_rate;
};

void Tuple::convert81(uint64_t &head, Board &b) {
    switch (head) {
        // [0000]
        case 0b00000000: head = 0x00; break;

        // [1000] 0100 0010 0001
        case 0b01000000: head = 0x01; break;
        case 0b00010000: b.rotate(3); head = 0x01; break;
        case 0b00000100: b.rotate(2); head = 0x01; break;
        case 0b00000001: b.rotate(1); head = 0x01; break;

        // [2000] 0200 0020 0002
        case 0b10000000: head = 0x02; break;
        case 0b00100000: b.rotate(3); head = 0x02; break;
        case 0b00001000: b.rotate(2); head = 0x02; break;
        case 0b00000010: b.rotate(1); head = 0x02; break;

        // [1100] 0110 0011 1001
        case 0b01010000: head = 0x03; break;
        case 0b00010100: b.rotate(3); head = 0x03; break;
        case 0b00000101: b.rotate(2); head = 0x03; break;
        case 0b01000001: b.rotate(1); head = 0x03; break;

        // [2200] 0220 0022 2002
        case 0b10100000: head = 0x04; break;
        case 0b00101000: b.rotate(3); head = 0x04; break;
        case 0b00001010: b.rotate(2); head = 0x04; break;
        case 0b10000010: b.rotate(1); head = 0x04; break;

        // [1010] 0101
        case 0b01000100: head = 0x05; break;
        case 0b00010001: b.rotate(1); head = 0x05; break;

        // [2020] 0202
        case 0b10001000: head = 0x06; break;
        case 0b00100010: b.rotate(1); head = 0x06; break;

        // [1200] 0120 0012 2001 2100 0210 0021 1002
        case 0b01100000: head = 0x07; break;
        case 0b00011000: b.rotate(3); head = 0x07; break;
        case 0b00000110: b.rotate(2); head = 0x07; break;
        case 0b10000001: b.rotate(1); head = 0x07; break;
        case 0b10010000: b.rotate_tran(3); head = 0x07; break;
        case 0b00100100: b.rotate_tran(2); head = 0x07; break;
        case 0b00001001: b.rotate_tran(1); head = 0x07; break;
        case 0b01000010: b.transpose(); head = 0x07; break;

        // [1020] 0102 2010 0201
        case 0b01001000: head = 0x08; break;
        case 0b00010010: b.rotate(3); head = 0x08; break;
        case 0b10000100: b.rotate(2); head = 0x08; break;
        case 0b00100001: b.rotate(1); head = 0x08; break;

        // [1110] 0111 1011 1101
        case 0b01010100: head = 0x09; break;
        case 0b00010101: b.rotate(3); head = 0x09; break;
        case 0b01000101: b.rotate(2); head = 0x09; break;
        case 0b01010001: b.rotate(1); head = 0x09; break;

        // [2220] 0222 2022 2202
        case 0b10101000: head = 0x0A; break;
        case 0b00101010: b.rotate(3); head = 0x0A; break;
        case 0b10001010: b.rotate(2); head = 0x0A; break;
        case 0b10100010: b.rotate(1); head = 0x0A; break;

        // [1120] 0112 2011 1201 1021 1102 2110 0211
        case 0b01011000: head = 0x0B; break;
        case 0b00010110: b.rotate(3); head = 0x0B; break;
        case 0b10000101: b.rotate(2); head = 0x0B; break;
        case 0b01100001: b.rotate(1); head = 0x0B; break;
        case 0b01001001: b.transpose(); head = 0x0B; break;
        case 0b01010010: b.rotate_tran(3); head = 0x0B; break;
        case 0b10010100: b.rotate_tran(2); head = 0x0B; break;
        case 0b00100101: b.rotate_tran(1); head = 0x0B; break;

        // [2210] 0221 1022 2102 2012 2201 1220 0122
        case 0b10100100: head = 0x0C; break;
        case 0b00101001: b.rotate(3); head = 0x0C; break;
        case 0b01001010: b.rotate(2); head = 0x0C; break;
        case 0b10010010: b.rotate(1); head = 0x0C; break;
        case 0b10000110: b.transpose(); head = 0x0C; break;
        case 0b10100001: b.rotate_tran(3); head = 0x0C; break;
        case 0b01101000: b.rotate_tran(2); head = 0x0C; break;
        case 0b00011010: b.rotate_tran(1); head = 0x0C; break;

        // [1210] 0121 1012 2101
        case 0b01100100: head = 0x0D; break;
        case 0b00011001: b.rotate(3); head = 0x0D; break;
        case 0b01000110: b.rotate(2); head = 0x0D; break;
        case 0b10010001: b.rotate(1); head = 0x0D; break;

        // [2120] 0212 2021 1202
        case 0b10011000: head = 0x0E; break;
        case 0b00100110: b.rotate(3); head = 0x0E; break;
        case 0b10001001: b.rotate(2); head = 0x0E; break;
        case 0b01100010: b.rotate(1); head = 0x0E; break;

        // [1111]
        case 0b01010101: head = 0x0F; break;

        // [2222]
        case 0b10101010: head = 0x10; break;

        // [1112] 1121 1211 2111
        case 0b01010110: head = 0x11; break;
        case 0b01011001: b.rotate(1); head = 0x11; break;
        case 0b01100101: b.rotate(2); head = 0x11; break;
        case 0b10010101: b.rotate(3); head = 0x11; break;

        // [2221] 2212 2122 1222
        case 0b10101001: head = 0x12; break;
        case 0b10100110: b.rotate(1); head = 0x12; break;
        case 0b10011010: b.rotate(2); head = 0x12; break;
        case 0b01101010: b.rotate(3); head = 0x12; break;

        // [1122] 1221 2211 2112
        case 0b01011010: head = 0x13; break;
        case 0b01101001: b.rotate(1); head = 0x13; break;
        case 0b10100101: b.rotate(2); head = 0x13; break;
        case 0b10010110: b.rotate(3); head = 0x13; break;

        // [1212] 2121
        case 0b01100110: head = 0x14; break;
        case 0b10011001: b.rotate(1); head = 0x14; break;

        default: break;
    }
}
