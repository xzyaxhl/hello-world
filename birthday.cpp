#include "birthday.h"
#include "xzypub.h"
#include "lcurl.h"
#include "xzy/smtp.h"
#include "log.h"


birthday::birthday()
{
}

birthday::~birthday()
{
}

void birthday::Run()
{
	
	for (;;)
	{
		file::readfiletojson(BIRTH_FILE, root);
		m_name_birth_mmap.clear();
		for (int i = 0; i < root.size(); i++)
		{
			m_name_birth_mmap[(root[i]["name"].asString())] = root[i]["日期"].asString();
		}
		string str_date = xzy::curdate();
		//转农历
		//https://api.xlongwei.com/service/datetime/nongli.json?day=2020-02-21
		string urlstr = "https://api.xlongwei.com/service/datetime/nongli.json?day=" + str_date;
		string strResponse;
		CURLcode nRes = CommonTools::HttpGet(urlstr, strResponse, 300);
		Json::Value root;
		file::stringtojson(strResponse, root);

		int year = root["lunarYear"].asInt();
		int month = root["lunarMonth"].asInt();
		int day = root["lunarDay"].asInt();
		//rq = ;
		Date date(year, month, day);
		bool isbirth = false;

			string name;
			for (auto ele : m_name_birth_mmap)
			{
				if (date.toStr() == ele.second)
				{
					isbirth = true;
					name = ele.first;
					break;
				}
			}
			string targer = "1184371074@qq.com";
			string ip = "183.3.225.42";//183.3.225.42
			string user = "2982515883@qq.com";
			string pass = "rlxxmmjxodsjdgif";
			Smtp smtp(targer.c_str(),user.c_str(),pass.c_str(),ip.c_str());
			string subject = "生日提醒";
			smtp.setsubject(subject.c_str());
			std::string content = name + ": 日期: " + date.toStr();
			smtp.setcontent(content.c_str());
			smtp.instance(); //发送邮件
		int int_offset = offset();
		WriteLog(INFO, "sleep offset: %d", int_offset);
		xzy::sleep_s(int_offset);
		//is_birth(rq);
		//if  邮件
		//else pass;
		//sleep();
	}
}

int birthday::offset()
{
	std::string str_time = xzy::nowtime();
	XTime xtime(str_time);

	Time timeflag("08:00:00");
	Time newtime = xtime.GetTime();
	Time subtime = newtime - timeflag;

	int sec = 24 * 3600;
	return sec - subtime.toSec();
}
/*
1 12 吴倩敏
1 15 我


6 7 吗
6 10 胡  大舅


8 23 兰
11 29 姐
12 3 爸
12 6 妈
12 30 爸
*/