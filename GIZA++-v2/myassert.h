#ifndef MY_ASSERT_DEFINED
#define MY_ASSERT_DEFINED
void myerror(int line,const char *file,const char *expression);
void imyerror(int line,const char *file,const char *expression);

#define iassert(expression) do {if (!(expression)) {imyerror(__LINE__,__FILE__,#expression);}} while (0)

#
#define massert(expr) do {} while(0)  //所以我对这里的定义有些疑惑，这里貌似是什么都不做的样子

#define vassert(expr) do {} while(0)

#include <cassert>

#endif





