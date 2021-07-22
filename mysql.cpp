#include "mysql.h"
#include <mysql/mysql.h>
#include <string>

namespace Mysql
{
	
	CMysql::CMysql()
	{
		m_host = "127.0.0.1";
		m_user = "root";
		m_pass = "123456";
		m_port = 3306;
		m_db = "toy";
	}
	CMysql::CMysql(std::string &host, std::string &user, std::string &pass, uint16_t port, std::string &db)
	{
		m_host = host;
		m_user = user;
		m_pass = pass;
		m_port = port;
		m_db = db;
	}

	CMysql::~CMysql()
	{
		mysql_close(&conn);

	}
	bool CMysql::mysqlinit()
	{

		mysql_init(&conn);
		//GlobeConfig::Mysql &mysql = globeconfig.mysql;
		if (0 == mysql_options(&conn, MYSQL_SET_CHARSET_NAME, "utf8"))
			printf("mysql opt charset utf8 ok");
		else
		{
			mysql_close(&conn);
			return false;
		}
		if (mysql_real_connect(&conn, m_host.c_str(), m_user.c_str(), m_pass.c_str(),m_db.c_str(), 0, NULL, NULL))
			return true;
		else
		{
			printf("error:%s\n", mysql_error(&conn));
			mysql_close(&conn);
		}
		return false;
	}
	/*
	std::string CMysql::mysqlquery(Json::Value &result, const char* sql)
	{
		MYSQL_RES *results;
		Json::Value node;
		Json::Value arraynode;
		std::string text = "<tr>";
		if (0 == mysql_query(&conn, sql))
		{
			results = mysql_store_result(&conn);
			int rows = mysql_num_rows(results);//ѐ˽
			//int fields = mysql_num_fields(results);//˽
			unsigned int fieldcount = mysql_num_fields(results);//ؖ¶θ
			MYSQL_FIELD *field = NULL;
			std::string fields[fieldcount];
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				field = mysql_fetch_field_direct(results, j);//¸󜪵Ŗµȡؖ¶σ
				fields[j] = field->name;
				text = text + "<td>" + field->name + "</td>";
			}
			text += "</tr>\n";

			for (unsigned int i = 0; i < rows; i++)
			{
				text += "<tr>";
				MYSQL_ROW row = NULL;
				row = mysql_fetch_row(results);//ָ
				for (unsigned int j = 0; j < fieldcount; j++)
				{
					node[fields[j]] = row[j];
					text = text + "<td>" + row[j] + "</td>";
					printf("%s: %s\t", fields[j].c_str(), row[j]);
				}
				arraynode[i] = node;
				text += "</tr>\n";
			}
			mysql_free_result(results);//ˍ·Žẻ¼¯
			result["data"] = arraynode;
		}

		mysql_close(&conn);

		return text;
	}*/

	int CMysql::mysqlquery(const char* sql, Json::Value &result)
	{
		MYSQL_RES *results;
		Json::Value node;
		Json::Value arraynode;
		if (0 == mysql_query(&conn, sql))
		{
			results = mysql_store_result(&conn);
			int rows = mysql_num_rows(results);//ѐ˽
			//int fields = mysql_num_fields(results);//˽
			unsigned int fieldcount = mysql_num_fields(results);//ؖ
			MYSQL_FIELD *field = NULL;
			std::string fields[fieldcount];
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				field = mysql_fetch_field_direct(results, j);//是给定结果集内某1列的字段编号fieldnr，以MYSQL_FIELD结构形式返回列的字段定义。可以使用该函数检索任意列的定义。Fieldnr的值应在从0到mysql_num_fields(result)-1的范围内。对于指定列，返回MYSQL_FIELD结构。没有错误。
				fields[j] = field->name;
			}
			for (unsigned int i = 0; i < rows; i++)
			{
				MYSQL_ROW row = NULL;
				row = mysql_fetch_row(results);//ָ
				for (unsigned int j = 0; j < fieldcount; j++)
				{
					node[fields[j]] = row[j];

				}
				arraynode[i] = node;

			}
			mysql_free_result(results);//
			result = arraynode;
			mysql_close(&conn);

			return 1;
		}
		return -1;
	}

	int CMysql::mysqlqueryb(const char* sql, Json::Value &result)
	{
		MYSQL_RES *results;
		Json::Value node;
		result = node;
		Json::Value arraynode;
		if (0 == mysql_query(&conn, sql))
		{
			results = mysql_store_result(&conn);
			int rows = mysql_num_rows(results);//ѐ˽
			//int fields = mysql_num_fields(results);//˽
			unsigned int fieldcount = mysql_num_fields(results);//ؖ
			MYSQL_FIELD *field = NULL;
			std::string fields[fieldcount];
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				field = mysql_fetch_field_direct(results, j);//是给定结果集内某1列的字段编号fieldnr，以MYSQL_FIELD结构形式返回列的字段定义。可以使用该函数检索任意列的定义。Fieldnr的值应在从0到mysql_num_fields(result)-1的范围内。对于指定列，返回MYSQL_FIELD结构。没有错误。
				fields[j] = field->name;
			}
			for (unsigned int i = 0; i < rows; i++)
			{
				MYSQL_ROW row = NULL;
				row = mysql_fetch_row(results);//ָ
				for (unsigned int j = 0; j < fieldcount; j++)
				{
					if(row[j] ==NULL)
						node[fields[j]] = Json::nullValue;
					else
						node[fields[j]] = row[j];
					printf("mysql->%d--%s: %s\n", i,fields[j].c_str(), row[j]);

				}
				result[i] = node;

			}
			mysql_free_result(results);//
			return 1;
		}
		return -1;
	}

	int CMysql::mysqlquery(const char* sql, std::string &text)
	{
		MYSQL_RES *results;
		text = "<tr>";
		if (0 == mysql_query(&conn, sql))
		{
			results = mysql_store_result(&conn);
			int rows = mysql_num_rows(results);//
			//int fields = mysql_num_fields(results);//
			unsigned int fieldcount = mysql_num_fields(results);//
			MYSQL_FIELD *field = NULL;
			std::string fields[fieldcount];
			for (unsigned int j = 0; j < fieldcount; j++)
			{
				field = mysql_fetch_field_direct(results, j);//是给定结果集内某1列的字段编号fieldnr，以MYSQL_FIELD结构形式返回列的字段定义。可以使用该函数检索任意列的定义。Fieldnr的值应在从0到mysql_num_fields(result)-1的范围内。对于指定列，返回MYSQL_FIELD结构。没有错误。
				fields[j] = field->name;
				text = text + "<td>" + field->name + "</td>";
			}
			text += "</tr>\n";
			for (unsigned int i = 0; i < rows; i++)
			{
				text += "<tr>";
				MYSQL_ROW row = NULL;
				row = mysql_fetch_row(results);//
				for (unsigned int j = 0; j < fieldcount; j++)
				{
					text = text + "<td>" + row[j] + "</td>";
				}
				text += "</tr>\n";
			}
			mysql_free_result(results);//ˍ·Žẻ¼¯
			mysql_close(&conn);
			return 1;
		}
		return -1;
	}

	int CMysql::mysqlinsert(const char *sql)
	{
		int ret = mysql_query(&conn, sql);
		if (ret != 0)
		{
			printf("error:%s\n", mysql_error(&conn));
			return 0;
		}
		my_ulonglong affected_row = mysql_affected_rows(&conn);

		printf("%d rows affected.\n", (int)affected_row);

		return (int)affected_row;
	}
}