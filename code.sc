void main(){
    println(100);
    println(123);
    println(34);
    for(int i = 0; i < 1; i++){
        println(fibo(i));
    }
}

int fibo(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n == 1 || n == 2) {
        return 1;
    }
    return fibo(n - 1) + fibo(n - 2);
}
