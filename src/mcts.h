#pragma once
#include <cmath>
#include <vector>
#include <random>
#include <unordered_map>
#include "tree.h"
#include "board.h"
#include "tuple.h"
#include "utilities.h"

class MCTS {
public:
    MCTS(Tuple *tuple, bool with_tuple = false, int simulation_count = 5000, uint32_t seed = 10) :
        tuple(tuple),
        with_tuple(with_tuple),
        simulation_count(simulation_count) { engine.seed(seed); }

    void playing(Board &board, int player, int sim) {
        // play with MCTS
        TreeNode node = find_next_move(board, player, sim);
        if (node.get_board() != board) {
            board = node.get_board();
        }
    }

    std::pair<std::string, unsigned> training(Board &board, int player, int sim, int game_length) {
        // play with MCTS
        TreeNode node = find_next_move(board, player, sim, game_length);

        // return best node's action
        if (node.get_board() != board) {
            board = node.get_board();
            return node.get_prev_action();
        }

        // cannot find child node
        return std::make_pair("none", 0);
    }

    // return board after best action
    TreeNode find_next_move(Board board, int player, int sim, int game_length = 10) {
        Tree tree(board);
        TreeNode root = tree.get_root();
        root.set_explore();
        root.set_player(player);
        root_expansion(&root);

        // select best child after 5000 MCTS search
        for (int i = 0; i < simulation_count; i++) {
            // Phase 1 - Selection 
            TreeNode* leaf = selection(&root);
            // Phase 2 - Expansion
            if (leaf->is_explore()) leaf = expansion(leaf);
            leaf->set_explore();
            // Phase 3 - Simulation
            // int value = simulation(leaf, sim);
            // Backup from the leaf so get the value of leaf player
            float value = leaf->get_state_value();
            // Phase 4 - Backpropagation
            backpropagation(leaf, value);
        }
        // cannot find move
        if (root.get_all_child().size() == 0) return root;
        // return best move
        std::uniform_real_distribution<> dis(0, 1);
        if (game_length >= 5) return root.get_best_child_node();
        else                  return root.get_child_with_temperature(dis(engine));
    }

private:
    TreeNode* selection(TreeNode* root) {
        // std::cout << "selection\n";
        TreeNode* node = root;
        TreeNode* best_node = nullptr;

        while (node->get_all_child().size() != 0) {
            float best_value = -1e9;
            const float t = log2(float(node->get_visit_count()));
            const float child_softmax_sum = node->get_child_softmax_total();
            std::vector<TreeNode> &child = node->get_all_child();
            // find the child with maximum UCB value + Progressive Bias
            for(size_t i = 0; i < child.size(); i++) {
                float q = child[i].get_win_rate();
                float n = float(child[i].get_visit_count());
                // float h = child[i].get_state_value();
                float poly = child[i].get_softmax_value() / child_softmax_sum;
                // check whether MCTS with tuple value
                if (!with_tuple) {
                    // h = 0.0f;
                    poly = 1.0f;
                }
                float ucb = sqrt(2 * t / n);
                //float pb = 3.0f * h / log2(n);
                float value = q + poly * 20 * ucb ;

                if (best_value < value) {
                    best_value = value;
                    best_node = &child[i];
                }
            }
            node = best_node;
        }
        return node;
    }

