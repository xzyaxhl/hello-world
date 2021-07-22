#ifndef __CLOCKTIME_H__
#define __CLOCKTIME_H__

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include "http.h"
//#include "clocktime.h"
#include "log.h"
#include "json/json.h"

#include "task.h"
#include "log.h"
#include "mysqlpool.h"
#include "XTime.h"

#define ABSPATH "/home/xuzhiyuan/clocktime/"
#define USERFILE ABSPATH "user.json"
#define TIMEFILE ABSPATH "time.json"

using namespace std;
void calculatesub(std::vector<std::vector<std::string>>& vec);
//计算有效时间
Time calculatevalidtime(Time time1, Time time2);
//加入周统计和月底统计
void calculateweektime(std::vector<std::vector<std::string>>& vec);
struct priod {
	std::string time1;
	std::string time2;
};

static std::vector<priod> validtime= {
	{"08:00:00","12:00:00"},
	{"13:30:00","17:30:00"},
	{"18:00:00","24:00:00"}
};

struct UserInfo{
	std::string user;
	std::string pass;
	std::string mail;
	std::string userid;
};

class ClockTime:public Task
{
	enum ConnType
	{
		work,
		adduser,
		deleteuser,
		deleteid,
		settime
	};
public:
	ClockTime();
	~ClockTime();
	void start();
private:
	string getfilename(const string& url);
	void formatcontext(shared_ptr<Http::IHttpResponse>, std::string& context);
	void writetofile(string filename, string context);
	std::string readtofile(string filename);

	bool run(std::string&, std::string&);
	bool Work(std::string&);
	bool AddUser(std::string &);
	bool DeleteUser(std::string&);
	bool DeleteId(std::string&);
	bool DeleteRecode(std::string&);
	bool SetTime(std::string&);

	void SetReponse200(shared_ptr<Http::IHttpResponse>, std::string& context);
	std::string jsontohtml(Json::Value& root);

	void SelectUser();
	bool isUser(std::string& user,std::string &pass, std::string mail ="");
	std::string getUserId(const std::string& user);
private:
	ConnType m_type;
	string m_query;
	std::map<string, string> m_parse_map;
	CMysqlPool* m_imysqlpool;

	std::string m_ruser;
	std::string m_password;

	std::string m_curtime;
	std::string m_starttime;
	std::string m_endtime;
	std::map<std::string, UserInfo> m_user_pass;
	bool m_UserPassFlag=false;
};

#endif // !__CLOCKTIME_H__

/*

clockintime/?
clockintime/work?user=�û���&pass=����
ɾ���û�
deleteuser?user=�û���&pass ר�ŵ�����
root ����û�
clockintime/adduser?user=����û���,password=�û�����&����&root=& pass
clockintime/deleteid?user=�û���&pass=1224&date=����&id=�ڼ�����¼
 Ĭ��ʱ��Ϊ����curdate,�����,��Ϊdate
clockintime/settime? user=�û���&pass=1224&type=invalid&starttime=&endtime=
clockintime/settime? user=�û���&pass=1224&type=valid&starttime=&endtime=
���� ��Чʱ�����Ч
�ʼ�֪ͨ��.
�Ӱ�
 �ж��ǲ��ǼӰ�, ����һ(�Ƿ�������)���ʼ� ,�ļ� �ǲ��Ƿ�������.txt,
�쳣
 ����򿨵�����, �ڶ��췢���ʼ�
ÿ�������Ĵ�
���
����   �ϰ��   �°�  ���칤ʱ
20201123  08:31:30 18:38:00 ���㹤ʱ  |  |  |
�ϼ� ���� ƽ���ϰ�ʱ��
  ���� ƽ��ÿ���ϰ���

clocktime
	user.json
		{
			info:[
			{name=xuzhiyuan,pass=12345,mail=},
			{name=xuzhiyuan,pass=12345,mail=},

			]
		}
/#include <sys/stat.h> ��
/#include <sys/types.h>

����ԭ�ͣ� ����int mkdir(const char *pathname, mode_t mode); ����
����˵���� ����mkdir()������mode��ʽ����һ���Բ���pathname������Ŀ¼��mode�����´���Ŀ¼��Ȩ�ޡ� ����
����ֵ�� ������Ŀ¼�����ɹ����򷵻�0�����򷵻�-1�����������¼��ȫ�ֱ���errno�С�


mode��ʽ�� ����
S_IRWXU 00700Ȩ�ޣ�������ļ�������ӵ�ж���д��ִ�в�����Ȩ��
S_IRUSR(S_IREAD) 00400Ȩ�ޣ�������ļ�������ӵ�пɶ���Ȩ��
S_IWUSR(S_IWRITE) 00200Ȩ�ޣ�������ļ�������ӵ�п�д��Ȩ��
S_IXUSR(S_IEXEC) 00100Ȩ�ޣ�������ļ�������ӵ��ִ�е�Ȩ��
S_IRWXG 00070Ȩ�ޣ�������ļ��û���ӵ�ж���д��ִ�в�����Ȩ��
S_IRGRP 00040Ȩ�ޣ�������ļ��û���ӵ�пɶ���Ȩ��
S_IWGRP 00020Ȩ�ޣ�������ļ��û���ӵ�п�д��Ȩ��
S_IXGRP 00010Ȩ�ޣ�������ļ��û���ӵ��ִ�е�Ȩ��
S_IRWXO 00007Ȩ�ޣ����������û�ӵ�ж���д��ִ�в�����Ȩ��
S_IROTH 00004Ȩ�ޣ����������û�ӵ�пɶ���Ȩ��
S_IWOTH 00002Ȩ�ޣ����������û�ӵ�п�д��Ȩ��
S_IXOTH 00001Ȩ�ޣ����������û�ӵ��ִ�е�Ȩ��


*/