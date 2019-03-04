#include <iostream>
#include <iterator>
#include <fstream>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "utilities.h"
#include "mcts.h"

void fight(int player1, int player2, Tuple *tuple) {
    /** 
     * player 
     * 1 : MCTS with tuple
     * 2 : MCTS
     * 3 : tuple
     * 4 : eat first
     */
    MCTS mcts_tuple(tuple, true), mcts(tuple, false);
    TuplePlayer tuple_player(tuple);
    RandomPlayer random_player;
    
    int black_win = 0, white_win = 0;

    for (int i = 0; i < 50; i++) {
        std::cout << i << std::endl;
        Board board;
        int color = 0, count = 0;

        while (!board.game_over() && count++ < 200) {
            if (color == 0) {
                switch (player1) {
                    case 1:
                        mcts_tuple.playing(board, color);
                        break;
                    case 2:
                        mcts.playing(board, color);
                        break;
                    case 3:
                        tuple_player.playing(board, color);
                        break;
                    case 4:
                        random_player.playing(board, color);
                        break;
                    default:
                        break;
                }
            }
            else {
                switch (player1) {
                    case 1:
                        mcts_tuple.playing(board, color);
                        break;
                    case 2:
                        mcts.playing(board, color);
                        break;
                    case 3:
                        tuple_player.playing(board, color);
                        break;
                    case 4:
                        random_player.playing(board, color);
                        break;
                    default:
                        break;
                }
            }
            color ^= 1; // toggle color
        }

        int black_bitcount = Bitcount(board.get_board(0));
        int white_bitcount = Bitcount(board.get_board(1));
        if      (black_bitcount == 0) white_win++;
        else if (white_bitcount == 0) black_win++;
        else {
            if (black_bitcount > white_bitcount)    black_win++;
            if (black_bitcount < white_bitcount)    white_win++;
        }
    }

    std::cout << "Playing 50 episodes: \n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Black: " << black_win * 100.0 / (black_win + white_win) << " %" << std::endl;
    std::cout << "White: " << white_win * 100.0 / (black_win + white_win) << " %\n" << std::endl;
}

int main(int argc, const char* argv[]) {
    std::cout << "Surakarta: ";
    std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
    std::cout << std::endl << std::endl;

    size_t total = 1000, block = 0, limit = 0;
    std::string play1_args, play2_args;
    std::string load, save;
    bool summary = false;
    for (int i = 1; i < argc; i++) {
        std::string para(argv[i]);
        if (para.find("--total=") == 0) {
            total = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--block=") == 0) {
            block = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--limit=") == 0) {
            limit = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--play1=") == 0) {
            play1_args = para.substr(para.find("=") + 1);
        } else if (para.find("--play2=") == 0) {
            play2_args = para.substr(para.find("=") + 1);
        } else if (para.find("--load=") == 0) {
            load = para.substr(para.find("=") + 1);
        } else if (para.find("--save=") == 0) {
            save = para.substr(para.find("=") + 1);
        } else if (para.find("--summary") == 0) {
            summary = true;
        }
    }

    Tuple tuple;
    Statistic stat(total, block, limit);
    TrainingPlayer play1(0, &tuple), play2(1, &tuple);

    if (load.size()) {
        std::ifstream in(load, std::ios::in);
        in >> stat;
        in.close();
        summary |= stat.is_finished();
    }

    while (!stat.is_finished()) {     
        play1.open_episode();
        play2.open_episode();
        stat.open_episode(play1.role() + ":" + play2.role());
        Episode& game = stat.back();

        while (true) {
            TrainingPlayer& who = game.take_turns(play1, play2);
            Action action = who.take_action(game.state());

            if (game.apply_action(action) != true) break;
            if (who.check_for_win(game.state())) break;
        }

        TrainingPlayer& win = game.last_turns(play1, play2);

        play1.close_episode();
        play2.close_episode();
        stat.close_episode(win.role());

        // after training some episodes, test playing result
        if (stat.episode_count() % 50000 == 0) {
            fight(3, 4, &tuple);
        }
    }

    if (summary) {
        stat.summary();
    }

    if (save.size()) {
        std::ofstream out(save, std::ios::out | std::ios::trunc);
        out << stat;
        out.close();
    }

    return 0;
}