    TreeNode* expansion(TreeNode* leaf) {
        // std::cout << "expansion\n";
        const Board& board = leaf->get_board();
        int player = leaf->get_player();
        float child_softmax_total = 0;
        // no need to expand if game is over
        if (board.game_over())  return leaf;

        std::vector<unsigned> eats, moves;
        board.get_possible_eat(eats, player);
        board.get_possible_move(moves, player);

        // expand all the possible child node, calculate tuple value, record previous action
        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
            float state_value = tuple->get_board_value(tmp, player);
            float softmax_value = exp(state_value * 3);
            child_softmax_total += softmax_value;
            leaf->get_all_child().push_back(TreeNode(
                tmp,
                state_value,
                softmax_value,
                player ^ 1,
                leaf,
                std::make_pair("eat", code)
            ));
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);
            float state_value = tuple->get_board_value(tmp, player);
            float softmax_value = exp(state_value * 3);
            child_softmax_total += softmax_value;
            leaf->get_all_child().push_back(TreeNode(
                tmp,
                state_value,
                softmax_value,
                player ^ 1,
                leaf,
                std::make_pair("move", code)
            ));
        }
        leaf->set_child_softmax_total(child_softmax_total);
        // there are no actions can be made
        if (leaf->get_all_child().size() == 0) return leaf;

        // randomly pick one child
        std::uniform_int_distribution<int> dis(0, leaf->get_all_child().size() - 1);
        return &(leaf->get_child(dis(engine)));
    }

    void root_expansion(TreeNode* leaf) {
        // std::cout << "expansion\n";
        const Board& board = leaf->get_board();
        int player = leaf->get_player();
        float child_softmax_total = 0;
        // no need to expand if game is over
        if (board.game_over())  return;

        std::vector<unsigned> eats, moves;
        board.get_possible_eat(eats, player);
        board.get_possible_move(moves, player);
        size_t child_size = eats.size() + moves.size();
        std::vector<float> dirichlet;
        std::gamma_distribution<float> gamma(0.3, 1.0f);
        float dir_sum = 0;
        for (size_t i = 0; i < child_size; i++) {
            float tmp = gamma(engine);
            dirichlet.emplace_back(tmp);
            dir_sum += tmp;
        }
        if (dir_sum >= std::numeric_limits<float>::min()) {
            for (float &v : dirichlet) v = v / dir_sum;
        }
        size_t child_counter = 0;
        // expand all the possible child node, calculate tuple value, record previous action
        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
            float state_value = tuple->get_board_value(tmp, player);
            float d_state_value = 0.75 * state_value + 0.25 * dirichlet[child_counter++];
            float softmax_value = exp(d_state_value * 3);
            child_softmax_total += softmax_value;
            leaf->get_all_child().push_back(TreeNode(
                tmp,
                state_value,
                softmax_value,
                player ^ 1,
                leaf,
                std::make_pair("eat", code)
            ));
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);
            float state_value = tuple->get_board_value(tmp, player);
            float d_state_value = 0.75 * state_value + 0.25 * dirichlet[child_counter++];
            float softmax_value = exp(d_state_value * 3);
            child_softmax_total += softmax_value;
            leaf->get_all_child().push_back(TreeNode(
                tmp,
                state_value,
                softmax_value,
                player ^ 1,
                leaf,
                std::make_pair("move", code)
            ));
        }
        leaf->set_child_softmax_total(child_softmax_total);
    }

    int simulation(TreeNode *leaf, int sim) {
        // std::cout << "simulation\n";
        TreeNode tmp(*leaf);
        Board& board = tmp.get_board();
        int player = tmp.get_player();
        int origin_player = player;
        int black_bitcount = Bitcount(board.get_board(0));
        int white_bitcount = Bitcount(board.get_board(1));
        // check if game is over before simulation
        if (board.game_over()) {
            if (origin_player == 0 && black_bitcount == 0) return -1;
            if (origin_player == 1 && white_bitcount == 0) return -1;
            return 1;
        }

        std::uniform_real_distribution<> dis(0, 1);
        std::vector<unsigned> eats, moves;

        // playout for at most 100 steps
        for (int i = 0; i < 100 && !board.game_over(); i++) {
            eats.clear(); moves.clear();
            board.get_possible_eat(eats, player);
            board.get_possible_move(moves, player);

            std::shuffle(eats.begin(), eats.end(), engine);
            std::shuffle(moves.begin(), moves.end(), engine);

            // random
            if (sim == 0) {
                int size1 = eats.size(), size2 = moves.size();
                if (dis(engine) * (size1 + size2) < size1) {
                    if (eats.size() > 0) {
                        board.eat(eats[0] & 0b111111, (eats[0] >> 6) & 0b111111);
                    }
                }
                else {
                    if (moves.size() > 0) {
                        board.move(moves[0] & 0b111111, (moves[0] >> 6) & 0b111111);
                    }
                }
            }
            // eat first
            else if (sim == 1) {
                if (eats.size() > 0) {
                    board.eat(eats[0] & 0b111111, (eats[0] >> 6) & 0b111111);
                }
                else if (moves.size() > 0) {
                    board.move(moves[0] & 0b111111, (moves[0] >> 6) & 0b111111);
                }
            }
            // tuple
            else if (sim == 2) {
                float best_value = -1e9;
                unsigned best_code = 0;
                int best_action_type;
                for (unsigned code : eats) {
                    Board tmp = Board(board);
                    tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
                    float value = tuple->get_board_value(tmp, player);
                    if (value > best_value) {
                        best_value = value;
                        best_code = code;
                        best_action_type = 0;
                    }
                }
                for (unsigned code : moves) {
                    Board tmp = Board(board);
                    tmp.move(code & 0b111111, (code >> 6) & 0b111111);
                    float value = tuple->get_board_value(tmp, player);
                    if (value > best_value) {
                        best_value = value;
                        best_code = code;
                        best_action_type = 1;
                    }
                }
                if (best_code != 0) {
                    if (!best_action_type)  board.eat(best_code & 0b111111, (best_code >> 6) & 0b111111);
                    else                    board.move(best_code & 0b111111, (best_code >> 6) & 0b111111);
                }
            }
            black_bitcount = Bitcount(board.get_board(0));
            white_bitcount = Bitcount(board.get_board(1));
            if((black_bitcount - white_bitcount > 3) || (black_bitcount - white_bitcount < -3)) break;
            player ^= 1; // toggle player
        }

        // the one has more piece wins
        // std::cout << black_bitcount << " " << white_bitcount << std::endl;
        if (origin_player == 0 && black_bitcount > white_bitcount)  return 1;
        if (origin_player == 1 && black_bitcount < white_bitcount)  return 1;
        if (black_bitcount == white_bitcount) return 0;
        return -1;
    }

    // void backpropagation(TreeNode *node, int value) {
    //     // std::cout << "backpropagation\n";
    //     while (node != NULL) {
    //         node->add_visit_count();
    //         if (value == 1) node->add_win_count();
    //         node = node->get_parent();
    //         value *= -1;
    //     }
    // }

    void backpropagation(TreeNode *node, float value) {
        // std::cout << "backpropagation\n";
        while (node != NULL) {
            node->update_win_rate(value);
            node->add_visit_count();
            node = node->get_parent();
            value *= -1;
        }
    }

private:
    Tuple *tuple;
    const bool with_tuple;
    const int simulation_count;
    std::default_random_engine engine;
};
