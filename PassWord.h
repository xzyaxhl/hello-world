#pragma once
#include "task.h"
#include "mysqlpool.h"

class PassWord :
    public Task
{
public:
    PassWord();
    ~PassWord();
    void start();
private:
    bool run(std::string&, std::string&);
    bool selectpassword(std::string&);
    bool setpassword(std::string&);
    bool deletepassword(std::string&);
private:
    CMysqlPool* m_imysqlpool;
    string m_query;
    std::map<string, string> m_parse_map;
    std::string m_key;
    std::string m_password;
};

