/**
 * 本文件是用来定义一些在main和testClient中都会用到的一些宏
 */
#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdio.h>
#include <stdio.h>

//调试开关，定义该宏时，工程中各部分为调试信息将会打印出来。
#define DEBUG 1
#if DEBUG
 #define  debug(...) printf("\nFile:%s,  Line:%d,  Function:%s    ",__FILE__,__LINE__,__FUNCTION__);\
 	printf(__VA_ARGS__); fflush(stdout);
#else
 	debug(...)
#endif


//测试开关，定义该宏开启的测试包括：定位模块时间的测量、导引模块时间测量
#define TEST 1

//服务器的IP和端口号
#define IP "121.49.97.136"
#define PORT 40000


#endif
