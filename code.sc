  void main(   int y, int z){
    int x = 0x5;
    long y=3;

    const int[5  ] y = {1,2,3,4,5};
    y[4] = 4;

    print(y);
}

void print(int[] output){
    @R0 = output[0];
    @R1 = output[1];
    @R2 = output[2];
    ...
}