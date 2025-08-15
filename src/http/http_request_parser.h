/**
* @file http_request_parser.h
* @brief  http请求解析
* @author liushisheng
* @date 2025-08-15
*/

#ifndef HTTP_REQUEST_PARSER_H
#define HTTP_REQUEST_PARSER_H

#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>

struct HttpRequest
{
	std::string method;
	std::string path;
	std::string version;
	std::unordered_map<std::string, std::string> headers;
	std::string body;
	std::unordered_map<std::string, std::string> query_params;
	std::unordered_map<std::string, std::string> cookies;
};

class HttpRequestParser
{
public:
	HttpRequestParser() = default;
	~HttpRequestParser() = default;

	static HttpRequest parse(const std::string& raw_request);

private:

	static std::string trim(const std::string& s);
	
	static std::unordered_map<std::string, std::string> parseQueryString(const std::string& query);

	static std::unordered_map<std::string, std::string> parseCookies(const std::string& cookie_header);
};

#endif // !HTTP_REQUEST_PARSER_H
