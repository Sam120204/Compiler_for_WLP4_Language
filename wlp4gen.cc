#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include <ostream>
#include <iostream>
#include <string>
#include <map>
#include <deque>
#include <iomanip>
#include <string>
using namespace std;

int newoffset = 0;

int get_newOffset() {
    ++newoffset;
    return newoffset;
}

vector<string> terminals = {
    "PLUS", "MINUS",
    "SLASH","PCT", "AMP", "STAR", "NEW", "LBRACK", "RBRACK", "FACTOR",
    "BOF", "INT", "WAIN", "LPAREN", "RPAREN", "COMMA", "LBRACE", "RBRACE",
    "RETURN", "SEMI", "STAR", "ID", "NUM", "NULL", "EOF", "BECOMES", "DELETE", "EQ",
    "LT", "LE", "GE", "GT", "ELSE", "IF", "NE", "PRINTLN", "WHILE"
};


map<string, string> type_table{
    {"NULL", "int*"},
    {"INT", "int"},
    {"NUM", "int"},
    {"INT STAR", "int*"}
};


void setup_constant(int reg, int num) {
    cout << "lis $" << reg << endl << ".word " << num << endl;
}

void push_reg(int num, int offset) {
    cout << "sw $" + to_string(num) + ", " << offset 
    << "($29)\nsub $30, $30, $14\n";
}

void pop_reg() {
    cout << "add $30,$30,$14\n";
}

// push for operations
void push(int reg) {
    cout << "sw $" << to_string(reg) << ", -4($30)\nsub $30, $30, $14\n";
}

// pop for operations
void pop(int reg) {
    cout << "add $30, $30, $14\nlw $" << to_string(reg) << ", -4($30)\n";
}

void setup_frame_pointer() {
    cout << "sub $29 , $30, $14\n";
}
    
void print_prologue() {
    cout << "; begin prologue\n";
    setup_constant(14, 4); // setup constant
    setup_constant(21, 1); // setup constant
    setup_frame_pointer();
    // for (int i = 0; i < 2; i++) { // push vars in the stack
    //     push_reg(i+1, -4*i);
    // }
    cout << "; end prologue\n\n";
    cout << "sw $31, -4($30)\nsub $30, $30, $14\nlis $31\n.word init\njalr $31\nadd $30, $30, $14\nlw $31, -4($30)\n";
}


class Var {
public:
    string symbol;
    string type;
    int offset;

    Var(const string name, const string type, const int offset = 0): symbol(name), type(type), offset(offset) {}
    //Var() : offset(0) {} 
    ~Var() = default;
};


class Symbol_table {
public:
    // map<string, Variable> symbolTableMap = {
    //     {"a", Variable("a", "int", 0)},
    //     {"b", Variable("b", "int", -4)},
    //     {"c", Variable("c", "int", -8)}
    // };
    int num_param = 0;
    int num_local_var = 0;
    map<string, Var> symbol_table_map;
    
    void printSymbolTable() const {
        cout << setw(10) << left << "Symbol" 
             << setw(10) << "Type" 
             << setw(15) << "Offset (from $29)" << endl;
        cout << string(35, '-') << endl;

        for (const auto& entry : symbol_table_map) {
            cout << setw(10) << left << entry.second.symbol
                 << setw(10) << entry.second.type
                 << setw(15) << entry.second.offset << endl;
            cout << "symbol is " << entry.first << endl;
        }
    }

    void add_pair_to_table(Var ele, bool if_para) {
        if (!if_para) {
            ele.offset = num_local_var * (-4);
            num_local_var += 1;
        } else {
            ele.offset = (1+num_param) * 4;
            num_param += 1;
            //cout << "ele.offset is " << ele.offset << endl;
        }

        symbol_table_map.insert(make_pair(ele.symbol, ele));

    }
};


