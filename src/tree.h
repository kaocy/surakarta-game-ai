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

    TreeNode* get_parent() { return parent; }
    void set_parent(TreeNode *parent) { this->parent = parent; }

    Board& get_board() { return board; }
    const Board& get_board() const { return board; }

    int get_win_score() { return win_score; }
    void add_win_score() { win_score++; }

    int get_visit_count() { return visit_count; }
    void add_visit_count() { visit_count++; }

    int get_player() { return player; }
    void set_player(int player) { this->player = player; }

    std::vector<TreeNode>& get_all_child() { return child; }
    TreeNode& get_child(int index) { return child.at(index); }

    bool is_explore() { return explore; }
    void set_explore() { explore = true; }

    TreeNode get_best_child_node() {
        return *std::max_element(child.begin(), child.end(),
                                 [](const TreeNode A, const TreeNode B) { return A.visit_count < B.visit_count; });
    }

private:
    TreeNode *parent;
    Board board;
    int win_score;
    int visit_count;
    int player; // current player
    bool explore;
    std::vector<TreeNode> child; 
};

class Tree {
public:
    Tree(const Board& b) { root = TreeNode(b); }
    TreeNode& get_root() { return root; }

private:
    TreeNode root;
};
