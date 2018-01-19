# Java-like-compiler
A Java-like compiler

## Install and Run Program
* Build Program:
```shell
make genast
make
```
* Run Tests:
```shell
make run
```
* Clean:
```shell
make clean
```
### To Compile and Run your code
```shell
./lang < test.lang > code.s
gcc -m32 -o test tester.c code.s
```
