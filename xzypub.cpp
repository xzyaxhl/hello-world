#include "xzypub.h"
#include <string>
#include <iostream>
#include <string.h>
#include "log.h"
#include <sys/time.h>
#include <sstream>
#include <fstream>
#include <assert.h>


using namespace std;

namespace xzy
{
	const char *fun(char *a)
	{
		std::string file(a);
		int n = file.rfind(".cpp");
		file.replace(n, 4, ".json");
		return  file.c_str();
	}

	const char *fun1(char *a, char *name)
	{
		std::string file(a);
		std::string dir = path::ptdname(file);
		file = dir + "/" + name;
		return file.c_str();
	}
	void replace(std::string &str, std::string &str1, std::string &str2)
	{
		int n = str.find(str1);
		while (n != -1)
		{
			str.replace(n, str1.size(), str2);
			n = str.find(str1);
		}
	}

	//ȷ����ǰ��û��log�ļ�  �к�׺+1
//�ļ�����׺���ܴ�__�ַ�
//httpserver_2019_10_17.log httpserver_2019_10_17__0.log  httpserver_2019_10_17__1.log
	void GetLogFileName(std::string& logfilename, bool flag)
	{
		if (-1 == access(logfilename.c_str(), F_OK))
		{
			return;
			//��ǰĿ¼�Ҳ�����log��logfilename��log���ļ���
		}
		else
		{
			if (!flag)
			{
				int n1 = logfilename.rfind(".");
				string curdate(xzy::curdate());
				if (logfilename[n1 - 1] != '_')
					logfilename = (logfilename.substr(0, n1) + "_" + curdate + logfilename.substr(n1));
				else
					logfilename = logfilename.substr(0, n1) + curdate + logfilename.substr(n1);
				flag = true;
			}
			else
			{
				int n1 = logfilename.rfind("__");
				if (n1 > -1)
				{
					int n2 = logfilename.rfind(".");
					if (n2 > n1)
					{
						int n = stoi(logfilename.substr(n1 + 2, n2));
						logfilename = logfilename.substr(0, n1 + 2) + to_string(n + 1) + logfilename.substr(n2);
					}
					else
					{
						printf("%s format failed! '.*__**'\n", logfilename.c_str());
						exit(0);
					}
				}
				else
				{
					int n1 = logfilename.rfind(".");
					logfilename = logfilename.substr(0, n1) + "__0" + logfilename.substr(n1);
				}
			}
			GetLogFileName(logfilename, flag);
		}
	}





