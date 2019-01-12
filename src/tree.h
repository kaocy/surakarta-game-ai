#pragma once
#include <vector>
#include "board.h"

class TreeNode {
public:
    TreeNode() {}
    TreeNode(const Board &b) : 
        parent(NULL), board(b), win_score(5), visit_count(9), player(0), explore(false) { child.clear(); }
    TreeNode(const Board &b, int player, TreeNode* parent) : 
        parent(parent), board(b), win_score(5), visit_count(9), player(player), explore(false) { child.clear(); }
    TreeNode(const TreeNode& node) = default;
    TreeNode& operator =(const TreeNode& node) = default;

    TreeNode* get_parent() {
        return parent;
    }
    void set_parent(TreeNode *parent) {
        this->parent = parent;
    }

    Board& get_board() {
        return board;
    }
    const Board& get_board() const {
        return board;
    }

    int get_win_score() {
        return win_score;
    }
    void add_win_score() {
        win_score++;
    }

    int get_visit_count() {
        return visit_count;
    }
    void add_visit_count() {
        visit_count++;
    }

    int get_player() {
        return player;
    }
    void set_player(int player) {
        this->player = player;
    }

    std::vector<TreeNode>& get_child() {
        return child;
    }
    TreeNode& get_child_node(int index) {
        if (index < 0 || index >= child.size()) {
            std::cout << "Out of range\n";
            index = (index % child.size() + child.size()) % child.size();
        }
        return child.at(index);
    }

    TreeNode get_best_child_node() {
        int max_visit_count = 0;
        TreeNode best_node;
        for (auto node : child) {
            if (node.visit_count > max_visit_count) {
                max_visit_count = node.visit_count;
                
                best_node = node;
            }
        }
        // std::cout<< best_node.visit_count<<std::endl;
        return best_node;
    }

    bool is_explore() {
        return explore;
    }
    void set_explore() {
        explore = true;
    }

private:
    TreeNode *parent;
    Board board;
    int win_score;
    int visit_count;
    int player; // current player
    std::vector<TreeNode> child;
    bool explore;
};

class Tree {
public:
    Tree(const Board& b) { root = TreeNode(b); }
    TreeNode& get_root() { return root; }

private:
    TreeNode root;
};