class Tree_N {
public:
    string curline; // Grammar rule name
    deque<shared_ptr<Tree_N>> children; // Child nodes for non-terminals
    string rule_start;
    string type;
    string be_assigned; // term factor : int -> be_assigned = factor
    string not_first_word_line;
    string kind;
    string terminals;
    //Constructors
    Tree_N(const string& inputLine) {
        stringstream line(inputLine);
        if (inputLine.find(":") != std::string::npos) {
            curline = inputLine.substr(0, inputLine.find(":"));
            curline.pop_back();
        } else {
            curline = inputLine;
        }

        
        
        // Initialize local variables
        string token, be_assigned_accum;

        
        // Process the first word separately
        getline(line, rule_start, ' ');
        string t = "";
        line.str(inputLine);
        line.clear(); 
        line >> token; // Skip the first token (already processed as rule_start)
        bool if_type = false;
        // Start processing the rest of the tokens
        while (line >> token) {
            // if (rule_start == "type") {
            //     t += token + " ";
            // }
            if (token == ":") { 
                if_type = true;
                break; // Stop if ':' is reached
            }
           
            be_assigned_accum += token + " ";
            
        }
        
        if (!be_assigned_accum.empty()) {
            be_assigned_accum.pop_back();
        }
        
        be_assigned = be_assigned_accum; 
        
        if (if_type) {
            line >> type;
        }
        
    }

    void printChildren() const {
        cout << "Children of Node: " << curline << "\n";
        for (const auto& child : children) {
            cout << "Child Node: " << child->curline << "\n";
        }
    }
    // Method to add child nodes
    void addChild(const shared_ptr<Tree_N>& child) {
        children.push_back(child);
    }

    void printTree() {
        if (!curline.empty()) {
            cout << curline;
            cout << endl;
            for (const auto & i : children) {
                i->printTree();
            }
        }
        return;
    }

    int get_num_line() {
        int count = 0;
        istringstream c(curline);
        string temp;
        while (c >> temp) {
            if (temp != ".EMPTY") {
                count++;
            }
            else return 0; // No children if .EMPTY}
        }
        return count; // First word is the kind, not a child
    }

    void displayTree(int depth = 0) const {
        string indent(depth * 2, ' '); // Indentation based on the depth of the tree
        cout << indent << "Rule Start: " << rule_start;
        if (!be_assigned.empty()) {
            cout << ", Be Assigned: " << be_assigned;
        }
        cout << "\n" << indent << "Current Line: " << curline << "\n";

        for (const auto& child : children) {
            child->displayTree(depth + 1); // Recursively display children
        }
    }
};


shared_ptr<Tree_N> build_parse_tree(istream& stream_in) {
    string temp;
    getline(stream_in, temp);
    auto tree_part = make_shared<Tree_N>(temp); // Create a node for the current line
    string kind;
    istringstream cur_l(temp);
    cur_l >> kind;
    // If kind is not in terminals, the node is not a terminal, so it will have children
    if (find(terminals.begin(), terminals.end(), kind) == terminals.end()) { // not terminals
        int num = tree_part->get_num_line()-1;
        for (int j = 0; j < num; ++j) {
            tree_part->addChild(build_parse_tree(stream_in)); // Recursively add children
        }
    }
    return tree_part;
}

// declarations of all the helper fucntions

void print_proced_body_part(shared_ptr<Tree_N> node, Symbol_table &sym_table);
// printintg the result for each procedure/wain
void binary_opertions(const shared_ptr<Tree_N>& node, Symbol_table &sym_table);
shared_ptr<Tree_N> get_real_lvalue(const shared_ptr<Tree_N>& node);
void AMP_case(const shared_ptr<Tree_N>& node, Symbol_table &sym_table);
void comparisons(const shared_ptr<Tree_N>& node, Symbol_table &sym_table);
void to_statement(const shared_ptr<Tree_N>& node, Symbol_table &sym_table);

