#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <string>
#include <unistd.h>
#include <map>
#include <stdint.h>
#include <json/json.h>
#include <vector>

class GlobalConfig
{
public:
	typedef Json::Value Jsona;
	typedef Json::Reader Jsonr;
	typedef Json::Writer jsonw;

	typedef std::map<std::string, std::string> map_ss;
	struct Vp {
		std::string key;
		std::string value;
	};
	std::vector<Vp> de_vpath;

	struct Mysql
	{
		std::string host;
		uint16_t  port;
		std::string user;
		std::string pass;
		std::string db;
	}mysql;

	std::string ip;
	uint16_t port;

	map_ss vp_sql;


	static GlobalConfig *instance(std::string configfilepath="./config.json");//����ʽ
	bool loadconfig(Jsona &);
private:
	GlobalConfig();
	GlobalConfig(std::string &filepath);
	std::string m_filepath;
};
#endif // !__CONFIG_H__

