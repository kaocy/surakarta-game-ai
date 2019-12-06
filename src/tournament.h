#pragma once
#include "board.h"
#include "agent.h"
#include "mcts.h"
#include "tuple.h"
#include "utilities.h"

/* command line interface to play with other opponent */
int tournament(int argc, const char* argv[]) {
    std::string tuple_args;
    int we = 1, opponent = 0;
    Board board;

    for (int i = 1; i < argc; i++) {
        std::string para(argv[i]);
        if (para.find("--tuple=") == 0) {
            tuple_args = para.substr(para.find("=") + 1);
        } else if (para.find("--black=") == 0) {
            uint64_t black = std::stoull(para.substr(para.find("=") + 1), NULL, 16);
            board.set_black(black);
        } else if (para.find("--white=") == 0) {
            uint64_t white = std::stoull(para.substr(para.find("=") + 1), NULL, 16);
            board.set_white(white);
        } else if (para.find("--first") == 0) {
            we = 0; opponent = 1;
        }
    }

    // std::cout << std::hex << board.get_board(0) << std::endl;
    // std::cout << std::hex << board.get_board(1) << std::endl;

    Tuple tuple(tuple_args);
    MCTS mcts_tuple(&tuple, true, 1000000);
    int current = 0;

    std::cout << "Start" << std::endl;
    while (true) {
        if (current == opponent) {
            std::cout << "Opponent's turn: ";
            std::string ori_str, dest_str;
            std::cin >> ori_str >> dest_str;
            unsigned ori = (ori_str.at(0) - '0') * 8 + (ori_str.at(1) - 'a' + 1);
            unsigned dest = (dest_str.at(0) - '0') * 8 + (dest_str.at(1) - 'a' + 1);

            uint64_t is_eat = (1ULL << dest) & board.get_board(we);
            if (is_eat) board.eat(ori, dest);
            else        board.move(ori, dest);
        }
        else {
            Board before(board);
            mcts_tuple.playing(board, we, 1);

            uint64_t before_chess = before.get_board(we);
            uint64_t after_chess = board.get_board(we);
            if (before_chess == after_chess) {
                std::cout << "oops! cannot move" << std::endl;
                break;
            }

            uint64_t ori = (before_chess ^ after_chess) & before_chess;
            uint64_t dest = (before_chess ^ after_chess) & after_chess;
            std::string type = (dest & before.get_board(opponent)) ? "eat" : "move";

            std::string ori_str = std::to_string(lsb_index(ori) / 8) +
                                  char(lsb_index(ori) % 8 + 'a' - 1);
            std::string dest_str = std::to_string(lsb_index(dest) / 8) +
                                   char(lsb_index(dest) % 8 + 'a' - 1);
            std::cout << type << " " << ori_str << " " << dest_str << std::endl;
        }

        if (board.game_over()) {
            if (current == opponent)    std::cout << "We lose!" << std::endl;
            else                        std::cout << "We win!" << std::endl;
            break;
        }
        current ^= 1;
    }
    return 0;
}