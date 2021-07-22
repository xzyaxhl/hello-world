
#ifndef __PRINTLOG_H__
#define __PRINTLOG_H__
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <string>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <thread>
//#include "spdlog/spdlog.h"


static int si = 1;
#define WriteEnter\
	WriteLog(INFO, "enter")
#define WriteExit\
	WriteLog(INFO, "exit")


#if 0
#define LOG1 (printf("%d: file: %s-----func: %s_____%d,thread_id: %d",si++,__FILE__,__FUNCTION__,__LINE__,pthread_self()),printf)

#define trace(fmt, ...) spdlog::info(fmt)
#define error(fmt, ...) spdlog::error(fmt)
#define errsys(fmt, ...) spdlog::warn(fmt)
#else

//#define LOG1 (printf("%d: file: %s-----func: %s_____%d,thread_id: %d",si++,__FILE__,__FUNCTION__,__LINE__,pthread_self()),printf)

#define PRINT(title, file, func, line, tick, fmt, ...) \
	printf("[%s] - %s:%s:%d %s, " fmt, title, file, func, line, tick, ##__VA_ARGS__)
#define PRINT_INFO(title, tick, fmt, ...) \
	PRINT(title, __FILE__, __FUNCTION__, __LINE__, tick, fmt, ##__VA_ARGS__)



#define trace(fmt, ...) PRINT_INFO("trace", "", fmt, ##__VA_ARGS__)
#define error(fmt, ...) PRINT_INFO("error", "", fmt, ##__VA_ARGS__)
#define errsys(fmt, ...) PRINT_INFO("error", strerror(errno), fmt, ##__VA_ARGS__)

#define trace(fmt, ...)
#define error(fmt, ...)
#define errsys(fmt, ...)
#endif

#define ERR_EXIT(m) \
do\
{\
    perror(m);\
    exit(EXIT_FAILURE);\
}\
while (0);\

#define MAX_MESG_LEN   2048
enum LogType
{
	DEBUG = 0x00,
	INFO = 0x01,
	WARN = 0x02,
	ERROR = 0x03,
	CRIT = 0x04
};

static const char * LOG_STR[] = {
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"CRIT"
};


#ifdef _DEBUG
#define log_level -1
#else
#define log_level -1
#endif

#define WriteLog1(level, format, ...) do{ \
    if (level > log_level) {\
        time_t now = time(NULL); \
        char msg[MAX_MESG_LEN]; \
        char timebuf[32]; \
        sprintf(msg, format, ##__VA_ARGS__); \
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
        fprintf(stdout, "[%s] [%s] [%s] [%d] %s\n", timebuf,LOG_STR[level],__FILE__,__FUNCTION__,__LINE__, msg); \
        fflush(stdout); \
    } \
    if (level == ERROR) {\
        exit(-1); \
    } \
} while(0) 


#define WriteLog(level, format, ...) do{ \
	struct timeval tv; \
	gettimeofday(&tv,NULL);\
    if (level > log_level) {\
        time_t now = time(NULL); \
        char msg[MAX_MESG_LEN]; \
        char timebuf[32]; \
        sprintf(msg, format, ##__VA_ARGS__); \
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
        fprintf(stdout, "[%s.%u] [%s] [%d] [%s] [%s] [%d] %s\n", timebuf,tv.tv_usec,LOG_STR[level],std::this_thread::get_id(),__FILE__,__FUNCTION__,__LINE__, msg); \
        fflush(stdout); \
    } \
    if (level == ERROR) {\
        exit(-1); \
    } \
} while(0) 

static void init_daemon(std::string &file)
{



		int pid;
		int i;
		if (pid = fork())
			exit(0);
		else if (pid < 0)
			exit(1);
		setsid();
		if (pid = fork())
			exit(0);
		else if (pid < 0)
			exit(1);
	
	std::string logfile = "";
	int fd;
	if (file != "")
	{
		printf("file:%s\n", file.c_str());

		int n = file.rfind("/");
		int n1 = file.rfind(".");
		logfile = file.substr(n + 1,n1-n-1)+".log";
		
		printf("logfile:::%s\n", logfile.c_str());
	}
	else
	{
		logfile = "pa.log";// strncpy(logfile, file, strlen(file));
		printf( "logfile,%s", logfile.c_str());
	}

	printf( "logfile:: %s\n", logfile.c_str());
	if ((fd = open(logfile.c_str(), O_RDWR | O_APPEND | O_CREAT, 0)) != -1)
	{
		WriteLog(DEBUG, "logfile:%s ok!", logfile.c_str());
		dup2(fd, STDOUT_FILENO);
		if (fd > STDERR_FILENO)
			close(fd);
	}
	else
	{
		WriteLog(DEBUG, "logfile:%s failed!", logfile.c_str());
	}
	chdir("/");
	return;
}


#endif