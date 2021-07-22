#include "lcurl.h"
#include <assert.h>

using namespace std;

unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ')
			strTemp += "+";
		else
		{
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] % 16);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (str[i] == '+') strTemp += ' ';
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}


size_t CommonTools::receive_data(void* contents, size_t size, size_t nmemb, void* stream) {
	string* str = (string*)stream;
	(*str).append((char*)contents, size * nmemb);
	return size * nmemb;
}

size_t CommonTools::writedata2file(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);//size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
	return written;
}

int CommonTools::download_file(const char* url, const char outfilename[FILENAME_MAX]) {
	CURL* curl;
	FILE* fp;
	CURLcode res;
	/*   调用curl_global_init()初始化libcurl  */
	res = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != res)
	{
		printf("init libcurl failed.");
		curl_global_cleanup();
		return -1;
	}
	/*  调用curl_easy_init()函数得到 easy interface型指针  */
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(outfilename, "wb");

		/*  调用curl_easy_setopt()设置传输选项 */
		res = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}
		/*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CommonTools::writedata2file);
		if (res != CURLE_OK) {
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}
		/*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}

		res = curl_easy_perform(curl);
		// 调用curl_easy_perform()函数完成传输任务
		fclose(fp);
		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			return -1;
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
		// 调用curl_easy_cleanup()释放内存

	}
	curl_global_cleanup();
	return 0;
}

CURLcode CommonTools::HttpGet(const std::string& strUrl, std::string& strResponse, int nTimeout) {
	CURLcode res;
	CURL* pCURL = curl_easy_init();

	if (pCURL == NULL) {
		return CURLE_FAILED_INIT;
	}

	curl_easy_setopt(pCURL, CURLOPT_URL, strUrl.c_str());
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//在使用该选项且第 3 个参数为 1 时，curl 库会显示详细的操作信息。这对程序的调试具有极大的帮助。
	curl_easy_setopt(pCURL, CURLOPT_NOSIGNAL, 1L);//多线程安全  
	curl_easy_setopt(pCURL, CURLOPT_TIMEOUT, nTimeout);//超时设置
	curl_easy_setopt(pCURL, CURLOPT_NOPROGRESS, 1L); //如果你不会PHP为CURL传输显示一个进程条
	curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, CommonTools::receive_data);//传递所有的数据到receive_date中处理
	//size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp); 
	curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, (void*)&strResponse);//将数据传入strResponse  可以是个文件指针
	res = curl_easy_perform(pCURL);//执行
	curl_easy_cleanup(pCURL);//关闭
	return res;
}

CURLcode CommonTools::HttpPost(const std::string& strUrl, std::string szJson, std::string& strResponse, int nTimeout) {
	CURLcode res;
	char szJsonData[1024];
	memset(szJsonData, 0, sizeof(szJsonData));
	strcpy(szJsonData, szJson.c_str());
	CURL* pCURL = curl_easy_init();
	struct curl_slist* headers = NULL;
	if (pCURL == NULL) {
		return CURLE_FAILED_INIT;
	}

	CURLcode ret;
	ret = curl_easy_setopt(pCURL, CURLOPT_URL, strUrl.c_str());
	//    std::cout << ret << std::endl;

	ret = curl_easy_setopt(pCURL, CURLOPT_POST, 1L);//POST请求需要如此
	headers = curl_slist_append(headers, "content-type:application/json");

	ret = curl_easy_setopt(pCURL, CURLOPT_HTTPHEADER, headers);

	ret = curl_easy_setopt(pCURL, CURLOPT_POSTFIELDS, szJsonData);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	ret = curl_easy_setopt(pCURL, CURLOPT_TIMEOUT, nTimeout);

	ret = curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, CommonTools::receive_data);

	ret = curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, (void*)&strResponse);

	res = curl_easy_perform(pCURL);
	curl_easy_cleanup(pCURL);
	return res;
}
