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
    Player play1(0, &tuple), play2(1, &tuple);
    MCTS mcts(&tuple);

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
            Player& who = game.take_turns(play1, play2);
            Action action = who.take_action(game.state());

            if (game.apply_action(action) != true) break;
            if (who.check_for_win(game.state())) break;
        }

        Player& win = game.last_turns(play1, play2);

        play1.close_episode();
        play2.close_episode();
        stat.close_episode(win.role());

        // after training some episodes, test MCTS playing result
        // if (stat.episode_count() % 1000 == 0) {
        //     mcts.playing();
        // }
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
