cmake_minimum_required(VERSION 2.6)

add_definitions(-Wall -O0 -g)
add_executable(markov main.c markov.c hash.c)
