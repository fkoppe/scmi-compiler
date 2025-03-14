void main(){
    string test = "Hello World";
    int c = 1;
    int x = !c;
    print(x);
    if(x){
        x++;
    }

    int a = malloc(4);
    int ninenine = 959;
    @sref(a, ninenine);

    int x2 = @dref(a);

    print(x2);
}