void print_return(const shared_ptr<Tree_N>& n, Symbol_table& m) {
    // cout << "n->be_assigned " << n->be_assigned << endl;
    if (n->rule_start == "ID") {
        string name = "";
        name = n->be_assigned;
       // cout << "name is " << name << endl;
        
        auto it = m.symbol_table_map.find(name);
        if (it != m.symbol_table_map.end()) {
          // cout << "name is " << n->curline << " and ID is " << n->be_assigned << endl;
         //  m.printSymbolTable();
           cout << "lw $3, " << it->second.offset << "($29)" << endl;
        }
    } else if (n->rule_start == "NUM") {
        cout <<  "lis $3\n.word " << n->be_assigned << endl;
    
    } else if (n->be_assigned == "ID LPAREN RPAREN" or
               n->be_assigned == "ID LPAREN arglist RPAREN") {

        print_proced_body_part(n,m); // procedure

    } else if (n->rule_start == "NULL") { // nullptr 
        cout << "add $3, $21, $0\n";
    } else if (n->children.size() == 3 and 
        (n->children[1]->rule_start == "PLUS" or n->children[1]->rule_start == "MINUS" or 
        n->children[1]->rule_start == "STAR" or n->children[1]->rule_start == "SLASH" or 
        n->children[1]->rule_start == "PCT")) {
            binary_opertions(n, m);
    } else if (n->children.size() == 3 and 
        (n->children[1]->rule_start == "EQ" or n->children[1]->rule_start == "NE" or 
        n->children[1]->rule_start == "LT" or n->children[1]->rule_start == "LE" or 
        n->children[1]->rule_start == "GE" or n->children[1]->rule_start == "GT")) {
            comparisons(n, m); 
        
    } else if (n->be_assigned == "STAR factor") {
        //cout << "here: STAR factor\n";
        // factor STAR factor....
        print_return(n->children[1], m);
        cout << "\n ; dereference ptr\nlw $3, 0($3)\n";
    } else if (n->be_assigned == "NEW INT LBRACK expr RBRACK") {
        // factor → NEW INT LBRACK expr RBRACK
        print_return(n->children[3], m);
        cout << "add $1, $3, $0\n";
        push(31);
        cout << "lis $5\n.word new\njalr $5\n";
        pop(31);
        cout << "bne $3, $0, 1\nadd $3, $0, $21" << std::endl;

    } else if (n->be_assigned == "AMP lvalue") {
        // factor AMP lvalue
        //cout << "here: AMP\n";
        // cout << "isAddressPtr RULE: " << n->curline << endl;
        AMP_case(n, m);
    } else {
        for (auto i : n->children) {
            print_return(i, m);
        }
    }
}

// void codePointerArithmetic(const string& op, const string& leftType, const string& rightType, 
// shared_ptr<Tree_N> leftNode, shared_ptr<Tree_N> rightNode, Symbol_table &sym_table) {
//     cout << "here\n";
    

//     //cout << "OP is " << op << " " << leftType << " " << rightType << endl;
//     if ((op == "PLUS" && leftType == ptrType && rightType == intType) || 
//         (op == "PLUS" && leftType == intType && rightType == ptrType)) {
//         if (leftType == intType) {
//             swap(leftNode, rightNode);
//         }
//         cout << "; ptr + int or int + ptr operation" << endl;
//         print_return(leftNode, sym_table); // Evaluate the pointer part
//         push(3);
//         print_return(rightNode, sym_table); // Evaluate the integer part
//         multiplyByFour();
//         pop(5);
//         cout << "add $3, $5, $3" << endl;
//         return;
//     } else if (op == "MINUS" && leftType == ptrType && rightType == intType) {
//         cout << "; ptr - int operation" << endl;
//         print_return(leftNode, sym_table);
//         push(3);
//         print_return(rightNode, sym_table);
//         multiplyByFour();
//         pop(5);
//         cout << "sub $3, $5, $3" << endl;
//         return;
//     } else if (op == "MINUS" && leftType == ptrType && rightType == ptrType) {
//         cout << "; ptr - ptr operation" << endl;
//         print_return(leftNode, sym_table);
//         push(3);
//         print_return(rightNode, sym_table);
//         pop(5);
//         cout << "sub $3, $5, $3" << endl;
//         divideByFour(); // Correcting the division to find the difference in elements
//         return;
//     }
// }

