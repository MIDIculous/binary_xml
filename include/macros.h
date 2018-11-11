#pragma once

#include <iostream>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "ANSI.h"

#ifndef nullptr
    #define nullptr NULL
#endif
#define INT_NULL_VALUE -0x80000000

#define ERRNO_SHOW(code,command,params) {int __e = errno;std::cerr << ANSI_RED_BRIGHT "[" << (code) << "] Error: " command "(" << params << ") - " << __e << ": " << strerror(__e) << ANSI_RESET_LF;}



#ifndef NDEBUG
    #define ASSERT_NO_(code,condition,action) do {if (!(condition)) { std::cerr << ANSI_RED_BRIGHT "[" << (code) << "]" ANSI_RED_DARK " Assertion " << #condition << " failed!" ANSI_RESET_LF;action;}} while(0)
#else
    #define ASSERT_NO_(code,condition,action) do {if (!(condition)) { std::cerr << ANSI_RED_BRIGHT "[" << (code) << "]" ANSI_RED_DARK " Assertion failed!" ANSI_RESET_LF;action;}} while(0)
#endif

#define ASSERT_NO_RET_FALSE(code,condition) ASSERT_NO_(code,condition,return false)
#define ASSERT_NO_RET_NULL(x,condition)    ASSERT_NO_(x,condition,return NULL)
#define ASSERT_NO_RET_0(x,condition)        ASSERT_NO_(x,condition,return 0)
#define ASSERT_NO_RET_1(x,condition)        ASSERT_NO_(x,condition,return 1)
#define ASSERT_NO_RET_N1(x,condition)        ASSERT_NO_(x,condition,return -1)
#define ASSERT_NO_RET(x,condition)        ASSERT_NO_(x,condition,return)
#define ASSERT_NO_EXIT_1(x,condition)        ASSERT_NO_(x,condition,_exit(1))
#define ASSERT_NO_DO_NOTHING(x,condition)        ASSERT_NO_(x,condition,{})


#define STRCPY(dst,src) {strncpy(dst,src,sizeof(dst));dst[sizeof(dst)-1] = '\0';}

#define LOG_ERROR(fmt,...) fprintf(stderr,ANSI_RED_BRIGHT "Error:" fmt ANSI_RESET_LF,__VA_ARGS__)
#define LOG(fmt,...) fprintf(stderr,fmt ANSI_RESET_LF,__VA_ARGS__)


#define MINIMIZE(x,y) do{if ((y) < (x)) (x) = (y);}while(0)
#define MAXIMIZE(x,y) do{if ((y) > (x)) (x) = (y);}while(0)

//#define exit _exit
