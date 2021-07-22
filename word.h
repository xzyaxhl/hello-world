#pragma once
#include "task.h"
#include "mysqlpool.h"

//std::string URL = "http://119.45.184.103:5888/";

class Word :public Task
{
public:
	Word();
	~Word();
	void start();
	bool run(std::string& s, string& text);
	bool Select(std::string&);
	bool SelectHtml(std::string&, std::vector<std::vector<std::string> >&);
	bool SelectAll(std::string&);
	bool SetFlag(std::string&);
	bool Flag(std::string&);
private:
	void SetReponse200(std::shared_ptr<Http::IHttpResponse> response, std::string& context);


private:
	CMysqlPool* m_imysqlpool;

	std::string m_query;
	std::map<std::string, std::string> m_parse_map;

	std::vector<std::vector<std::string> >m_sqlvec;//sql resut
	std::string m_listid="1";
	std::string m_wordid="1";
	std::string m_flag = "";
};

/*

<!DOCTYPE html>
<html>
<head>
	<title>Audio Player</title>
</head>
<body>

<img src="images_14_2.png"></br>
<button onclick="click01()">播放</button>
<button onclick="click02()">暂停</button>
<audio id="audio01" src="1.mp3"></audio>
<script>
	var c=document.getElementById("audio01");
	function click01(){
		if(c.paused){
			c.play();
		}
	}
	function click02(){
		if(c.play){
			c.pause();
		}
	}
</script>
</body>
</html>

*/
