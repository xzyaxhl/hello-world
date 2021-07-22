#pragma once
#include <string>
#include <time.h>
#include <libgen.h>
#include <vector>
#include <map>
#include "json/json.h"

using namespace std;

namespace xzy
{
	//���øú�����ȵ������ļ����ļ���
#define GetFilename() fun(__FILE__)
	const char *fun(char *a);
	/*Ĭ����json��ʽ*/

#define GetName(name) fun1(__FILE__,name)
	const char *fun1(char *a, char *name);

	void replace(std::string &str, std::string &str1, std::string &str2);


	//ȷ����ǰ��û��log�ļ�  �к�׺+1
//�ļ�����׺���ܴ�__�ַ�
//httpserver_2019_10_17.log httpserver_2019_10_17__0.log  httpserver_2019_10_17__1.log
	void GetLogFileName(std::string& logfilename, bool flag = false);

	//�ػ����� Ĭ�Ͻ�log������ļ���
	void init_daemon(std::string& file, bool flag = true);

	//��ʱ
	void sleep_s(int s);
	void sleep_ms(int ms);
	void sleep_us(int us);
	//2020-11-24 08:12:12
	std::string now();
	//2020-11-24
	std::string curdate();
	//08:12:12
	std::string nowtime();

	//���ص�ǰʱ��� 
/*
now_s�� 1566750627737923
now_ms��1566750627737
now_us��1566750627
*/
	long unix_timestamp_s();
	long unix_timestamp_ms();
	long unix_timestamp_us();
	//2021-01-02 08:12:22		1609546342
	//2021/01/09 08:12:22		1609546342
	time_t StringToDatetime(string str);
	//1609546342		2021-01-02 08:12:22
	string DatetimeToString(time_t time);

	int GetWeekNum(string str);

	bool isdigitstr(string str);

	int GetSecond(string str);


	string Left(string str, int x, string f );

	string Left(int n, int x);
	
	string GetTime(int sec);

	bool IsDate(string str);
}

namespace path
{

	void ptfname(std::string& dir, std::string& file, const std::string& path);
	void ptfname(std::string& dir, std::string& file, const char* path);
	std::string ptfname(std::string path);
	void ptfname(std::string& file, const char* path);

	void ptdname(std::string& dir, const char* path);
	std::string ptdname(std::string path);
}



namespace url
{
	unsigned char ToHex(unsigned char x);

	unsigned char FromHex(unsigned char x);

	std::string UrlEncode(const std::string& str);

	std::string UrlDecode(const std::string& str);

	//����url����Ĳ���   ? 
	void SetQuery(const std::string& url, std::string& query);
	//����������map��
	void SetParse(const std::string& query, std::map<std::string, std::string>& ssmap);

}

namespace str
{
	//�����ŷָ�string
	std::vector<std::string> split(const std::string& s, char delim);
	std::vector<std::string> split_1(const std::string& s,char delim);
	void split(std::string& str, std::vector<std::string>& vec, std::string &delim);

	void split(std::string& str, std::vector<std::string>& vec, const char* delim);

}
namespace file
{
	bool existsfile(const char* filename);


	//��ȡ�ļ��е���,һ��vecԪ��Ϊһ��,ȥ����β�����з�
	std::vector<std::string> readfileline(std::string filename);
	//
	bool readfiletojson(std::string filename,Json::Value&);

	bool readfiletojson(const char* filename, Json::Value&);
	std::string jsontostring(Json::Value& value);

	std::string jsontostringstyle(Json::Value& value);

	void jsontofile(const char* filename, Json::Value& jsonRoot);

	void stringtojson(string& str, Json::Value& jsonRoot);
}