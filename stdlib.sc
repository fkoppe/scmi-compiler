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


int malloc(int size) {
    int s = size + 4;

    int plast = 0;            // previous block address
    int pcurrent = @FREE;     // current block address

    int pnext = @dref(pcurrent);           // next block address (at offset 0)
    int len = @dref(pcurrent + 4);         // size of current block (stored at offset 4)
    while (pcurrent != 0) {
        if (len >= s) {
            // Found an adequate block
            if (plast == 0) {
                // removing the head block
                @FREE = pnext;
            } else {
                // updating previous block to skip current block
                @sref(plast, pnext);
            }
            // return the address of the usable memory (after metadata)
            return pcurrent;
        } else {
            plast = pcurrent;
            pcurrent = pnext;
        }
    }

    // No suitable block found; allocate new block
    int a = @HP;
    @HP += s;

    @sref(a, s);       // size of block

    // return pointer to memory region (after metadata)
    return a + 4;
}


int free(int address) {
    int s = @dref(address-4);

    // Store metadata (optionals, good practice)
    int of = 0;
    @sref(address, of);           // next pointer (0 since allocated block)
    @sref(address + 4, s);       // size of block

    // Insert the freed block at the front of the free list
    @sref(address, @FREE);
    @FREE = address;

    return 0; // success
}
