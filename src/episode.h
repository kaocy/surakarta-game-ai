#pragma once
#include <iostream>
#include "board.h"
#include "action.h"
#include "agent.h"

class Episode {
public:
    Episode() : ep_state(initial_state()), turn_count(0) {}

    Board& state() { return ep_state; }
    const Board& state() const { return ep_state; }

    bool apply_action(Action move) {
        return (move.apply(state()) != -1);
    }

    Player& take_turns(Player& play1, Player& play2) {
        return (turn_count++ % 2) ? play2 : play1;
    }

    Player& last_turns(Player& play1, Player& play2) {
        return take_turns(play2, play1);
    }

protected:
    static Board initial_state() {
        return {};
    }

private:
    Board ep_state;
    int turn_count;
};
