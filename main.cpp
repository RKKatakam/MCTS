#include <iostream>
#include <vector>
#include <random>
#include <limits>

const int HUMAN_PLAYER = 1;
const int AI_PLAYER = -1;

// Board representation and helper functions
using Board = std::vector<std::vector<int> >;

std::vector<std::pair<int, int> > get_legal_moves(const Board& board) {
    std::vector<std::pair<int, int> > legal_moves;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == 0) {
                legal_moves.push_back({i, j});
            }
        }
    }
    return legal_moves;
}

int get_winner(const Board& board) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != 0) {
            return board[i][0];
        }
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != 0) {
            return board[0][i];
        }
    }

    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != 0) {
        return board[0][0];
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != 0) {
        return board[0][2];
    }

    return 0;
}

// Node class
class Node {
public:
    Board state;
    int player;
    Node* parent;
    std::vector<Node*> children;
    int visits = 0;
    int wins = 0;

    Node() : state(3, std::vector<int>(3, 0)), player(HUMAN_PLAYER), parent(nullptr) {}

    bool is_terminal() {
        return get_winner(state) != 0 || get_legal_moves(state).empty();
    }

    bool is_fully_expanded() {
        return children.size() == get_legal_moves(state).size();
    }
};

// MCTS class
class MCTS {
public:
    Node* root;
    std::mt19937 rng; // Random number generator

    MCTS(Node* root) : root(root), rng(std::random_device{}()) {}

    Node* expand(Node* node) {
        auto legal_moves = get_legal_moves(node->state);
        std::uniform_int_distribution<> dist(0, legal_moves.size() - 1);
        for (const auto& move : legal_moves) {
            Node* child = new Node();
            child->state = node->state; 
            child->state[move.first][move.second] = node->player;
            child->player = -node->player;
            child->parent = node;
            node->children.push_back(child);
        }
        return node->children[dist(rng)];
    }

    Node* select() {
        Node* node = root;
        while (!node->is_terminal()) {
            if (!node->is_fully_expanded()) {
                return expand(node);
            } else {
                node = best_child(node);
            }
        }
        return node;
    }

    int simulate(Node* node) {
        Board state = node->state;
        int player = node->player;
        std::uniform_int_distribution<> dist(0, 2);
        while (get_winner(state) == 0) {
            auto legal_moves = get_legal_moves(state);
            if (legal_moves.empty()) {
                return 0;
            }
            const auto& move = legal_moves[dist(rng) % legal_moves.size()];
            state[move.first][move.second] = player;
            player = -player;
        }
        return get_winner(state);
    }

    void backpropagate(Node* node, int result) {
        while (node != nullptr) {
            node->visits++;
            if (node->player == result) {
                node->wins++;
            }
            node = node->parent;
        }
    }

    Node* search(int n) {
        for (int i = 0; i < n; i++) {
            Node* node = select();
            int result = simulate(node);
            backpropagate(node, result);
        }
        return best_child(root);
    }

private:
    Node* best_child(Node* node) {
        double best_score = -std::numeric_limits<double>::infinity();
        Node* best_child = nullptr;
        for (Node* child : node->children) {
            if (child->visits == 0) {
                return child;
            }
            double score = static_cast<double>(child->wins) / child->visits +
                           2 * std::sqrt(static_cast<double>(node->visits) / child->visits);
            if (score > best_score) {
                best_score = score;
                best_child = child;
            }
        }
        return best_child;
    }
};

// Main function
int main() {
    Board state(3, std::vector<int>(3, 0)); 
    int player = AI_PLAYER;

    Node* root = new Node();
    root->state = state;
    root->player = player;

    MCTS mcts(root);
    Node* best_move = mcts.search(10000);

    // Print the board after the AI's move
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            std::cout << best_move->state[i][j] << " ";
        }
        std::cout << std::endl;
    }

    // Cleanup: Delete the dynamically allocated nodes
    // (Implementation omitted for brevity, but important to avoid memory leaks)

    return 0;
}