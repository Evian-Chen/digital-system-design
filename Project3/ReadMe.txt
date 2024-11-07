This program can generate pla file by doing the following commands:

1. compile
g++ b11110007.cpp -o b11110007

2. add input/output
b11110007 input.kiss output.kiss output.dot

3. generate state transition graph
dot -T png output.dot > output.png

then you can see a output.kiss, an output.dot and a STG in the current file directory.