#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm> 
#include <queue>
using namespace std;
const std::string ALPHABET    = ".ALPHABET";
const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";
const std::string EMPTY       = ".EMPTY";

bool isChar(std::string s) {
    return s.length() == 1;
}
bool isRange(std::string s) {
    return s.length() == 3 && s[1] == '-';
}
unordered_map<string, string> keywords = {
    {"if", "IF"}, {"else", "ELSE"}, 
	{"while", "WHILE"}, {"println", "PRINTLN"},
    {"int", "INT"}, {"new", "NEW"}, 
	{"return", "RETURN"}, {"wain", "WAIN"},
    {"delete", "DELETE"}, {"NULL", "NULL"}
};

int main() {
	string file = "wlp4.dfa";
	ifstream filestream{"wlp4.dfa"};
	std::istream& rf = filestream;
	std::string s;
	vector<char> chars;
	map<string, map<char, string>> transitions;
	std::getline(rf, s); // Alphabet section (skip header)
	// Read characters or ranges separated by whitespace
	while(rf >> s) {
		if (s == STATES) { 
			break; 
		} else {
			if (isChar(s)) {
				//// Variable 's[0]' is an alphabet symbol
				chars.emplace_back(s[0]);
			} else if (isRange(s)) {
				for(char c = s[0]; c <= s[2]; ++c) {
				//// Variable 'c' is an alphabet symbol
					chars.emplace_back(c);
				}
			} 
		}
	}

	std::getline(rf, s); // States section (skip header)
	// Read states separated by whitespace
	vector<string> accept_states, initial_states;
	while(rf >> s) {
		if (s == TRANSITIONS) { 
			break; 
		} else {
			static bool initial = true;
			bool accepting = false;
			if (s.back() == '!' && !isChar(s)) {
				accepting = true;
				s.pop_back();
			}
			//// Variable 's' contains the name of a state
			if (initial) {
				//// The state is initial
				initial = false;
				initial_states.emplace_back(s);
			}
			if (accepting) {
				//// The state is accepting
				accept_states.emplace_back(s);
			}
		}
	}

	std::getline(rf, s); // Transitions section (skip header)
	// Read transitions line-by-line
	while(std::getline(rf, s)) {
		if (s == INPUT) { 
		// Note: Since we're reading line by line, once we encounter the
		// input header, we will already be on the line after the header
			break; 
		} else {
			std::string fromState, symbols, toState;
			std::istringstream line(s);
			std::vector<std::string> lineVec;
			while(line >> s) {
				lineVec.push_back(s);
			}
			fromState = lineVec.front();
			toState = lineVec.back();
			int len = lineVec.size()-1;
			for(int i = 1; i < len; ++i) {
				std::string s = lineVec[i];
				if (isChar(s)) {
					symbols += s;
				} else if (isRange(s)) {
					for(char c = s[0]; c <= s[2]; ++c) {
						symbols += c;
					}
				}
			}
			// {start: {a: start, b: start, ..., z: start, c: seen_c},
			//  {seen_c: {a: start..., s: seen_cs}}...}
			for ( const char& c : symbols ) {
				//// There is a transition from 'fromState' to 'toState' on 'c'
				transitions[fromState][c] = toState;
			}
		}
	}

	try {
		string read_in = "";
		while (cin >> s) {
			if ((s.rfind("//", 0) == 0)) {
				getline(cin, s);
				continue;
			}
			read_in += s + " ";
		}
		if (!read_in.empty()) read_in.pop_back(); // ignore the last space
		queue<char> arr_of_char;
		string cur = "", state = initial_states[0];
		for (const char& cha : read_in) arr_of_char.push(cha);
		while (arr_of_char.size() != 0) {
			char front = arr_of_char.front();
			// map<string, map<char, string>> transitions;
			if (transitions[state][front].empty()) {
				auto res = find(accept_states.begin(), accept_states.end(), state);
				if (res != accept_states.end()) {
					if (state == "ID" and (keywords.find(cur) != keywords.end())) {
						state = keywords[cur];
					} else if (state == "number") {
						try { stoi(cur); }
						catch (...) { cerr << "ERROR: Invalid number format for the input." << endl; return 1; }
					}
					cout << state << " " << cur << endl;
					if (front == ' ') arr_of_char.pop();
					cur = "", state = initial_states[0];
					
				} else {
					cerr << "ERROR: Invalid transition." << endl;
					return 1;
				}
			} else {
				cur += front;
				state = transitions[state][front];
				arr_of_char.pop();
			}
		}
		auto res = find(accept_states.begin(), accept_states.end(), state);
		if (res != accept_states.end()) {
			if ((state == "ID") and (keywords.find(cur) != keywords.end())) {
				state = keywords[cur];
			} else if (state == "NUM") {
				try { stoi(cur); }
				catch (...) { cerr << "ERROR" << endl;  }
			}
			cout << state << " " << cur << endl;
		} else {
			cerr << "ERROR" << endl;
			return 1;
		}
	
    } catch (const std::invalid_argument &error) {
		std::cerr << "ERROR: Invalid argument for the input" << error.what() << std::endl;
    }
}
