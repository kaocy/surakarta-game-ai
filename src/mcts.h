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
    MCTS(Tuple *tuple) : tuple(tuple) {}

    void playing() { 
        std::uniform_real_distribution<> dis(0, 1);
        std::vector<unsigned> eats, moves;
        int black_win = 0, white_win = 0;

        for (int i = 0; i < 50; i++) {
            //std::cout << i << std::endl;
            Board board;
            int player = 0, count = 0;
            while (!board.game_over() && count++ < 200) {
                if (player == 1) { // use MCTS to select action
                    board = find_next_move(board, player);
                }               
                else { // random play
                    eats.clear(); moves.clear();
                    board.get_possible_eat(eats, player);
                    board.get_possible_move(moves, player);
                    
                    float best_value = -1e9;
                    unsigned best_code = 0;
                    Board best_state;
                    int best_action_type;

                    for (unsigned code : eats) {
                        Board tmp = Board(board);
                        tmp.eat(code & 0b111111, (code >> 6) & 0b111111);
                        float value = tuple->get_board_value(tmp);
                        if (value > best_value) {
                            best_value = value;
                            best_code = code;
                            best_state = tmp;
                            best_action_type = 0;
                        }
                    }
                    for (unsigned code : moves) {
                        Board tmp = Board(board);
                        tmp.move(code & 0b111111, (code >> 6) & 0b111111);
                        float value = tuple->get_board_value(tmp);
                        if (value > best_value) {
                            best_value = value;
                            best_code = code;
                            best_state = tmp;
                            best_action_type = 1;
                        }
                    }
                    if (best_code != 0) {
                        if (!best_action_type)  board.eat(best_code & 0b111111, (best_code >> 6) & 0b111111);
                        else                    board.move(best_code & 0b111111, (best_code >> 6) & 0b111111);
                    }
                    // std::shuffle(eats.begin(), eats.end(), engine);
                    // std::shuffle(moves.begin(), moves.end(), engine);

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
                }
                player ^= 1; // toggle player
            }

            int black_bitcount = Bitcount(board.get_board(0));
            int white_bitcount = Bitcount(board.get_board(1));
            if (black_bitcount == 0) {
                white_win++;
                //std::cout << "White wins\n";
            }    
            else if (white_bitcount == 0) {
                black_win++;
                //std::cout << "Black wins\n";
            }
            else {
                //std::cout << "Too Long" << black_bitcount << " "<< white_bitcount <<std::endl;
            }
        }

        std::cout << "Playing 50 episodes: \n";
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "Black: " << black_win * 100.0 / (black_win + white_win) << " %" << std::endl;
        std::cout << "White: " << white_win * 100.0 / (black_win + white_win) << " %" << std::endl;
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
            if (leaf->is_explore())
                leaf = expansion(leaf);
            else
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
        //std::cout << "selection\n";
        TreeNode* node = root;
        TreeNode* best_node = nullptr;
        int layer = 0;
        while (node->get_child().size() != 0) {
            // std::cout << "--------find child---------\n";
            float best_value = -1e9, worst_value = 1e9;
            float t = float(node->get_visit_count()) + 1;
            std::vector<TreeNode> &child = node->get_child();
            // find the child with maximum UCB value plus n-tuple weight
            for (int i = 0; i < child.size(); i++) {
                // std::cout << &child[i] << std::endl;
                // if (!child[i].is_explore())  return &child[i];
                float w = float(child[i].get_win_score());
                float n = float(child[i].get_visit_count()) + 1;
                float h = tuple->get_board_value(child[i].get_board());
                float value = w / n + 0.3*sqrt(2 * log2(t) / n) /*+ h / n*/;
                //std::cout << w << " "<< n << std::endl;
                if (layer && best_value < value) {
                    best_value = value;
                    best_node = &child[i];
                }
                if (!layer && worst_value > value) {
                    worst_value = value;
                    best_node = &child[i];
                }
            }
            // std::cout << "--------------------------\n";
            node = best_node;
            layer ^= 1;
            // std::cout << &node << std::endl;
        }
        return node;
    }

    TreeNode* expansion(TreeNode* leaf) {
        //std::cout << "expansion\n";
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
            leaf->get_child().push_back(TreeNode(tmp, player ^ 1, leaf));
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, (code >> 6) & 0b111111);
            leaf->get_child().push_back(TreeNode(tmp, player ^ 1, leaf));
        }

        // there are no actions can be made
        if (leaf->get_child().size() == 0) {
            //std::cout << "expansion size = 0\n";
            //Board::data black = board.get_board(0);
            //Board::data white = board.get_board(1);
            //std::cout << Bitcount(black) << " " << Bitcount(white) << std::endl;
            // for (unsigned i = 0; i < 64; i++) {
            //     if (black & (1ULL << i))    std::cout << 'O';
            //     else if (white & (1ULL << i))    std::cout << 'X';
            //     else    std::cout << '*';
            //     std::cout << ' ';
            //     if (i % 8 == 7) std::cout << std::endl;
            // }
            // std::cout << "---------------\n";
            return leaf;
        }

        // randomly pick one child
        std::uniform_int_distribution<int> dis(0, leaf->get_child().size() - 1);
        return &(leaf->get_child_node(dis(engine)));
    }

    int simulation(TreeNode *leaf) {
        //std::cout << "simulation\n";
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

            player ^= 1; // toggle player
        }

        // the one has more piece wins
        int black_bitcount = Bitcount(board.get_board(0));
        int white_bitcount = Bitcount(board.get_board(1));
        if (origin_player == 0 && black_bitcount >= white_bitcount)  return 1;
        if (origin_player == 1 && black_bitcount <= white_bitcount)  return 1;
        return -1;
    }

    void backpropagation(TreeNode *node, int value) {
        //std::cout << "backpropagation\n";
        node->add_visit_count();
        if (value == 1) node->add_win_score();
        while (node->get_parent() != NULL) {
            //std::cout << node->get_parent()->get_win_score() << " " << node->get_parent()->get_visit_count() << std::endl;
            value *= -1;
            node = node->get_parent();
            node->add_visit_count();
            if (value == 1) node->add_win_score();
        }
    }

private:
    Tuple *tuple;
    std::default_random_engine engine;
};
