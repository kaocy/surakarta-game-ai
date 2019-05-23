#include <iostream>
#include <iterator>
#include <fstream>
#include <thread>
#include <mutex>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "utilities.h"
#include "mcts.h"

const std::string PLAYER[] = {"MCTS_tuple1", "MCTS_tuple2", "tuple1", "tuple2"};
const std::string SIMULATION[] = {"(random)", "(eat-first)", "(tuple)"};
std::mutex mtx;
int fight_black_win, fight_white_win;

void fight_thread(int player1, int player2, int sim1, int sim2, Tuple *tuple, Tuple *tuple2, int game_count, int seeds) {
    MCTS mcts_tuple(tuple, true, seeds), mcts(tuple2, false, seeds);
    TuplePlayer tuple_player(tuple), tuple2_player(tuple2);
    
    int black_win = 0, white_win = 0;
    for (int i = 0; i < game_count; i++) {
        // std::cout << i << std::endl;
        Board board;
        int color = 0, count = 0, current, sim;

        while (!board.game_over() && count++ < 200) {
            current = color ? player2 : player1;
            sim = color ? sim2 : sim1;
            switch (current) {
                case 0:
                    mcts_tuple.playing(board, color, sim);
                    break;
                case 1:
                    mcts.playing(board, color, sim);
                    break;
                case 2:
                    tuple_player.playing(board, color);
                    break;
                case 3:
                    tuple2_player.playing(board, color);
                    break;
                default:
                    break;
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
    mtx.lock();
    fight_black_win += black_win;
    fight_white_win += white_win;
    mtx.unlock();
    // std::cout << black_win << " " << white_win << std::endl;
}

void fight(int player1, int player2, int sim1, int sim2, Tuple *tuple, Tuple *tuple2, int game_count) {
    /** 
     * player 
     * 0 : MCTS with tuple
     * 1 : MCTS
     * 2 : tuple
     * 3 : eat first
     *
     * simulation
     * 0 : random
     * 1 : eat first
     * 2 : tuple
     */
    std::cout << PLAYER[player1];
    if (player1 <= 1)   std::cout << SIMULATION[sim1];
    std::cout << " VS " << PLAYER[player2];
    if (player2 <= 1)   std::cout << SIMULATION[sim2];
    std::cout << std::endl;

    fight_black_win = 0, fight_white_win = 0;
    std::vector<std::thread> threads;
    for(int i = 0; i < 5; i++) {
        threads.push_back(std::thread(fight_thread, player1, player2, sim1, sim2, tuple, tuple2, game_count / 5, i));
    }
    for (auto& th : threads) {
        th.join();
    }

    std::cout << "Playing " << game_count << " episodes: \n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Black: " << fight_black_win * 100.0 / (fight_black_win + fight_white_win) << " %" << std::endl;
    std::cout << "White: " << fight_white_win * 100.0 / (fight_black_win + fight_white_win) << " %\n" << std::endl;
}

int main(int argc, const char* argv[]) {
    std::cout << "Surakarta: ";
    std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
    std::cout << std::endl << std::endl;

    size_t game_count = 50;
    int sim1 = 1, sim2 = 1;
    std::string play1_args, play2_args, tuple_args, tuple2_args;
    for (int i = 1; i < argc; i++) {
        std::string para(argv[i]);
        
        if (para.find("--game=") == 0) {
            game_count = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--play1=") == 0) {
            play1_args = para.substr(para.find("=") + 1);
        } else if (para.find("--play2=") == 0) {
            play2_args = para.substr(para.find("=") + 1);
        } else if (para.find("--sim1=") == 0) {
            sim1 = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--sim2=") == 0) {
            sim2 = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--tuple=") == 0) {
            tuple_args = para.substr(para.find("=") + 1);
        } else if (para.find("--tuple2=") == 0) {
            tuple2_args = para.substr(para.find("=") + 1);
        }
    }

    Tuple tuple(tuple_args), tuple2(tuple2_args);
    time_t now;
    // after training some episodes, test playing result
    time(&now); printf("%s",ctime(&now));
    fight(0, 1, sim1, sim2, &tuple, &tuple2, game_count);
    time(&now); printf("%s",ctime(&now));
    fight(1, 0, sim2, sim1, &tuple, &tuple2, game_count);
    time(&now); printf("%s",ctime(&now));
    fight(2, 3, sim1, sim2, &tuple, &tuple2, game_count);
    time(&now); printf("%s",ctime(&now));
    fight(3, 2, sim2, sim1, &tuple, &tuple2, game_count);
    time(&now); printf("%s",ctime(&now));
    fight(0, 2, sim1, sim2, &tuple, &tuple2, game_count);
    time(&now); printf("%s",ctime(&now));
    fight(2, 0, sim2, sim1, &tuple, &tuple2, game_count);
    time(&now); printf("%s",ctime(&now));
    return 0;
}
