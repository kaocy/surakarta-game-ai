#pragma once
#include <list>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"

class Statistic {
public:
    /**
     * the total episodes to run
     * the block size of statistic
     * the limit of saving records
     *
     * note that total >= limit >= block
     */
    Statistic(size_t total, size_t block = 0, size_t limit = 0)
        : total(total),
          block(block ? block : total),
          limit(limit ? limit : total),
          count(0) {}

public:
    /**
     * show the statistic of last 'block' games
     *
     * the format would be
     * 1000   ops = 241563 (170543|896715)
     * Black: 48.7 %
     * White: 51.3 %
     *
     * where (block = 1000 by default)
     *  '1000': current index (n)
     *  'ops = 241563 (170543|896715)': the average speed is 241563
     *                                  the average speed of player1 is 170543
     *                                  the average speed of player2 is 896715
     */
    void show(bool tstat = true) const {
        size_t blk = std::min(data.size(), block);
        size_t black_win = 0, white_win = 0;
        size_t sop = 0, op1 = 0, op2 = 0;
        time_t sdu = 0, du1 = 0, du2 = 0;
        auto it = data.end();
        for (size_t i = 0; i < blk; i++) {
            auto& ep = *(--it);
            if (ep.winner() == "Black") black_win++;
            else if (ep.winner() == "White")   white_win++;
            sop += ep.step();
            op1 += ep.step(0);
            op2 += ep.step(1);
            sdu += ep.time();
            du1 += ep.time(0);
            du2 += ep.time(1);
        }

        std::ios ff(nullptr);
        ff.copyfmt(std::cout);
        std::cout << std::fixed << std::setprecision(0);
        std::cout << count << "\t";
        std::cout << "ops = " << (sop * 1000.0 / sdu);
        std::cout <<     " (" << (op1 * 1000.0 / du1);
        std::cout <<      "|" << (op2 * 1000.0 / du2) << ")";
        std::cout << std::endl;
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "Black: " << black_win * 100.0 / (black_win + white_win) << " %" << std::endl;
        std::cout << "White: " << white_win * 100.0 / (black_win + white_win) << " %" << std::endl;
        std::cout << std::endl;
        std::cout.copyfmt(ff);
    }

    void summary() const {
        auto block_temp = block;
        const_cast<Statistic&>(*this).block = data.size();
        show();
        const_cast<Statistic&>(*this).block = block_temp;
    }

    bool is_finished() const { return count >= total; }

    void open_episode(const std::string& flag = "") {
        if (count++ >= limit) data.pop_front();
        data.emplace_back();
        data.back().open_episode(flag);
    }

    void close_episode(const std::string& flag = "") {
        data.back().close_episode(flag);
        if (count % block == 0) show();
    }

    int episode_count() { return count; }

    Episode& at(size_t i) {
        auto it = data.begin();
        while (i--) it++;
        return *it;
    }
    Episode& front() { return data.front(); }
    Episode& back() { return data.back(); }

    friend std::ostream& operator <<(std::ostream& out, const Statistic& stat) {
        for (const Episode& rec : stat.data) out << rec << std::endl;
        return out;
    }
    friend std::istream& operator >>(std::istream& in, Statistic& stat) {
        for (std::string line; std::getline(in, line) && line.size(); ) {
            stat.data.emplace_back();
            std::stringstream(line) >> stat.data.back();
        }
        stat.total = std::max(stat.total, stat.data.size());
        stat.count = stat.data.size();
        return in;
    }

private:
    size_t total;
    size_t block;
    size_t limit;
    size_t count;
    std::list<Episode> data;
};
