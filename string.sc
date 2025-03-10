void main(){
    //string output = "Hello World";
    //=>
    char[] output = {72,101,108,108,111,32,87,111,114,108,100};

    //@printStr(output)
    //=>

    int out_length = @length(output);
    for(int i = 0; !(i < out_length); i = i + 1;){
        @output(output[i]);
    }
}