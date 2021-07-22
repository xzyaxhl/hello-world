#include "config.h"
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>


GlobalConfig::GlobalConfig()
{

}
GlobalConfig::GlobalConfig(std::string& filepath) :m_filepath(filepath)
{
	if (-1 == access(m_filepath.c_str(), F_OK))
	{
		printf("config file [%s] not find!\nPress any key to exit!", m_filepath.c_str());
		getchar();
		exit(0);
	}

	struct stat filestat;
	stat(m_filepath.c_str(), &filestat);
	const size_t filesize = filestat.st_size;

	char* fbuff = new char[filesize + 1]();

	assert(fbuff != NULL);
	FILE* fp = fopen(filepath.c_str(), "rb");

	if (fp == NULL || fread(fbuff, filesize, 1, fp) != 0x01)
	{
		delete[]fbuff;
		fclose(fp);
	}
	Jsona root;
	Jsonr reader;
	reader.parse(fbuff, root);
	loadconfig(root);
	delete[]fbuff;
	fclose(fp);
}

bool GlobalConfig::loadconfig(Jsona& root)
{
	Jsona node;
	ip = root["ip"].asString();
	port = root["port"].asInt();
	std::cout << ip << "  " << port << std::endl;
	if (root.isMember("mysql") && root["mysql"].isObject())
	{
		node = root["mysql"];
		mysql.host = node["host"].asString();
		mysql.pass = node["pass"].asString();
		mysql.port = node["port"].asInt();
		mysql.db = node["db"].asString();
		mysql.user = node["user"].asString();
		std::cout << mysql.host << mysql.user << std::endl;
	}
	else
		return false;
	if (root.isMember("vsql") == 1 && root["vsql"].isArray() == 1) //isArray不存在时返回true,isMember vsql后面跟数字（vsql1）true，
	{
		node = root["vsql"];
		int i = 0;
		Jsona node1;
		for (i; i < node.size(); i++)
		{
			node1 = node[i];
			Vp temp;

			if (node1.isObject())
			{
				temp.key = node1["desc"].asString();
				temp.value = node1["vpath"].asString();
				de_vpath.push_back(temp);
				vp_sql.insert(make_pair(node1["vpath"].asString(), node1["sql"].asString()));
			}
		}
	}
	else
		return false;
	return true;
}

GlobalConfig* GlobalConfig::instance(std::string configfile)
{
	static GlobalConfig* m_globalconfig = NULL;
	if (m_globalconfig == NULL)
		m_globalconfig = new GlobalConfig(configfile);
	return m_globalconfig;
}
