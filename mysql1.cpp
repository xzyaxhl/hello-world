/*#include "mysql.h"
#include <mysql/mysql.h>
#include <string.h>

extern GlobeConfig globeconfig;

namespace Mysql
{
	
	CMysql::CMysql():m_result(NULL)
	{
		
		m_errmsg[0] = m_iFields = { 0 };
		mysql_init(&conn);
		setConnected(false);
	}
	CMysql::CMysql(GlobeConfig &globe): m_result(NULL)// : globeconfig(globe)
	{
		m_errmsg[0] = m_iFields = { 0 };
		m_host = globe.mysql.host;
		m_user = globe.mysql.user;
		m_pass = globe.mysql.pass;
		m_db = globe.mysql.db;
		m_port = globe.mysql.port;
		mysql_init(&conn);
		setConnected(false);
	}
	CMysql::CMysql(const CMysql& cmysql) : m_result(NULL)
	{
		m_errmsg[0] = m_iFields ={ 0 };
		m_host = cmysql.m_host;
		m_user = cmysql.m_user;
		m_pass = cmysql.m_pass;
		m_db = cmysql.m_db;
		m_port = cmysql.m_port;
		mysql_init(&conn);
		setConnected(false);
	}

	CMysql::~CMysql()
	{
		FreePreResult();
		mysql_close(&conn);	
		setConnected(false);
	}

	bool CMysql::isConnected()
	{
		return m_connstatu;
	}

	int CMysql::connect(std::string &host, std::string &user, std::string &pass, std::string &db, int port)
	{
		if (isConnected())
		{
			return 0;
		}
		if (0 == mysql_options(&conn, MYSQL_SET_CHARSET_NAME, "utf8"))
			printf("ok");
		if (mysql_real_connect(&conn, m_host.c_str(), m_user.c_str(), pass.c_str(), db.c_str(), m_pass, NULL, NULL))
		{
			printf("%s", mysql_error(&conn));
			return -1;
		}
		printf("[mysql] conn to %s [user:%s] succ!\r\n", m_host, m_user);
		//设置连接标志为 true
		setConnected(true);
		return 0;



	}

	void CMysql::closeconnect()
	{
		mysql_close(&conn);
		setConnected(false);
	}


	std::string CMysql::mysqlquery(Json::Value &result, const char* sql)
	{
		MYSQL_RES *results;
		Json::Value node;
		Json::Value arraynode;
		std::string text="<tr>";
		if (0 == mysql_query(&conn, sql))
		{
			m_result = mysql_store_result(&conn);
			int rows = mysql_num_rows(m_result);//行数
			//int fields = mysql_num_fields(results);//列数
			unsigned int fieldcount = mysql_num_fields(m_result);//字段个数
			MYSQL_FIELD *field = NULL;
			std::string fields[fieldcount];
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				field = mysql_fetch_field_direct(m_result, j);//根据j的值取字段名
				fields[j] = field->name;
				text =text+ "<td>" + field->name + "</td>";
			}
			text += "</tr>\n";
			for (unsigned int i = 0; i < rows; i++)
			{
				text += "<tr>";
				MYSQL_ROW row = NULL;
				row = mysql_fetch_row(m_result);//指定结果集获取下一行
				for (unsigned int j = 0; j < fieldcount; j++)
				{
					node[fields[j]] = row[j];
					text = text + "<td>" + row[j] + "</td>";
					printf("%s: %s\t", fields[j].c_str(), row[j]);
				}
				arraynode[i] = node;
				text += "</tr>\n";
			}
			mysql_free_result(m_result);//释放结果集
			result["data"] = arraynode;
		}
		mysql_close(&conn);		
		return text;
	}

	int CMysql::SelectQuery(const char* sql)
	{
		if (sql == NULL)
		{
			printf("%s", "sql==NULL");
			return -1;
		}
		if (!isConnected())
		{
			printf("%s", "mysql not connect!");
			return -2;
		}
		try
		{
			if (mysql_real_query(&conn, sql, strlen(sql)) != 0)
			{
				printf("%s", mysql_error(&conn));
				int n = reconnect();
				if (n != 0)
					return -3;
				if (mysql_real_query(&conn, sql, strlen(sql)) != 0)
					return -33;
			}
			FreePreResult();
			m_result = mysql_store_result(&conn);
			if (m_result == NULL)
			{
				printf("%s", mysql_error(&conn));
				return -4;
			}
		}
		catch (...)
		{
			printf("reconnect() is called ,select !!!***\r\n");
			reconnect();
			return -5;
		}
		m_iFields = mysql_num_fields(m_result);
		m_mapFieldNameIndex.clear();
		MYSQL_FIELD *fields = NULL;
		fields = mysql_fetch_fields(m_result);
		for (unsigned int i = 0; i < m_iFields; i++)
		{
			m_mapFieldNameIndex[fields[i].name] = i;
		}
		return 0;
	}

	int CMysql::ModifyQuery(const char *sql)
	{
		if (sql == NULL)
		{
			printf("%s", "sql==NULL");
			return -1;
		}
		if (!isConnected())
		{
			printf("%s", "mysql not connect!");
			return -2;
		}
		try
		{
			if (mysql_real_query(&conn, sql, strlen(sql)) != 0)
			{
				printf("%s", mysql_error(&conn));
				return -3;
			}
		}
		catch (...)
		{
			printf("ReConnect()  is called  ,modify!!!***\r\n");
			reconnect();
			return -5;
		}
		return 0;
	}

	char **CMysql::FetchRow()
	{
		if (m_result == NULL)
			return NULL;
		m_row = mysql_fetch_row(m_result);
		return m_row;
	}

	char *CMysql::getField(const char *filedname)
	{
		return GetField(m_mapFieldNameIndex[filedname]);
	}
	char *CMysql::GetField(unsigned int iFieldIndex)
	{
		if (iFieldIndex >= m_iFields)
			return NULL;
		return m_row[iFieldIndex];
	}
	void CMysql::FreePreResult()
	{
		if (m_result != NULL)
		{
			mysql_free_result(m_result);
			m_result = NULL;
		}
	}
	const char* CMysql::GetErr()
	{
		return m_errmsg;
	}
	bool CMysql::isConnected()
	{
		return m_connstatu;
	}
	void CMysql::setConnected(bool bTrueFalse)

	{
		m_connstatu = bTrueFalse;
	}

	int CMysql::reconnect()
	{
		closeconnect();
		if (mysql_real_connect(&conn, m_host.c_str(), m_user.c_str(), m_pass.c_str(), m_db.c_str(), m_port, NULL, NULL))
		{
			printf("%s", mysql_error(&conn));
			return -1;
		}
		setConnected(true);
		return 0;
	}
	void CMysql::setused()
	{
		m_buseidle = true;
	}
	void CMysql::setidle()
	{
		m_buseidle = false;
	}
	bool CMysql::isidle()
	{
		return !m_buseidle;
	}
}*/