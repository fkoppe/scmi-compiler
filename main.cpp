#include <iostream>

#include "tokens.hpp"
#include "lexer.hpp"

int main() {
    lexString("  void main(int y,int z){    int x = 0x5;long y=3;   const int[5  ] y = {1,  2, 3   , 4,  5};    y[4] = 4;    print(y); }    ");
}