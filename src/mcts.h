#pragma once
#include <cmath>
#include <vector>
#include <random>
#include "tree.h"
#include "board.h"
#include "tuple.h"
#include "utilities.h"

class MCTS {
public:
    MCTS(Tuple *tuple, bool with_tuple = false) : tuple(tuple), with_tuple(with_tuple) { engine.seed(180); }

    void playing(Board &board, int player) {
        // play with MCTS
        board = find_next_move(board, player);
    }

    // return board after best action
    Board find_next_move(Board board, int player) {
        Tree tree(board);
        TreeNode root = tree.get_root();
        root.set_explore();
        root.set_player(player);

        // select best child after 1000 MCTS search
        for (int i = 0; i < 1000; i++) {
            // Phase 1 - Selection 
            TreeNode* leaf = selection(&root);
            // Phase 2 - Expansion
            if (leaf->is_explore()) leaf = expansion(leaf);
            // else                    leaf->set_explore();
            leaf->set_explore();
            // Phase 3 - Simulation
            int value = simulation(leaf);
            // Phase 4 - Backpropagation
            backpropagation(leaf, value);
        }
        return root.get_best_child_node().get_board();
    }

private:
    TreeNode* selection(TreeNode* root) {
        // std::cout << "selection\n";
        TreeNode* node = root;
        TreeNode* best_node = nullptr;
        int root_color = root->get_player();
        float layer = 1.0f;  // 1 mine -1 theirs
        while (node->get_all_child().size() != 0) {
            // std::cout << "--------find child---------\n";
            float best_value = -1e9;
            float t = float(node->get_visit_count()) + 1;
            std::vector<TreeNode> &child = node->get_all_child();
            int color = node->get_player();

            // find the child with maximum UCB value plus n-tuple weight
            for (size_t i = 0; i < child.size(); i++) {
                float w = float(child[i].get_win_score());
                float n = float(child[i].get_visit_count()) + 1;
                float h = tuple->get_board_value(child[i].get_board(), color);

                // check whether MCTS with tuple value
                if (!with_tuple)    h = 0.0f;

                float value = -w / n + 0.5f * sqrt(2 * log2(t) / n) + h / n;
                if (best_value < value) {
                    best_value = value;
                    best_node = &child[i];
                }
            }
            node = best_node;
            layer *= -1.0f;
        }
        return node;
    }

    TreeNode* expansion(TreeNode* leaf) {
        // std::cout << "expansion\n";
        const Board& board = leaf->get_board();
        int player = leaf->get_player();

        // no need to expand if game is over
        if (board.game_over())  return leaf;

        std::vector<unsigned> eats, moves;
        board.get_possible_eat(eats, player);
        board.get_possible_move(moves, player);

        // expand all the possible child node
        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
            leaf->get_all_child().push_back(TreeNode(tmp, player ^ 1, leaf));
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);
            leaf->get_all_child().push_back(TreeNode(tmp, player ^ 1, leaf));
        }

        // there are no actions can be made
        if (leaf->get_all_child().size() == 0) return leaf;

        // randomly pick one child
        std::uniform_int_distribution<int> dis(0, leaf->get_all_child().size() - 1);
        return &(leaf->get_child(dis(engine)));
    }

    int simulation(TreeNode *leaf) {
        // std::cout << "simulation\n";
        TreeNode tmp(*leaf);
        Board& board = tmp.get_board();
        int player = tmp.get_player();
        int origin_player = player;
        
        // check if game is over before simulation
        if (board.game_over()) {
            int black_bitcount = Bitcount(board.get_board(0));
            int white_bitcount = Bitcount(board.get_board(1));
            if (origin_player == 0 && black_bitcount == 0) return -1;
            if (origin_player == 1 && white_bitcount == 0) return -1;
            return 1;
        }

        std::uniform_real_distribution<> dis(0, 1);
        std::vector<unsigned> eats, moves;
        // random playout for at most 100 steps
        for (int i = 0; i < 100 && !board.game_over(); i++) {
            eats.clear(); moves.clear();
            board.get_possible_eat(eats, player);
            board.get_possible_move(moves, player);

            std::shuffle(eats.begin(), eats.end(), engine);
            std::shuffle(moves.begin(), moves.end(), engine);

            if (eats.size() > 0) {
                board.eat(eats[0] & 0b111111, (eats[0] >> 6) & 0b111111);
            }
            else if (moves.size() > 0) {
                board.move(moves[0] & 0b111111, (moves[0] >> 6) & 0b111111);
            }
            // int size1 = eats.size(), size2 = moves.size();
            // if (dis(engine) * (size1 + size2) < size1) {
            //     if (eats.size() > 0) {
            //         board.eat(eats[0] & 0b111111, (eats[0] >> 6) & 0b111111);
            //     }
            // }
            // else {
            //     if (moves.size() > 0) {
            //         board.move(moves[0] & 0b111111, (moves[0] >> 6) & 0b111111);
            //     }
            // }

            player ^= 1; // toggle player
        }

        // the one has more piece wins
        int black_bitcount = Bitcount(board.get_board(0));
        int white_bitcount = Bitcount(board.get_board(1));
        // std::cout << black_bitcount << " " << white_bitcount << std::endl;
        if (origin_player == 0 && black_bitcount > white_bitcount)  return 1;
        if (origin_player == 1 && black_bitcount < white_bitcount)  return 1;
        if (black_bitcount == white_bitcount) return 0;
        return -1;
    }

    void backpropagation(TreeNode *node, int value) {
        // std::cout << "backpropagation\n";
        while (node != NULL) {
            node->add_visit_count();
            if (value == 1) node->add_win_score();
            node = node->get_parent();
            value *= -1;
        }
    }

private:
    Tuple *tuple;
    bool with_tuple;
    std::default_random_engine engine;
};
