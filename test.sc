void main() {
    int a = 65;
    int b = 2;
    int c = 1;

    // Arithmetic operations
    int result = a * b + 4 % 2; // 65 * 2 + 0 = 130
    @output(result); // Outputs: '�' (ASCII 130, non-printable character)

    // Logical operations
    if ((a > 60 && b < 3) || c == 0) {
        @output(88); // Outputs: 'X'
    }

    // Arrays and function calls
    char[] array = {67, 68, 69, 70}; // ASCII 'C', 'D', 'E', 'F'
    int arrLength = @length(array);
    @output(arrLength + 48); // Outputs: '4'

    // Loop structures
    for(int i = 0; i < arrLength; i++){
        @output(array[i]); // Outputs: CDEF
    }

    int counter = 0;
    while (counter < arrLength) {
        @output(array[counter] + 1); // Outputs: DEFG
        counter = counter + 1;
    }

    // Function usage
    int funcResult = funcTest(array[0]);
    @output(funcResult); // Outputs: 'D'
}

int funcTest(int x){
    return x + 1;
}
