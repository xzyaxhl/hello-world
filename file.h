#pragma once
#include "task.h"

#define ROOT "/home/xuzhiyuan/file/"

#define HTTP_VERSION			"HTTP/1.1"
#define HTTP_HEAD_CONTENT_TYPE	"Content-type"
#define HTTP_HEAD_CONTENT_LEN	"Content-length"
#define HTTP_HEAD_CONNECTION	"Connection"
#define HTTP_HEAD_KEEP_ALIVE	"Keep-Alive"
#define HTTP_ATTR_KEEP_ALIVE 	"keep-alive"
#define HTTP_HEAD_HTML_TYPE		"text/html"
#define HTTP_HEAD_TXT_TYPE		"text/json;charset=utf-8"
#define HTTP_HEAD_CSS_TYPE		"text/css"
#define HTTP_HEAD_GIF_TYPE		"image/gif"
#define HTTP_HEAD_JPG_TYPE		"image/jpeg"
#define HTTP_HEAD_PNG_TYPE		"image/png"
#define HTTP_HEAD_MP3_TYPE      "audio/mp3"

std::string  extention_name(const std::string& basename);
const char* http_content_type(const std::string& extension);

class SelectFile :
    public Task
{
public:
    SelectFile();
    ~SelectFile();
    void start();
    void readfile(std::string& filepath);

};

