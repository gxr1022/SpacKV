#!/bin/sh
# This is a comment!
# MY_MESSAGE="Hello World"
# echo $MY_MESSAGE
# x="hello"
# # expr $x + 1
# # read MY_MESSAGE
# echo "Hello $MY_MESSAGE - hope you're well."
# MYVAR="hi there"
# echo "MYVAR is: $MYVAR"
# student=(L W G Z L)
# echo ${student[0]}
# echo ${student[@]}
# echo ${student[*]}
# # Arithmetic Operators
# # Relational Operators
# # Boolean Operators
# # String Operators
# # File Test Operators
# val=`expr 2 + 2`
# echo "$val"
# a=10
# b=20
# val=`expr $a + $b`
# echo "$val"
# val=`expr $a != $b`
# echo "$val"

# #所有条件表达式应该放在[]里面
# [ $a -eq $b ]

mkdir -p build & cd build
cmake -DCMAKE_BUILD_TYPE=debug .. &&cmake --build .
gdb ./db_bench