// expr → expr PLUS term
// expr → expr MINUS term
// term → term STAR factor
// term → term SLASH factor
// term → term PCT factor
void binary_opertions(const shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
    cout << "\n; below is the binary operations part\n";

    const std::string ptrType = "int*";
    const std::string intType = "int";
    cout << "; codePointerArithmetic\n";
    auto multiplyByFour = []() {
        cout << "mult $3, $14\nmflo $3" << endl;
    };
    
    
    auto divideByFour = []() {
        cout << "div $3, $14\nmflo $3" << endl;
    };

    auto add_reg_three = []() {
        cout << "add $3, $5, $3\n";
    };

    auto sub_reg_three = []() {
        cout << "sub $3, $5, $3\n";
    };

    auto operation = node->children[1];
    auto type = operation->rule_start;
    auto l = node->children[0], r = node->children[2];
    //cout << left->type << endl << right->type<< endl;

    if (type == "PLUS" and l->type == "int*"
    and r->type == "int") { // case: pointer + int
        // int* + int
        cout << "; below is int* + int\n";
        print_return(l, sym_table);
        push(3);
        print_return(r, sym_table);
        multiplyByFour();
        pop(5);
        add_reg_three();
        return;
    } else if (type == "MINUS" and l->type == "int*" and r->type == "int") { // case: pointer - int
        // int* - int
        cout << "; below is int*-int\n";
        print_return(l, sym_table);
        push(3);
        print_return(r, sym_table);
        multiplyByFour();
        pop(5);
        sub_reg_three();
        return;
    } else if (type == "PLUS" and l->type == "int" and r->type == "int*") { // case: int + pointer
        // int + int*
        cout << "; below is int+int*\n";
        print_return(r, sym_table);
        push(3);
        print_return(l, sym_table);
        multiplyByFour();
        pop(5);
        add_reg_three();
        return;
    } else if (type == "MINUS" and l->type == "int*" and r->type == "int*") {
        // int - int
        cout << "; below is int-int\n";
        print_return(l, sym_table);
        push(3);
        print_return(r, sym_table);
        pop(5);
        sub_reg_three();
        cout << "; divid by four here\n";
        divideByFour();
        return;
    }

    // handle expr and term
    print_return(l, sym_table);
    push(3);
    print_return(r, sym_table);
    pop(5);
    if (type == "PLUS") {
        add_reg_three();
    } else if (type == "MINUS") {
        sub_reg_three();
    } else if (type == "STAR") {
        cout << "mult $5, $3\nmflo $3\n";
    } else if (type == "SLASH") {
        cout << "div $5, $3\nmflo $3\n";
    } else if (type == "PCT") {
        cout << "div $5, $3\nmfhi $3\n";
    }
}

void comparisons(const shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
    cout << "\n; below is the comparisions part\n";
    auto operation = node->children[1];
    auto type = operation->rule_start;
    auto left = node->children[0], right = node->children[2];
    string op_sign;
    if (left->type == "int*" and right->type == "int*") {
        op_sign = "sltu";
    } else { op_sign = "slt"; }

    print_return(left, sym_table);
    push(3);
    print_return(right, sym_table);
    pop(5);
    if (type == "EQ") {
        cout << op_sign << " $6, $3, $5\n" << op_sign << " $7, $5, $3\nadd $3, $6, $7\nsub $3, $21, $3\n";
    } else if (type == "NE") {
        cout << op_sign << " $6, $3, $5\n" << op_sign << " $7, $5, $3\nadd $3, $6, $7\n";
    } else if (type == "GT") {
        cout << op_sign << " $3, $3, $5\n";
        
    } else if (type == "GE") {
        cout << op_sign << " $3, $5, $3\nsub $3, $21, $3\n";
    
    } else if (type == "LT") {
        cout << op_sign << " $3, $5, $3\n";
    } else if (type == "LE") {
        cout << op_sign << " $3, $3, $5\nsub $3, $21, $3\n";
    } 
}

 // get statement lvalue BECOMES expr SEMI's lvalue id
