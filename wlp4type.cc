#include <memory>
#include <ostream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <deque>
using namespace std;


const std::string EMPTY = ".EMPTY";
const std::string ID = "ID";
const std::string INT = "INT";
const std::string INT_STAR = "INT STAR";
const std::string NUM = "NUM";
const std::string NIL = "NULL";

const std::string PLUS = "PLUS";
const std::string MINUS = "MINUS";
const std::string STAR = "STAR";
const std::string SLASH = "SLASH";
const std::string PCT = "PCT";
const std::string AMP = "AMP";
const std::string EQ = "EQ";
const std::string NE = "NE";
const std::string LT = "LT";
const std::string LE = "LE";
const std::string GT = "GT";
const std::string GE = "GE";

const std::string MAIN = "main";
const std::string PROCEDURES = "procedures";
const std::string PROCEDURE = "procedure";
const std::string DCL = "dcl";
const std::string DCLS = "dcls";
const std::string EXPR = "expr";
const std::string TERM = "term";
const std::string FACTOR = "factor";
const std::string LVALUE = "lvalue";

vector<string> terminals = {
    "BOF", "INT", "WAIN", "LPAREN", "RPAREN", "COMMA", "LBRACE", "RBRACE",
    "RETURN", "SEMI", "STAR", "ID", "NUM", "NULL", "EOF", "BECOMES", "PLUS", "MINUS",
    "SLASH","PCT", "AMP", "STAR", "NEW", "LBRACK", "RBRACK", "FACTOR", "IF", "ELSE",
    "WHILE", "PRINTLN", "DELETE", "EQ", "NE", "LT", "LE", "GE", "GT"
};



map<string, string> type_table{
    {"NULL", "int*"},
    {"INT", "int"},
    {"NUM", "int"},
    {"INT STAR", "int*"}
};

class Symbol {
public:
    string name;
    Symbol(const string& name): name(name) {}
    virtual ~Symbol() = default;
};


class Variable : public Symbol {
public:
    string type;
    Variable(const string& name, const string& type)
        : Symbol(name), type(type) {}
};


// Declaration of the Procedure class
class Procedure : public Symbol {
public:
    vector<string> names;
    deque<string> types_paramters; // Signature: parameter types
    map<string, map<string, Variable>> symbolTable; // Symbol table for variables within the procedure

    // Constructor
    Procedure(const string& name, const deque<string>& types_paramters)
        : Symbol(name),  types_paramters(types_paramters) {}

    // //Add a variable to the procedure's symbol table
    // void addVariable(const string& varName, const string& varType) {
    //     symbolTable[varName] = (varName, varType);
    // }

    // //Check if a variable is declared in the procedure's symbol table
    // bool isVariableDeclared(const string& varName) const {
    //     return symbolTable.find(varName) != symbolTable.end();
    // }

    // //Get the type of a variable declared in the procedure's symbol table
    // string getVariableType(const string& varName) const {
    //     auto it = symbolTable.find(varName);
    //     if (it != symbolTable.end()) {
    //         return it->second->type; // Access the type of the Variable object
    //     }
    //     return ""; // Return empty string if variable is not found
    // }

    //Print procedure information
    // void printProcedureInfo() const {
    //     cout << "Procedure: " << name << "\nParameters: ";
    //     for (const auto& type : types_paramters) {
    //         cout << type << " ";
    //     }
    //     cout << "\nSymbol Table:\n";
    //     for (const auto& entry : symbolTable) {
    //         cout << "  " << entry.first << " : " << entry.second.type << "\n";
    //     }
    // }

    void display() const {
        std::cout << "Procedure: " << name << "\nParameter Types: ";
        for (const auto& term : types_paramters) {
            std::cout << term << " ";
        }
        std::cout << "\n";
    }
};

class Tree_N {
public:
    string kind;
    string curline; // Grammar rule name
    string term; // tp
    string terminals; // Actual value (for terminals)
    deque<shared_ptr<Tree_N>> children; // Child nodes for non-terminals

    // Constructors
    Tree_N(const string& inputLine) : curline{inputLine} {
        stringstream line(inputLine);
        line >> kind; // First word in the line is kind
        line >> terminals;
        string signal;
        while (line >> signal) {
            terminals += ' ' + signal;
        }
    }

    // Method to add child nodes
    void addChild(const shared_ptr<Tree_N>& child) {
        children.push_back(child);
    }

