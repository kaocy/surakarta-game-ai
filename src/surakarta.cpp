#include <iostream>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "utilities.h"

int main(int argc, const char* argv[]) {
    bsf_table_init();
    Player play1(0), play2(1);
    Episode game;

    while (true) {
        Player& who = game.take_turns(play1, play2);
        Action move = who.take_action(game.state());
        std::cout << who.role() << "  ";

        if (game.apply_action(move) != true) break;

        Board::data black = game.state().get_board(0);
        Board::data white = game.state().get_board(1);

        std::cout << Bitcount(black) << " " << Bitcount(white) << std::endl;
        for (unsigned i = 0; i < 64; i++) {
            if (black & (1ULL << i))    std::cout << 'O';
            else if (white & (1ULL << i))    std::cout << 'X';
            else    std::cout << '*';
            std::cout << ' ';
            if (i % 8 == 7) std::cout << std::endl;
        }
        std::cout << "---------------\n";

        if (who.check_for_win(game.state())) break;
    }
    Player& win = game.last_turns(play1, play2);
    std::cout << "Winner: " << win.role() << std::endl;

    return 0;
}