shared_ptr<Tree_N> get_real_lvalue(shared_ptr<Tree_N>& node) {
    //cout << "line is " << node->curline << endl;
    if (node->rule_start != "ID") {
        if (node->curline == "lvalue STAR factor" or 
        node->curline == "lvalue LPAREN lvalue RPAREN") {
            return get_real_lvalue(node->children[1]);
        } else if (node->curline == "factor ID" or node->curline == "lvalue ID") {
            
            return get_real_lvalue(node->children[0]);
        }
    }
    // cout << "return node is " << node->curline << endl; 
    return node;
}

void AMP_case(const shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
    // factor AMP lvalue
    auto lval = get_real_lvalue(node->children[1]);
    
    string name = lval->be_assigned; // ID b : int
   // cout << "ID is " << name << endl;
    auto it = sym_table.symbol_table_map.find(name);
    string offset;
    if (it != sym_table.symbol_table_map.end()) {
        offset = to_string(it->second.offset);
        cout << "lis $3\n.word " << offset << "\nadd $3, $3, $29\n";
    } else {
        //cout << "NEW!!lval line is " << lval->children.back()->curline << endl;
        print_return(node->children.back()->children.back(), sym_table);
    }
    
    
}


// adding variable to the symbol table
void dcl_update_symbol_table(Symbol_table& symbol_table, shared_ptr<Tree_N>& node, bool signal, int num) { // dcl type ID
    // dcl type ID
    auto ID = node->children[1];
    string varName = ID->be_assigned;
    string t = node->children[0]->type;
  //  cout << "t is " <<varName << " type is " <<t << endl;

    if (signal == false) { push(num); }
    symbol_table.add_pair_to_table(Var(varName, t), signal);
    
}


void print_pop_epil(Symbol_table &sym_table) {
    cout << "; begin epilogue\n";
    for (int i = 0; i < sym_table.num_param + sym_table.num_local_var; i++) {
        pop_reg();
    }
    cout << "jr $31\n";
}


void get_procedure_argument(shared_ptr<Tree_N> node, Symbol_table &sym_table, int & args_size) {
    //cout << "Be assigned is " << node->be_assigned << endl;
    if (!node->children.empty()) {
        auto argument = node->children[0];
        if (argument->rule_start == "expr") { // arglist → expr
            
            print_return(argument, sym_table);
            push(3);
            args_size++;
        }
        
        if (node->be_assigned == "expr COMMA arglist") { // arglist → expr COMMA arglist
            get_procedure_argument(node->children[2], sym_table, args_size);
        }
    }
}


// push($29)
// push($31)
// lis $3
// .word 1
// push($3) ; arg1
// lis $3 
// .word 2
// push($3) ; arg2 
// lis $5
// .word foo
// jalr $5
// pop ($31) 
// pop ($31) 
// pop($31) 
// pop($29)

void print_proced_body_part(shared_ptr<Tree_N> node, Symbol_table &sym_table) {
    cout << "; below is the procedure body part\n";
    string name = "";
    name = node->children[0]->be_assigned;
    
    push(29);
    push(31);
    int parameters_count = 0;
    // factor → ID LPAREN arglist RPAREN
    if (node->be_assigned == "ID LPAREN arglist RPAREN") {
        //cout << "here\n";
        get_procedure_argument(node->children[2], sym_table, parameters_count);
    }

    cout << "lis $5\n.word " << "Procedure" << name << endl << "jalr $5\n";
    pop(31);
    pop(29);
}


