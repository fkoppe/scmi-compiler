void prints(char[] str) {
    int len = @length(str);

    for(int i = 0; i < len; i++) {
        printc(str[i]);
    }
}

void printsln(char[] str) {
    prints(str);
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
    if (integer == 0) {
        @output(48); // ASCII '0'
        return;
    }

    if (integer < 0) {
        @output(45); // ASCII '-'
        integer = 0-integer; // Make it positive for processing
    }

    // Store digits in an array (since recursion or decrementing might not be allowed)
    int[] digits = int[10]; // Enough space for an int (assuming 32-bit)
    int index = 0;

    while (integer != 0) {
        digits[index] = 48 + (integer % 10) ; // Store ASCII character
        integer /= 10;
        index++;
    }

    // Print stored digits in the correct order
    int i = index - 1;
    while (i >= 0) {
        @output(digits[i]);
        i = i - 1; // Manual decrement
    }
}

void println(int integer) {
    print(integer);
    @output(10);
}