	//�ػ����� Ĭ�Ͻ�log������ļ���
	void init_daemon(std::string& file, bool flag)
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
			int n = file.rfind("/");
			int n1 = file.rfind(".");
			logfile = file.substr(n + 1, n1 - n - 1) + ".log";
			GetLogFileName(logfile);
		}
		else
		{
			logfile = "log.log";// strncpy(logfile, file, strlen(file));
			GetLogFileName(logfile);
			printf("logfile,%s", logfile.c_str());
		}
		if (flag)
		{
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
		}
		chdir("/");
		return;
	}

	std::string now()
	{
		time_t now1 = time(NULL);
		char buf[32];
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now1));
		string s = buf;
		return s;
	}

	std::string curdate()
	{
		time_t now1 = time(NULL);
		char buf[32];
		strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now1));
		string s = buf;
		return s;
	}



	std::string nowtime()
	{
		time_t now1 = time(NULL);
		char buf[32];
		strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now1));
		string s = buf;
		return s;
	}

	void sleep_s(int s)
	{
		sleep(s);
	}

	void sleep_ms(int ms)
	{
		struct timeval delay;
		delay.tv_sec = 0;
		delay.tv_usec = ms * 1000;
		select(0, NULL, NULL, NULL, &delay);
	}

	void sleep_us(int us)
	{
		struct timeval delay;
		delay.tv_sec = 0;
		delay.tv_usec = us;
		select(0, NULL, NULL, NULL, &delay);
	}

	long unix_timestamp_s()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec;
	}
	long unix_timestamp_ms()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}
	long unix_timestamp_us()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);

		return tv.tv_sec * 1000000 + tv.tv_usec;
	}

	time_t StringToDatetime(string str)
	{
		char* cha = (char*)str.data();             // 将string转换成char*。
		tm tm_;                                    // 定义tm结构体。
		int year=0, month=0, day=0, hour=0, minute=0, second=0;// 定义时间的各个int临时变量。
		if (str.find('-') != std::string::npos)
			sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
		else if (str.find('/') != std::string::npos)
			sscanf(cha, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
		else {
			printf("输入格式有误: %d/%d/%d %d:%d:%d or %d-%d-%d %d:%d:%d \n");
		}
		tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
		tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
		tm_.tm_mday = day;                         // 日。
		tm_.tm_hour = hour;                        // 时。
		tm_.tm_min = minute;                       // 分。
		tm_.tm_sec = second;                       // 秒。
		tm_.tm_isdst = 0;                          // 非夏令时。
		time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式。
		return t_;                                 // 返回值。
	}

	string Left(string str, int x, string f = "0")
	{
		string temp = "";
		for (int i = x; i > str.size(); i--)
		{
			temp += f;
		}
		return temp + str;
	}

	string Left(int n, int x)
	{
		return Left(to_string(n), x);
	}

	string DatetimeToString(time_t time)
	{
		tm* tm_ = localtime(&time);                // 将time_t格式转换为tm结构体
		int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
		year = tm_->tm_year + 1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900。
		month = tm_->tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1。
		day = tm_->tm_mday;                        // 临时变量，日。
		hour = tm_->tm_hour;                       // 临时变量，时。
		minute = tm_->tm_min;                      // 临时变量，分。
		second = tm_->tm_sec;                      // 临时变量，秒。
		char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// 定义时间的各个char*变量。
		sprintf(yearStr, "%d", year);              // 年。
		sprintf(monthStr, "%d", month);            // 月。
		if (monthStr[1] == '\0')
		{
			monthStr[2] = '\0';
			monthStr[1] = monthStr[0];
			monthStr[0] = '0';
		}
		sprintf(dayStr, "%d", day);                // 日。
		if (dayStr[1] == '\0')
		{
			dayStr[2] = '\0';
			dayStr[1] = dayStr[0];
			dayStr[0] = '0';
		}
		sprintf(hourStr, "%d", hour);              // 时。
		if (hourStr[1] == '\0')
		{
			hourStr[2] = '\0';
			hourStr[1] = hourStr[0];
			hourStr[0] = '0';
		}
		sprintf(minuteStr, "%d", minute);          // 分。
		if (minuteStr[1] == '\0')                  // 如果分为一位，如5，则需要转换字符串为两位，如05。
		{
			minuteStr[2] = '\0';
			minuteStr[1] = minuteStr[0];
			minuteStr[0] = '0';
		}
		sprintf(secondStr, "%d", second);          // 秒。
		if (secondStr[1] == '\0')                  // 如果秒为一位，如5，则需要转换字符串为两位，如05。
		{
			secondStr[2] = '\0';
			secondStr[1] = secondStr[0];
			secondStr[0] = '0';
		}
		char s[20];                                // 定义总日期时间char*变量。
		sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// 将年月日时分秒合并。
		string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。
		return str;                                // 返回转换日期时间后的string变量。
	}

	int GetWeekNum(string str)
	{
		/*
		0-53
		周一到周日为一周
		1------7
		*/
		time_t t = StringToDatetime(str);
		//cout << t << endl;
		struct tm* ptm;
		ptm = localtime(&t);
		char weeknum[3];
		strftime(weeknum, sizeof(weeknum), "%W", ptm);
		return atoi(weeknum);
	}

	bool isdigitstr(string str)
	{
		/*for (auto i : str)
		{
			if (!isdigit(i))
				return false;
		}*/
		for (int i = 0; i < str.size(); i++)
		{
			if (!isdigit(str[i]))
				return false;
		}
		return true;
	}

	int GetSecond(string str)
	{
		if (str.size() != 8 || str[2] != ':' || str[5] != ':')
			return -1;
		string hourstr, minutestr, secondstr;
		hourstr = str.substr(0, 2);
		if (!isdigitstr(hourstr)) {
			printf("hour error,hour: %s\n", hourstr.c_str());
			return -1;
		}
		minutestr = str.substr(3, 2);
		if (!isdigitstr(minutestr)) {
			printf("minutestr error,minutestr: %s\n", minutestr.c_str());
			return -1;
		}
		secondstr = str.substr(6, 2);
		cout << "seconde:  ["<<secondstr<<"]" << endl;
		if (!isdigitstr(secondstr)) {
			printf("secondstr error,secondstr: %s\n", secondstr.c_str());
			return -1;
		}
		int hour, minute, second;
		hour = atoi(hourstr.c_str());
		minute = atoi(minutestr.c_str());
		second = atoi(secondstr.c_str());
		if (minute > 59)
			printf("minute:%d\n", minute);
		if (second > 59)
			printf("second: %d\n", second);

		return hour * 3600 + minute * 60 + second;
	}

	string GetTime(int sec)
	{
		if (sec < 0)
		{
			printf("second in %d\n", sec);
			return "";
		}
		int second, minute, hour;
		second = sec % 60;
		sec = sec / 60;
		minute = sec % 60;
		hour = sec / 60;
		return Left(hour, 2) + ":" + Left(minute, 2) + ":" + Left(second, 2);
	}

	bool IsDate(string str)
	{
		int months1[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
		int months2[12] = { 31,29,31,30,31,30,31,31,30,31,30,31 };
		int boolday = 0;
		if (str.find('-') != std::string::npos)
		{
			if (str.size() != 10 || str[4] != '-' || str[7] != '-')
				return false;
		}
		else
		{
			if (str.size() != 10 || str[4] != '/' || str[7] != '/')
				return false;
		}
		string yearstr, monthstr, daystr;
		yearstr = str.substr(0, 4);
		if (!isdigitstr(yearstr)) {
			printf("yearstr error,yearstr: %s\n", yearstr.c_str());
			return false;
		}
		monthstr = str.substr(5, 2);
		if (!isdigitstr(monthstr)) {
			printf("monnthstr error,minutestr: %s\n", monthstr.c_str());
			return false;
		}
		daystr = str.substr(8, 2);
		if (!isdigitstr(daystr)) {
			printf("daystr error,daystr: %s\n", daystr.c_str());
			return false;
		}
		int year, month, day;
		year = atoi(yearstr.c_str());
		month = atoi(monthstr.c_str());
		day = atoi(daystr.c_str());
		if (year % 4 == 0 && year % 100 != 0)//闰年
			boolday = 29;

		if (month > 12)
		{
			printf("error month:%d\n", month);
			return false;
		}
		if (month != 2)
			boolday = months1[month - 1];
		if (day > boolday)
		{
			printf("error day: %d\n", day);
			return false;
		}
		return true;
	}
}

namespace path
{
	void ptfname(std::string& dir, std::string& file, const std::string& path)
	{
		ptfname(dir, file, path.c_str());
	}

	void ptfname(std::string& dir, std::string& file, const char* path)
	{
		char* base = strdup(path);
		dir = dirname(base);
		free(base);
		base = strdup(path);
		file = basename(base);
		free(base);
	}
	string ptfname(std::string path)
	{
		char* base = strdup(path.c_str());
		string file = basename(base);
		free(base);
		return file;
	}
	void ptfname(string& file, const char* path)
	{
		char* base = strdup(path);
		file = basename(base);
		free(base);
	}
	//·��ֻ���ļ���  a.txt   ����.
	void ptdname(string& dir, const char* path)
	{
		char* base = strdup(path);
		dir = dirname(base);
		free(base);
	}
	//·��ֻ���ļ���  a.txt   ����. Ĭ����./a.txt
	string ptdname(std::string path)
	{
		char* base = strdup(path.c_str());
		string temp = dirname(base);
		free(base);
		return temp;
	}
}


namespace url
{
	unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
	}

	std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}

	std::string UrlDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				assert(i + 2 < length);
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}

	void SetQuery(const string& url, string& query)
	{
		int npos = url.find('?');
		if (npos == std::string::npos)
			return;
		query = url.substr(npos + 1);
	}


	void SetParse(const string& query, std::map<std::string, std::string>& ssmap)
	{
		string s = query;
		string s1;
		int npos = 1;
		int npos1 = 1;
		int len = s.size();
		while (npos)
		{
			npos = s.find('&');
			npos1 = s.find('=');
			if (npos1 == std::string::npos)
				break;
			ssmap[s.substr(0, npos1)] = s.substr(npos1 + 1, (npos == std::string::npos) ? len : npos - npos1 - 1);
			cout << s.substr(0, npos1) << "   :" << s.substr(npos1 + 1, (npos == std::string::npos) ? len : npos - npos1 - 1) << endl;

			if (npos == std::string::npos)
				break;
			s = s.substr(npos + 1);
			len = s.size();
		}
	}
}