// paramlist → dcl
// paramlist → dcl COMMA paramlist
void paramlist(shared_ptr<Tree_N> node, Symbol_table &sym_table) {
    if (node->children.size() == 3) {
        // paramlist → dcl COMMA paramlist
        paramlist(node->children[2], sym_table);
    }
    if (node->children[0]->rule_start == "dcl") {
        // paramlist → dcl
       // cout << "here: " << node->curline << endl;
        dcl_update_symbol_table(sym_table, node->children[0], true, 3);
    }
}

 void handle_Decl_with_Initialization(shared_ptr<Tree_N>& dclNode, Symbol_table &sym_table) {
    // ignore: dcls .EMPTY case
    if (dclNode->curline == "dcls dcls dcl BECOMES NULL SEMI" or
    dclNode->curline == "dcls dcls dcl BECOMES NUM SEMI") {
        handle_Decl_with_Initialization(dclNode->children.front(), sym_table); // recurse for the rest 
        cout << "\n; dcls part\n";
        print_return(dclNode->children[3], sym_table);
        dcl_update_symbol_table(sym_table, dclNode->children[1], false, 3);
        
    }
}



void stmt_check_for_control_structures(shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
    if (node->curline == "statements statements statement") {
        stmt_check_for_control_structures(node->children[0], sym_table);
        auto statement_back = node->children.back();
        to_statement(statement_back, sym_table);
    } else return;
}

void control_if_while_statements(const shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
    int newoffset = get_newOffset();
    if (node->children[0]->rule_start == "IF") {
        cout << "\n; this is if part\n";
        auto test = node->children[2];
        print_return(test, sym_table);
        cout << "beq $3, $0, else" << newoffset << endl;
        auto stmts1 = node->children[5], stmts2 = node->children[9];
        stmt_check_for_control_structures(stmts1, sym_table); // code(stmts1)
        cout << "beq $0, $0, endif" << newoffset << endl;
        cout << "else" << newoffset << ":" << endl;
        stmt_check_for_control_structures(stmts2, sym_table); // code(stmts2)
        cout << "endif" << newoffset << ":" << endl;

    } else if (node->children[0]->rule_start == "WHILE") {
        cout << "\n; this is while loop part\n";
        auto test = node->children[2];
        cout << "loop" << newoffset << ":" << endl; // loop:
        print_return(test, sym_table); // code(test)
        cout << "beq $3, $0, endwhile" << newoffset << endl; // beq $3, $0, endWhile
        auto stmts1 = node->children[5];
        stmt_check_for_control_structures(stmts1, sym_table); // code(stmts1)
        cout << "beq $0, $0, loop" << newoffset << endl;
        cout << "endwhile" << newoffset << ":" << endl;
    }
}

void handle_print(const shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
    // case where node->curline = PRINTLN LPAREN expr RPAREN SEMI
    auto n = node->children[2];
    push(1);
    print_return(n, sym_table);
    cout << "add $1, $3, $0\n";
    push(31);
    cout << "lis $5\n.word print\njalr $5\n";
    pop(31);
    pop(1);
}

