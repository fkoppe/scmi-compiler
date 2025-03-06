void main() {
    int[] test = {1,2,3,4,5};
    int a = 4;
    int b = 2;
    int c = 0;
    int d = 0;

    a = test[0];
    b = test[4];

    test[1] = 42;
    c = test[1];

    test[2] = 24;
    d = test[2];

    test[0] = test[4];
    int e = test[0];

    @output(a,b,c,d,e);
}
