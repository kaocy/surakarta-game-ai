#pragma once
#include <iostream>
#include <sstream>
#include <chrono>
#include "board.h"
#include "action.h"
#include "agent.h"

class statistic;

class Episode {
friend class statistic;
public:
    Episode() : ep_state(initial_state()),
                ep_time(0),
                turn_count(0) { ep_moves.reserve(10000); }

    Board& state() { return ep_state; }
    const Board& state() const { return ep_state; }
    std::string winner() { return ep_winner; }
    const std::string& winner() const { return ep_winner; }

    void open_episode(const std::string& tag) {
        ep_open = { tag, millisec() };
    }
    void close_episode(const std::string& tag) {
        ep_close = { tag, millisec() };
        ep_winner = tag;
    }

    bool apply_action(Action move) {
        int result = move.apply(state());
        if (result == -1)   return false;
        ep_moves.emplace_back(move, millisec() - ep_time);
        return true;
    }

    TrainingPlayer& take_turns(TrainingPlayer& play1, TrainingPlayer& play2) {
        ep_time = millisec();
        return (turn_count++ % 2) ? play2 : play1;
    }

    TrainingPlayer& last_turns(TrainingPlayer& play1, TrainingPlayer& play2) {
        return take_turns(play2, play1);
    }

public:
    size_t step(unsigned who = -1u) const {
        int size = ep_moves.size(); // 'int' is important for handling 0
        switch (who) {
            case 0: return (size + 1) / 2;
            case 1: return size / 2;
            default: return size;
        }
    }

    time_t time(unsigned who = -1u) const {
        time_t time = 0;
        switch (who) {
            case 0:
                for (unsigned i = 0; i < ep_moves.size(); i += 2) time += ep_moves[i].time;
                break;
            case 1:
                for (unsigned i = 1; i < ep_moves.size(); i += 2) time += ep_moves[i].time;
                break;
            default:
                time = ep_close.when - ep_open.when;
                break;
        }
        return time;
    }

public:
    friend std::ostream& operator <<(std::ostream& out, const Episode& ep) {
        out << ep.ep_open << '|';
        for (const move& mv : ep.ep_moves) out << mv;
        out << '|' << ep.ep_close;
        return out;
    }
    friend std::istream& operator >>(std::istream& in, Episode& ep) {
        ep = {};
        std::string token;
        std::getline(in, token, '|');
        std::stringstream(token) >> ep.ep_open;
        std::getline(in, token, '|');
        for (std::stringstream moves(token); !moves.eof(); moves.peek()) {
            ep.ep_moves.emplace_back();
            moves >> ep.ep_moves.back();
        }
        std::getline(in, token, '|');
        std::stringstream(token) >> ep.ep_close;
        return in;
    }

protected:
    struct move {
        Action code;
        time_t time;
        move(Action code = {}, time_t time = 0) : code(code), time(time) {}
        operator Action() const { return code; }

        friend std::ostream& operator <<(std::ostream& out, const move& m) {
            // out << m.code;
            if (m.time) out << '(' << std::dec << m.time << ')';
            return out;
        }
        friend std::istream& operator >>(std::istream& in, move& m) {
            // in >> m.code;
            m.time = 0;
            if (in.peek() == '(') {
                in.ignore(1);
                in >> std::dec >> m.time;
                in.ignore(1);
            }
            return in;
        }
    };

    struct meta {
        std::string tag;
        time_t when;
        meta(const std::string& tag = "N/A", time_t when = 0) : tag(tag), when(when) {}

        friend std::ostream& operator <<(std::ostream& out, const meta& m) {
            return out << m.tag << "@" << std::dec << m.when;
        }
        friend std::istream& operator >>(std::istream& in, meta& m) {
            return std::getline(in, m.tag, '@') >> std::dec >> m.when;
        }
    };

    static Board initial_state() {
        return {};
    }
    static time_t millisec() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

private:
    Board ep_state;
    std::vector<move> ep_moves;  
    time_t ep_time;
    int turn_count;

    meta ep_open;
    meta ep_close;
    std::string ep_winner;
};
