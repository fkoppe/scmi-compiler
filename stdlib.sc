void prints(char[] string) {
    int len = @length(string);

    for(int i = 0; i < len; i++) {
        printc(string[i]);
    }
}

void printsln(char[] string) {
    prints(string);
    @output(10);
}

void printc(char character) {
    @output(character);
}

void printcln(char character) {
    printc(character);
    @output(10);
}

void print(int integer) {
    if(0 == integer) {
        @output(integer + 48);
    }

    while(integer != 0) {
        @output((integer % 10) + 48);
        integer /= 10;
    }
}

void println(int integer) {
    print(integer);
    @output(10);
}

int fib(int x) {
    int a = 0;
    int b = 0;

    if(x == 0) {
        return 0;
    }
    else if(x == 1) {
        return 1;
    }
    else {
        a = fib(x - 1);
        b = fib(x - 2);
        return a + b;
    }
}