    void print_parse_t() {
        cout << curline;
        if (!term.empty()) cout << " : " << term;
        cout << endl;
        for (const auto & child : children) {
           child->print_parse_t();
        }
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

};


shared_ptr<Tree_N> build_parse_tree(istream& in) {
    string s;
    getline(in, s);
    auto n = make_shared<Tree_N>(s); // Create a node for the current line
    string kind;
    istringstream line(s);
    line >> kind;
    // If kind is not in terminals, the node is not a terminal, so it will have children
    if (find(terminals.begin(), terminals.end(), n->kind) == terminals.end()) { // not terminals
        int num = n->get_num_line()-1;
        for (int j = 0; j < num; ++j) {
            n->addChild(build_parse_tree(in)); // Recursively add children
        }
    }
    return n;
}


void assignTypeFromChildren(shared_ptr<Tree_N> cur_node) {
    for (const auto& child : cur_node->children) {
        if (!child->term.empty()) {
            cur_node->term = child->term;
            break;
        }
    }
}

void handlePrintLn(std::shared_ptr<Tree_N> n);
void handleDeallocate(std::shared_ptr<Tree_N> n);
void handleAssignment(std::shared_ptr<Tree_N> n);
bool isComparisonOp(std::shared_ptr<Tree_N> n);
void handleComparison(std::shared_ptr<Tree_N> n);

void update_type_for_nodes(shared_ptr<Tree_N> cur, map<string, Variable>* vt = nullptr) {
    istringstream is(cur->curline);
    string instr;
    is >> instr;

    if (instr == "NUM") {
        cur->term = "int";
    } else if (instr == "NULL") {
        cur->term = "int*";
    } else if (instr == "dcl") {
        string term = cur->children.front()->terminals;
        if (type_table.count(term) == 1) {
            cur->children.back()->term = type_table[term];
        } else {
            cerr << "ERROR: Undefined term '" << term << "'." << endl;
            return;
        }
    } else if (vt && instr == "ID") {
        auto it = vt->find(cur->terminals);
        if (it != vt->end()) {
            cur->term = it->second.type;
        } else {
            cerr << "ERROR: Variable " << cur->terminals << " is not found." << endl;
            return;
        }
    } else {
        for (auto& child : cur->children) {
            update_type_for_nodes(child, vt);
            // This is for nodes where the type depends on their children (lvalue, factor, expr, term).
            if (!child->term.empty() &&
                (instr == "expr" || instr == "factor" || instr == "term" || instr == "lvalue")) {
                cur->term = child->term;
            }
        }

        if (vt) {
            cout << cur->curline << endl;
            if (instr == "expr" || instr == "term") {
                // Binary arithmetic operations have exactly three children: left operand, operator, right operand
                if (cur->children.size() == 3) {
                    auto operation = cur->children[1]->kind;
                    auto leftType = cur->children[0]->term;
                    auto rightType = cur->children[2]->term;
                    // auto operation = cur->children[1]->kind;
                    //cout << "Instr is " << instr << " operation is " << operation << "leftT is " <<
                    //leftType << " and rghtT is " << rightType << endl;
                    // Arithmetic operation rules
                    if ((operation == "PLUS" || operation == "MINUS" || operation == "STAR" || operation == "SLASH" || operation == "PCT") && 
                    leftType == "int" && rightType == "int") {
                        cur->term = "int";
                    } else if ((operation == "PLUS" || operation == "MINUS") &&
                      (leftType == "int*" && rightType == "int" || leftType == "int" && rightType == "int*")) {
                        cur->term = "int*";
                    } else if (operation == "MINUS" &&  leftType == "int*" && rightType == "int*") {
                        cur->term = "int";
                    } else {
                        cerr << "ERROR: Type mismatch in operation " << operation << leftType << " " << rightType <<" . instructino is " << instr << endl;
                        return;
                    }
                }
            } else if (instr == "factor" || instr == "lvalue" ) {
                // Handling unary operations '&' and '*' with specific invalid combinations
                if (cur->children.size() == 2) {
                    auto operation = cur->children[0]->kind;
                    auto operandType = cur->children[1]->term;
                    auto operandNode = cur->children[1];
                    if (operation == "AMP" && operandType == "int") {
                        cur->term = "int*";
                    } else if ((operation == "STAR") && operandType == "int*") {
                        cur->term = "int";
                    } else if ((operation == "STAR") && operandType == "AMP") {
                        cur->term = "int";                    
                    } else {
                        cerr << "ERROR: Invalid operation '" << operation << "' on type '" << operandType << "'." << endl;
                        return;
                    }

                    // Prevent invalid combinations like *& or &* by checking the child's operation type
                    if ((operation == "AMP" && cur->children[1]->kind == "STAR") ||
                        (operation == "STAR" && cur->children[1]->kind == "AMP")) {
                        cerr << "ERROR: Invalid combination of '*' and '&' operators." << endl;
                        return;
                    }
                } else if (cur->curline.find("NEW INT LBRACK expr RBRACK") != string::npos) {
                    if (cur->children[3]->term == "int") {
                        cur->term = "int*";
                    } else {
                        cerr << "ERROR: Array size for 'new' operation must be of type 'int'." << endl;
                        return;
                    }
                }
            } else 
            if (instr == "statement") {
                // string stmtType = cur->children[0]->kind;
                // if (stmtType == "lvalue") {
                //     auto lvalueType = cur->children[0]->term; // Type of lvalue
                //     auto exprType = cur->children[cur->children.size() - 2]->term; // Type of expression
                //     if (lvalueType != exprType) {
                //         cerr << "ERROR: Type mismatch in assignment." << endl;
                //         return;
                //     }
                // } 
                if (cur->curline.find("lvalue BECOMES expr SEMI") != string::npos ) {
                    string lvalueType, exprType;
                    for (const auto& i:cur->children) {
                        if (i->kind == "expr") {
                            exprType = i->term;
                        } else if (i->kind == "lvalue") {
                            lvalueType = i->term;
                        }
                    }
                    if (lvalueType != exprType) {
                        cerr << "ERROR: Type mismatch in assignment." << endl;
                        return;
                    }
                } else if (cur->curline.find("PRINTLN LPAREN expr RPAREN SEMI") != string::npos) {
                    // PRINTLN statement
                    for (const auto& i:cur->children) {
                        if (i->kind == "expr" and i->term != "int") {
                            cerr << "ERROR: PRINTLN argument must be of type int, found " << i->term << "." << endl;
                            return;
                        }
                    }
                } else if (cur->curline.find("DELETE LBRACK RBRACK expr SEMI") != string::npos) {
                    // DELETE statement
                    for (const auto& i:cur->children) {
                        if (i->kind == "expr" and i->term != "int*") {
                            cerr << "ERROR: DELETE argument must be of type int*, found " << i->term << "." << endl;
                            return;
                        }
                    }

                }
            }

            if (cur->children.size() == 3) {
                string operation = cur->children[1]->kind;
               // if (cur->children.size() == 3 and 
                if (operation == "EQ" || operation == "NE" || operation == "LT" || operation == "LE" || operation == "GE" || operation == "GT") {
                    // IF or WHILE statement, focusing on the test part
                    string leftExprType, rightExprType;

                    for (const auto& i : cur->children) {
                        if (leftExprType.empty() and i->term == "expr") {
                            leftExprType = i->term;
                        } else if (i->term == "expr") {
                            rightExprType = i->term;
                        }
                    }
                    if (leftExprType != rightExprType) {
                        cerr << "ERROR: Both sides of a comparison must be same type" << endl;
                        return;
                    }
                }
            }
            // if (cur->curline == "statement PRINTLN LPAREN expr RPAREN SEMI") {
            //     handlePrintLn(cur); // done
            // } else if (cur->curline == "statement DELETE LBRACK RBRACK expr SEMI") {
            //     handleDeallocate(cur); // done
            // } else if (cur->curline == "statement lvalue BECOMES expr SEMI") {
            //     handleAssignment(cur); // done
            // } else if (isComparisonOp(cur)) {
            //     handleComparison(cur);
            // }
        }
    }
}


void handleComparison(std::shared_ptr<Tree_N> n) {
    std::string leftType;
    std::string rightType;
    for (auto child : n->children) {
        if (leftType.length() == 0 && child->kind == EXPR) {
            leftType = child->term;
        } else if (child->kind == EXPR) {
            rightType = child->term;
        }
    }
    if (leftType != rightType) {
        throw std::invalid_argument("comparison type invalid");
    }
}

bool isComparisonOp(std::shared_ptr<Tree_N> n) {
    std::string op;
    if (n->children.size() == 3) {
        op = n->children.at(1)->kind;
    }
    return op == EQ || op == NE || op == LT || op == LE || op == GE || op == GT;
}

void handlePrintLn(std::shared_ptr<Tree_N> n) {
    for (auto child : n->children) {
        if (child->kind == EXPR && child->term != "int") {
            throw std::invalid_argument("invalid arg to println");
        }
    }
}

void handleDeallocate(std::shared_ptr<Tree_N> n) {
    for (auto child : n->children) {
        if (child->kind == EXPR && child->term != "int*") {
            throw std::invalid_argument("invalid arg to delete");
        }
    }
}


void handleAssignment(std::shared_ptr<Tree_N> n) {
    std::string leftType;
    std::string rightType;
    for (auto child : n->children) {
        if (child->kind == EXPR) {
            rightType = child->term;
        } else if (child->kind == LVALUE) {
            leftType = child->term;
        }
    }
    if (leftType != rightType) {
        std::cout << n->kind << std::endl;
        throw std::invalid_argument("assignment type invalid");
    }
}


string check_type(string s) {
    if (s == "num") return "NUM";
    if (s == "null") return "NULL";
    if (s == "dcl") return "DCL";
    if (s == "dcls") return "DCLS";
    return "";
}

void get_r_type(shared_ptr<Tree_N> expression, map<string, Variable> &var_table) {
    update_type_for_nodes(expression, &var_table);
    //  cout << "Variables:\n";
    // for (const auto& var_entry : var_table) {
    //     // Assuming Variable class has a meaningful way to display itself
    //     // Adjust to match your Variable class's members if this is not the case
    //     cout << "  Name: " << var_entry.first << ", Type: " << var_entry.second.type << "\n";
    // }
    // cout << "---------------------------\n";
    // cout << expression->type << endl;
    if (expression->term != "int") {
        cerr << "ERROR: The return type is not an integer it is" << expression->term << endl;
        return;
    }
}

void check_dcls(shared_ptr<Tree_N> cur_node, map<string, Variable>& var_table) {
    // Base case
    if (cur_node->terminals == ".EMPTY") return;
    
    auto& sec_child = cur_node->children[1];
    string t = type_table[sec_child->children.front()->terminals], id = sec_child->children.back()->terminals;
    if (var_table.find(id) != var_table.end()) {
        cerr << "ERROR: duplicate variable in the body function: " + id << endl;
        return;
    }

    if (t != type_table[cur_node->children[3]->kind]) {
        cerr << "ERROR: In procedure [wain]: Variable name num initialized with wrong type of value\n";
        return;
    }

    var_table.insert({id, Variable(id, t)});

    // Recursive call to process the next dcls node
    check_dcls(cur_node->children.front(), var_table);
}

void assignTypeFromChildren(shared_ptr<Tree_N> cur_node);

void update_procedure_table(shared_ptr<Tree_N> cur_node, 
                            map<string, map<string, Variable>> &var_table, 
                            map<string, Procedure> &proceds_tb) {
    istringstream ele(cur_node->curline);
    string cur;
    ele >> cur;
    if (cur == "procedures") {
        map<string, Variable> vtable;
        string procedure_name;
        ele >> procedure_name;

        deque<string> types_paramters;
        proceds_tb.insert({procedure_name, {procedure_name, types_paramters}});

        // we need to get first child of the node, then iterate the grandchildren to get all the declarations
        auto curIt = cur_node->children.front()->children.begin();
        istringstream iterateChildren(cur_node->children.front()->curline);
        iterateChildren >> cur;
        while (iterateChildren >> cur) {
            if (cur == "dcl") {
                auto node_class = *curIt;
                string type = node_class->children.front()->terminals;
                string term = node_class->children.back()->terminals;
                Variable var(term, type_table[type]);
                if (vtable.find(term) != vtable.end()) {
                    cerr << "ERROR: Duplicate parameter name in procedure '" << procedure_name << "'.\n";
                    return;
                } else {
                    vtable.insert({term, var});
                }
                
                auto procedureIt = proceds_tb.find(procedure_name);
                if (procedureIt != proceds_tb.end()) {
                    procedureIt->second.types_paramters.push_back(type);
                }
            } else if (cur == "expr") {
                get_r_type(*curIt, vtable);
            } else if (cur == "dcls") {
                check_dcls(*curIt, vtable);
                //var_table.insert({procedure_name, vtable});
            }
            curIt++;
        }
        // After populating vtable and paramTypes for this procedure, update the symbolTable map
        var_table[procedure_name] = vtable;
    } else {
        for (const auto& child : cur_node->children) {
            update_procedure_table(child, var_table, proceds_tb);
        }
    }
}


// Procedure Name: main
// Parameter Types: INT INT 
// ---------------------------
void display_procedure_table(const map<string, Procedure>& proceds_tb) {
    for (const auto& entry : proceds_tb) {
        cout << "Procedure Name: " << entry.first << "\n";
        // Assuming Procedure class has a display method or adjust to use its members directly if not
        entry.second.display(); 
        cout << "---------------------------\n";
    }
}

// Procedure: main
// Variables:
//   Name: a, Type: int
//   Name: b, Type: int
// ---------------------------
void display_variable_table(const map<string, map<string, Variable>>& var_table) {
    for (const auto& proc_entry : var_table) {
        cout << "Procedure: " << proc_entry.first << "\n";
        cout << "Variables:\n";
        for (const auto& var_entry : proc_entry.second) {
            // Assuming Variable class has a meaningful way to display itself
            // Adjust to match your Variable class's members if this is not the case
            cout << "  Name: " << var_entry.first << ", Type: " << var_entry.second.type << "\n";
        }
        cout << "---------------------------\n";
    }
}

// Node Kind: start, terminals: BOF procedures EOF
//   Node Kind: BOF, terminals: BOF
//   Node Kind: procedures, terminals: main
//     Node Kind: main, terminals: INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
//       Node Kind: INT, terminals: int
//       Node Kind: WAIN, terminals: wain
//       Node Kind: LPAREN, terminals: (
//       Node Kind: dcl, terminals: type ID
//         Node Kind: type, terminals: INT
//           Node Kind: INT, terminals: int
//         Node Kind: ID, terminals: b, Type: int
//       Node Kind: COMMA, terminals: ,
//       Node Kind: dcl, terminals: type ID
//         Node Kind: type, terminals: INT
//           Node Kind: INT, terminals: int
//         Node Kind: ID, terminals: a, Type: int
//       Node Kind: RPAREN, terminals: )
//       Node Kind: LBRACE, terminals: {
//       Node Kind: dcls, terminals: .EMPTY
//       Node Kind: statements, terminals: .EMPTY
//       Node Kind: RETURN, terminals: return
//       Node Kind: expr, terminals: term, Type: int
//         Node Kind: term, terminals: factor, Type: int
//           Node Kind: factor, terminals: NUM, Type: int
//             Node Kind: NUM, terminals: 241, Type: int
//       Node Kind: SEMI, terminals: ;
//       Node Kind: RBRACE, terminals: }
//   Node Kind: EOF, terminals: EOF
void displayChildren(const shared_ptr<Tree_N>& node, int depth = 0) {
    // Create an indentation string based on the depth of the node in the tree
    string indent(depth * 2, ' ');

    cout << indent << "Node Kind: " << node->kind;
    if (!node->terminals.empty()) {
        cout << ", terminals: " << node->terminals;
    }
    if (!node->term.empty()) {
        cout << ", Type: " << node->term;
    }
    cout << "\n";

    // Recursively display each child
    for (const auto& child : node->children) {
        displayChildren(child, depth + 1); // Increase depth for indentation
    }
}

int main() {
    // var_table: A map where each key is a string representing a variable name within the procedure, 
    // and the value is a Variable object which contains the name and type of the variable.

    map<string, map<std::string, Variable>> var_table;
    map<string, Procedure> proceds_tb;
    auto parse_tree = build_parse_tree(cin);
    //parse_tree->printTree(); // Print the tree starting from the root of the tree 
    update_type_for_nodes(parse_tree);
    update_procedure_table(parse_tree, var_table, proceds_tb);
    //update_type_for_nodes(parse_tree, &(var_table.find("main"))->second);
    // display_procedure_table(proceds_tb);
    // display_variable_table(var_table);
    //displayChildren(parse_tree);

    //display_variable_table(var_table);
    parse_tree->print_parse_t(); // Print the tree starting from the root of the tree 
    auto mainProcIt = proceds_tb.find("main");
    if (mainProcIt != proceds_tb.end()) {
        auto mainProc = mainProcIt->second;
        if (mainProc.types_paramters.size() != 2) {
            cerr << "ERROR: Does not have 2 parameters" << endl;
            return 1;
        }
        if (mainProc.types_paramters[1] != "INT") {
            cerr << "ERROR: The second parameter of wain is not int type." << endl;
            return 1;
        }
    }
    return 0; 
}