namespace str
{
	std::vector<std::string> split(const std::string& s, char delim)
	{
		vector<string> elems;
		stringstream ss(s);
		string item;
		while (getline(ss, item, delim))
		{
			if (item[0] == delim)
				item = item.substr(1, item.size() - 1);
			if (item == "")
				continue;
			
			elems.push_back(item);
		}
		return elems; 
	}

	std::vector<std::string> split_1(const std::string& s ,char delim)
	{
		std::vector<std::string> vec;
		const char* start = s.c_str();
		const char* end = start + s.size();
		int startlen = 0;
		int endlen = 0;
		string str = "";
		for (; start != end; start++)
		{

			if (start[0] == delim || start == end - 1)
			{
				if (start == end - 1)
					endlen++;

				str = s.substr(startlen, endlen - startlen);
				startlen = endlen + 1;
				if (str.size() != 0)
					vec.push_back(str);
			}
			endlen++;
		}
		return vec;
	}

	void split(std::string& str, std::vector<std::string>& vec, std::string& delim)
	{
		split(str, vec, delim.c_str());
	}

	void split(std::string& str, std::vector<std::string>& vec, const char* delim)
	{
		string de(delim);
		vec.clear();
		int startpos = 0;
		int endpos = 0;
		std::string strtemp;
		while (true)
		{
			if ((endpos = str.find_first_of(de, startpos)) != std::string::npos)
			{
				if (endpos != startpos)// //clocket//fs?
				{
					strtemp = str.substr(startpos, endpos - startpos);
					vec.push_back(strtemp);
				}
				startpos = endpos + 1;
			}
			else
			{
				if (endpos != startpos)
				{
					strtemp = str.substr(startpos, str.size());
					vec.push_back(strtemp);
				}
				break;
			}
		}
	}
}

