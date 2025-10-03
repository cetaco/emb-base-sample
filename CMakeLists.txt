cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
zephyr_include_directories(.) 
project(emb-base-sample-main)
target_sources(app PRIVATE src/main.c)

