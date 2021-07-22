#ifndef __PARAMCHECK_H__
#define __PARAMCHECK_H__

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string>
#include <assert.h>
#define INVALID_FD(fd) (fd < 0)
#define INVALID_POINTER(p) (p == NULL)

// 禁止拷贝基类
class IUncopyable
{
	public:
		IUncopyable(){};
	private://
		IUncopyable(IUncopyable &);
		IUncopyable & operator=(const IUncopyable&);//赋值运算
};
//因为派生类复制（或者赋值），一定要先复制基类。

//所以，通过抑制基类复制行为可以达到抑制派生类复制行为的目的。

// desc: 设置errno
// param: eno/需要设置的errno值
// return: void
inline void seterrno(int eno)
{
	errno = eno;
}

#endif
