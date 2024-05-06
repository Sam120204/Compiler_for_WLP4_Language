#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <stack>
#include <deque>
#include <map>
#include <algorithm>
#include "wlp4data.h"
using namespace std;

struct Token {
    string type;
    string lex;
};

struct Node {
    vector<string> production;
    vector<Node> children;
    Token tk;
};

void printparseTree(Node ele) {
    if (ele.production.empty()) {
        Token t = ele.tk;
        std::cout << t.type << " " << t.lex;
    } else {
        for (std::string s : ele.production) {
            std::cout << s << " ";
        }
    }
    std::cout << std::endl;
    for (Node i : ele.children) {
        printparseTree(i);
    }
}

int main() {
    std::istringstream istr(WLP4_COMBINED);
    std::istream& in = istr;
    std::string line;

    // expr expr + term -> [expr] [expr, +, term]];
    vector<vector<string>> productions;
    std::getline(in, line); // .CFG section (skip header)
    while(getline(in, line) and !line.empty()) {
        if (line == ".TRANSITIONS") { 
            break; 
        } else {
            istringstream iss{line};
            vector<string> tokens = {};
            string token;
            while (iss >> token) {
                tokens.emplace_back(token);
            }
            productions.emplace_back(tokens);
        }
    }

    // TRANSITIONS part
    map<pair<int, string>, int> transitions; // For shift actions: state and symbol to new state

    // transitions = {
    // {{0, "BOF"}, 6},
    // {{1, "id"}, 2} }
    
    while(getline(in, line) and !line.empty()) {
        //cout << line << endl;
        if (line == ".REDUCTIONS") { 
            break; 
        } else {
            int start_state, to_state;
            string symbol;
            istringstream iss{line};
            if (!(iss >> start_state >> symbol >> to_state)) {
                cerr << "Error reading transitions." << endl;
            }
            transitions[{start_state, symbol}] = to_state;
        }
    }
    
    // map<pair<int, string>, pair<int, string>> reductions = {
    // {{2, "-"}, 3},
    // {{3, ")"}, 2} }

    map<pair<int, string>, int> reductions;  // For reductions: state to (rule index, LHS symbol)
    while(getline(in, line) and !line.empty()) {
        //cout << line << endl;
        if (line == ".END") { 
            break; 
        } else {
            int state, rule_num;
            string tag;
            istringstream iss{line};
            
            if (!(iss >> state >> rule_num >> tag)) {
                cerr << "Error reading reductions." << endl;
            }
            reductions[{state, tag}] = rule_num;
        }
    }

    
    // for (const auto & [key, val] : transitions) {
    //     cout << key.first << " " << key.second << " :" << val << endl;
    // }
    //  cout << endl;
    // for (const auto & [key, val] : reductions) {
    //     cout << key.first << " " << key.second << " :" << val << endl;
    // }

    std::istream &newstream = std::cin;
    std::deque<Token> unread;
    string input;
    unread.push_back(Token{"BOF", "BOF"});
    while (std::getline(newstream, input)) {
        std::istringstream line(input);
        std::string type, lex;
        line >> type >> lex;
        unread.push_back(Token{type, lex});
    }
    unread.push_back({"EOF", "EOF"});


    // SLR(1) algorithm
    // Stacks for states and symbols
    stack<int> state_stack;
    stack<string> sym_stack;
    stack<Node> tree_stack; // Stack for nodes to build the tree
    int cur_state;
    int num_shift = 0;
    // Initialize with the starting state
    state_stack.push(0);
    unread.push_back({".ACCEPT", ".ACCEPT"});

    for (const auto & tk : unread) {
        string tk_type = tk.type;
        while (true) {
            cur_state = state_stack.top();
            auto check_reduct_It = reductions.find({cur_state, tk_type});
            
            // find the transition
            if (check_reduct_It == reductions.end()) { break; }
         
            int rule_num = check_reduct_It->second; 
            vector<string> production = productions[rule_num];

            vector<Node> children = {};
            while (production.size() > 1) {
                if (production.back() != ".EMPTY") {
                    children.emplace_back(tree_stack.top()); // New children node
                    tree_stack.pop(); // Remove the old nodes from the treeStack
                    sym_stack.pop();
                    state_stack.pop();
                }
                production.pop_back();
            }
            sym_stack.push(production[0]);

            // Update tree_stack since it is productions
            vector<string> cur_production = productions[rule_num];
            vector<Node> reversed(children.rbegin(), children.rend());
            children = move(reversed);
            tree_stack.push({cur_production, children, {tk_type, tk_type}});


            auto check_next_state_It = transitions.find({state_stack.top(), sym_stack.top()});
            if (check_next_state_It != transitions.end()) { // find the transition
                // shift part
                state_stack.push(check_next_state_It->second);
            }
            

            if (tk_type == ".ACCEPT") { 
                printparseTree(tree_stack.top());
                return 0;
            }

        }
        sym_stack.push(tk_type);
        // Create node and push to treeStack
        tree_stack.push({{},{}, {tk.type, tk.lex}});

        auto check_next_state_It = transitions.find({state_stack.top(), sym_stack.top()});
        if (check_next_state_It == transitions.end()) { // find the transition
            cerr << "ERROR at " << num_shift << endl;
            return 0;
        } else {
            state_stack.push(check_next_state_It->second);
        }
        num_shift++;                                   

    }
    
    return 0;
}