void to_statement(const shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
   // cout << "here " << node->not_first_word_line << endl;;
    if (node->be_assigned == "lvalue BECOMES expr SEMI") {
        cout << "; to-lvalue\n";
        //cout << "here " << node->children[0]->curline << endl;;
        if (node->children[0]->curline == "lvalue ID") {
            // lvalue -> ID
            auto lvalue = node->children[0];
            shared_ptr<Tree_N> real_lvalue = get_real_lvalue(lvalue);
            string real_lvalue_name = real_lvalue->be_assigned;
            auto it = sym_table.symbol_table_map.find(real_lvalue_name);
            string offset;
            if (it != sym_table.symbol_table_map.end()) {
                offset = to_string(it->second.offset);
            }

            print_return(node->children[2], sym_table);
            cout << "sw $3, " << offset << "($29)\n";
        } else if (node->children[0]->curline == "lvalue STAR factor") {
            // lvalue → STAR factor
            cout << "\n ;deference\n";
            print_return(node->children[2], sym_table);
            push(3);
            auto ID_node = node->children.front()->children.back();
            print_return(ID_node, sym_table);
            pop(5);
            cout << "sw $5, 0($3)" << endl;
        }
    } else if (node->be_assigned == "IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"
    or node->be_assigned == "WHILE LPAREN test RPAREN LBRACE statements RBRACE") {// adding the control structures here as well

        control_if_while_statements(node, sym_table);
    } else if (node->be_assigned == "PRINTLN LPAREN expr RPAREN SEMI") {
        handle_print(node, sym_table);
    } else if (node->be_assigned == "DELETE LBRACK RBRACK expr SEMI") {
        print_return(node->children[3], sym_table);
        string name = to_string(get_newOffset());
        cout << "beq $3, $21, jumpd" << name << "\nadd $1, $3, $0\n";
        push(31);
        cout << "lis $5\n.word delete\njalr $5\n";
        pop(31);   
        cout << "jumpd" << name << ":\n";
    
    
    }
}

void handle_statement(const shared_ptr<Tree_N>& node, Symbol_table &sym_table) {
    if (node->curline == "statements statements statement") {
        cout << "; statements part\n";
        handle_statement(node->children[0], sym_table);
        to_statement(node->children.back(), sym_table);
    }
}

void get_main_code(shared_ptr<Tree_N> node, Symbol_table &sym_table) {
    int count = 1;
    for (auto & i : node->children) {
        // cout << i->curline << endl;
        if (i->rule_start == "dcls") {
            handle_Decl_with_Initialization(i, sym_table);

        } else if (i->rule_start == "dcl") {
            //cout << "DCL LINE IS " << i->curline << endl;
            dcl_update_symbol_table(sym_table, i, false, count);
            count += 1;
        } 
         else if (i->rule_start == "statements") {
            handle_statement(i, sym_table);
        } else if (i->rule_start == "expr") {
            //cout << "; here\n";
            print_return(i, sym_table);
        }
    }
    print_pop_epil(sym_table);
}



void print_proced_structure(shared_ptr<Tree_N> node, Symbol_table &sym_table) {
    cout << "; below is the procedure structure part\n";
    string name = "";
    // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    name = node->children[1]->be_assigned;
    
    cout << "Procedure" << name << ":\n";
    cout << "sub $29, $30, $14\n";
    for (auto & i : node->children) {
        if (i->rule_start == "params" and i->be_assigned != ".EMPTY") {
            paramlist(i->children[0], sym_table);
        } else if (i->rule_start == "dcls") {
            // dcls → dcls dcl BECOMES NULL SEMI

            handle_Decl_with_Initialization(i, sym_table);

        } else if (i->rule_start == "statements") {
            // statements statements statement
            handle_statement(i, sym_table);
        }  else if (i->rule_start == "expr") {
            print_return(i, sym_table);
        
        
        }
    }
    print_pop_epil(sym_table);
}



void search_tree(shared_ptr<Tree_N> node, Symbol_table& sym_table) {
   // cout << node->curline << endl;
   if (node->rule_start == "procedures") { // procedures procedure procedures
        search_tree(node->children.back(), sym_table);
        if (node->children.size() == 2) { // wain
            search_tree(node->children[0], sym_table); // wain
        }
    } else if (node->rule_start == "main") {
        get_main_code(node, sym_table);
        
    } else if (node->rule_start == "procedure") {
        // initializing each procedure's table
        auto procedure_table = make_shared<Symbol_table>();
        print_proced_structure(node, *procedure_table);
    } 
}


int main() {
    cout << ".import print\n.import init\n.import new\n.import delete\n";
    auto parse_tree = build_parse_tree(cin);
    //parse_tree->printTree();
    cout << endl;
    Symbol_table sym_table;
    int currentOffset = 0;
    string return_content = "";

    // ; prologue
    print_prologue();
    search_tree(parse_tree->children[1], sym_table);

    return 0;
}