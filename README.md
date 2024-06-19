## Compiler for WLP4 Language

### Introduction
This repository contains a compiler for the WLP4 language, which is a simplified version of C. WLP4 stands for “Waterloo, Language, Plus, Pointers, Plus, Procedures.” The compiler is implemented in C++ and consists of several components to handle different stages of the compilation process, including scanning, parsing, type checking, and code generation.

### Overview of Files
The main components of the compiler are divided into the following files:

1. **wlp4scan.cc** - Scanner
2. **wlp4parse.cc** - Parser
3. **wlp4type.cc** - Type Checker
4. **wlp4gen.cc** - Code Generator

### WLP4 Resources
- [WLP4 Programming Language Tutorial](https://student.cs.uwaterloo.ca/~cs241/wlp4/WLP4tutorial.html)
- [WLP4 Programming Language Specification](https://student.cs.uwaterloo.ca/~cs241/wlp4/WLP4.html)

### Compilation and Execution
To compile and execute the WLP4 compiler, follow these steps:

1. **Compile the source files:**
    ```sh
    g++ -o wlp4scan wlp4scan.cc
    g++ -o wlp4parse wlp4parse.cc
    g++ -o wlp4type wlp4type.cc
    g++ -o wlp4gen wlp4gen.cc
    ```

2. **Run the scanner:**
    ```sh
    ./wlp4scan <input_file> > tokens.txt
    ```

3. **Run the parser:**
    ```sh
    ./wlp4parse < tokens.txt > parse_tree.txt
    ```

4. **Run the type checker:**
    ```sh
    ./wlp4type < parse_tree.txt > typed_tree.txt
    ```

5. **Generate the code:**
    ```sh
    ./wlp4gen < typed_tree.txt > output.asm
    ```

### Detailed Description of Components

#### Scanner (wlp4scan.cc)
The scanner reads the input WLP4 program and converts it into a sequence of tokens. It handles the recognition of keywords, identifiers, numbers, and other symbols.

#### Parser (wlp4parse.cc)
The parser takes the sequence of tokens produced by the scanner and constructs a parse tree according to the grammar rules of WLP4. It uses a stack-based approach to handle shifts and reductions based on the SLR(1) parsing method.

#### Type Checker (wlp4type.cc)
The type checker traverses the parse tree and ensures that all expressions and statements have correct types. It manages symbol tables for variables and procedures, performs type inference, and checks for type consistency in operations and assignments.

#### Code Generator (wlp4gen.cc)
The code generator produces assembly code from the typed parse tree. It handles the translation of WLP4 constructs into corresponding assembly instructions, manages stack frames, and ensures proper calling conventions for procedures.

### Example
Here is a simple example of a WLP4 program and the corresponding steps to compile and run it:

**Input WLP4 Program (example.wlp4):**
```c
//
// WLP4 Program to compute:
//   a^b if 0 <= a,b < 10
//   -1 otherwise
//

int wain(int a, int b) {
   int counter = 0;
   int product = 0;
   product = 0-1;  // only binary minus
   if (a >= 0) {
      if (b >= 0) {
         if (a < 10) {
            if (b < 10) {
               product = 1;
               counter = 0;
               while (counter < b) {
                  product = product * a;
                  counter = counter + 1;
               }
            } else {} // must have else
         } else {} 
      } else {} 
   } else {}
   return product;
}
```

**Commands:**
1. Scan the input program:
    ```sh
    ./wlp4scan < example.wlp4 > tokens.txt
    ```

2. Parse the tokens:
    ```sh
    ./wlp4parse < tokens.txt > parse_tree.txt
    ```

3. Type check the parse tree:
    ```sh
    ./wlp4type < parse_tree.txt > typed_tree.txt
    ```

4. Generate the assembly code:
    ```sh
    ./wlp4gen < typed_tree.txt > output.asm
    ```

**Generated Assembly Code (output.asm):**
```assembly
; begin prologue
lis $14
.word 4
sub $29 , $30, $14
sw $31, -4($30)
sub $30, $30, $14
lis $31
.word init
jalr $31
add $30, $30, $14
lw $31, -4($30)
; end prologue

add $3, $1, $2
sw $3, -4($30)
sub $30, $30, $14
lw $3, -4($30)
add $30, $30, $14
jr $31
```

### Contribution
Contributions to improve the compiler are welcome. Feel free to fork the repository, make your changes, and submit a pull request.

### Contact
For any questions or issues, please contact Jiayou Zhong at j55zhong@uwaterloo.ca
