#pragma once
#ifndef __BIRTHDAY_H__
#define __BIRTHDAY_H__
#include <iostream>
#include <json/json.h>
#include "thread.h"
#include "XTime.h"

#define BIRTH_FILE "../../../birth.json"

class birthday:public Pthread::Thread
{
public:
	birthday();
	~birthday();

private:
	void Run() override;
	int offset();
	std::map <std::string, std::string> m_name_birth_mmap;
	Json::Value root;
};

#endif // !__BIRTHDAY_H__



