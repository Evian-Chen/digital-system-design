使用g++ -o robdd robdd.cpp直接編譯整個robdd.cpp，並使用robdd pla_4_vars.pla
 pla_4_vars.dot與robdd pla_5_vars.pla
 pla_5_vars.dot執行，此程式會讀入pla檔案並輸出一個dot檔案，得到dot之後就可以用dot -T png pla_4_vars.dot > pla_4_vars.png以及dot -T png pla_5_vars.dot > pla_5_vars.png 指令產生png。