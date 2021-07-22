#include <unistd.h>
#include <stdio.h>
#include "log.h"
#include "httpserver.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string>
#include <sys/param.h>  
#include "birthday.h"

static void usage()
{
	printf("Usage: ./pa [Options]\n"
		"\nOptions:\n"
		"  -h\t: this help\n"
		"  -c\t: config file\n\n");
	exit(1);
}

void segv_handler(int no)
{
	char buf[512];
	char cmd[512];
	FILE* file;

	snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
	if (!(file = fopen(buf, "r")))
	{
		//exit(EXIT_FAILURE);
	}
	if (!fgets(buf, sizeof(buf), file))
	{
		//eixt(EXIT_FAILURE);
	}
	if (buf[strlen(buf) - 1] == '\n')
	{
		buf[strlen(buf) - 1] = '\0';
	}
	snprintf(cmd, sizeof(cmd), "gdb %s %d\n", buf, getpid());
	system(cmd);
}

int main(int argc, char** argv)
{

	char* buffer;
	//也可以将buffer作为输出参数
	if ((buffer = getcwd(NULL, 0)) == NULL)
	{
		perror("getcwd error");
	}
	else
	{
		printf("%s\n", buffer);
		free(buffer);
	}
	//signal(SIGSEGV, segv_handler);
	char ch;
	std::string configfile = "";
	std::string flagdaemon = "";
	while ((ch = getopt(argc, (char* const*)argv, "hc:")) != -1)
		switch (ch)
		{
		case 'c':
			configfile = optarg;
			break;
		case 'h':
			usage();
			break;
		default:
			usage();
		}
	static GlobalConfig* globelconfig = GlobalConfig::instance(configfile);

	std::string file = argv[0];
#ifdef _DEBUG
	init_daemon(file);
#else
	init_daemon(file);
#endif



	Http::HttpServer httpserver("0.0.0.0", 5888);// , globalconfig);


	if (httpserver.start(1024) < 0) {
		WriteLog(DEBUG, "server create failed\n");
		return -1;
	}

	printf("************************************\n");
	printf("* List of classes of commands:\n\n");
	printf("* CTRL^D -- exit\n");
	printf("************************************\n");
	//birthday* birth = new birthday();
	//birth->Start();

	for (;;) {
		char buff[1024];
		char* cmd = NULL;// fgets(buff, sizeof(buff), stdin);
		if (cmd == NULL)
			sleep(1000);
		continue;
	}
	return 0;
}