namespace file
{
	bool existsfile(const char* filename)
	{
		if (-1 == access(filename, F_OK))
		{
			printf("Xfile:[ %s ] was not found!\nPress any key to exit!", filename);
			return false;
		}
		return true;
	}


	std::vector<std::string> readfileline(std::string filename)
	{
		vector<std::string> filelinelist;
		ifstream in(filename);
		std::string line;
		if (in)
		{
			while (getline(in, line))
			{
				line.pop_back();
				filelinelist.push_back(line);
			}
		}
		else
		{
			cout << "[readfileline] no such file" << endl;
		}
		return filelinelist;
	}
	bool readfiletojson(std::string filename, Json::Value& root)
	{
		readfiletojson(filename.c_str(), root);
	}
	bool readfiletojson(const char* filename, Json::Value& root)
	{
		ifstream ifs;
		ifs.open(filename);
		Json::Reader reader;

		if (reader.parse(ifs, root, false))
		{
			ifs.close();
			return true;
		}
		ifs.close();
		return false;

	}
	std::string jsontostring(Json::Value& value)
	{
		Json::FastWriter fastwriter;
		return fastwriter.write(value);
	}

	std::string jsontostringstyle(Json::Value& value)
	{
		Json::StyledWriter stylewriter;
		return stylewriter.write(value);
	}
	void jsontofile(const char* filename, Json::Value& jsonRoot)
	{
		ofstream ofs;
		ofs.open(filename);
		ofs << jsonRoot.toStyledString();
		ofs.close();
	}

	void stringtojson(string& str, Json::Value& jsonRoot)
	{
		Json::Reader reader;
		if (reader.parse(str, jsonRoot))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
		{
			printf("stringtojson() parse str failed!str:%s\n",str.c_str());
		}
	}

}
