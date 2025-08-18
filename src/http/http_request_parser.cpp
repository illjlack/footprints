/**
* @file http_request_parser.cpp
* @brief http请求解析
* @author liushisheng
* @date 2025-08-15
*/

#include "http_request_parser.h"
#include "comm/log.h"
#include <format>
#include <algorithm>
#include <iostream>

std::string urlDecode(const std::string& value)
{
    std::string decoded;
    for (size_t i = 0; i < value.length(); ++i)
    {
        if (value[i] == '+')
        {
            decoded += ' ';
        }
        else if (value[i] == '%' && i + 2 < value.length())
        {
            int hex = 0;
            std::istringstream(value.substr(i + 1, 2)) >> std::hex >> hex;
            decoded += static_cast<char>(hex);
            i += 2;
        }
        else
        {
            decoded += value[i];
        }
    }

    return decoded;
}

std::string HttpRequestParser::trim(const std::string& s)
{
	size_t start = s.find_first_not_of(" \r\n\t");
	size_t end = s.find_last_not_of(" \r\n\t");

	if (start == std::string::npos || end == std::string::npos)
	{
		return "";
	}

	return s.substr(start, end - start + 1);
}

std::unordered_map<std::string, std::string> HttpRequestParser::parseQueryString(const std::string& query)
{
	std::unordered_map<std::string, std::string> params;
	std::istringstream iss(query);

	std::string item;
	while (std::getline(iss, item, '&'))
	{
		size_t pos = item.find('=');
		if (pos != std::string::npos)
		{
			params[item.substr(0, pos)] = item.substr(pos + 1);
		}
		else
		{
			params[item] = "";
		}
	}
	return params;
}

std::unordered_map<std::string, std::string> HttpRequestParser::parseCookies(const std::string& cookie_header)
{
	std::unordered_map<std::string, std::string> cookies;
	std::istringstream iss(cookie_header);

	std::string item;

	while (std::getline(iss, item, ';'))
	{
		size_t pos = item.find('=');
		if (pos != std::string::npos)
		{
			cookies[trim(item.substr(0, pos))] = trim(item.substr(pos + 1));
		}
	}
	return cookies;
}

HttpRequest HttpRequestParser::parse(const std::string& rawRequest)
{
	HttpRequest req;
	std::istringstream iss(rawRequest);

	std::string line;
	if (!std::getline(iss, line))
	{
		LOG_WARN("Failed to read request line");
		return req;
	}
	line = trim(line);
	std::istringstream line_iss(line);
	line_iss >> req.method >> req.path >> req.version;
	req.path = urlDecode(req.path);
	LOG_DEBUG(std::format("Parsing HTTP request: {} {} {}", req.method, req.path, req.version));

	size_t qpos = req.path.find('?');
	if (qpos != std::string::npos)
	{
		req.query_params = parseQueryString(req.path.substr(qpos + 1));
		req.path = req.path.substr(0, qpos);
	}

	while (std::getline(iss, line) && !trim(line).empty())
	{
		line = trim(line);
		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			std::string key = trim(line.substr(0, pos));
			std::string value = trim(line.substr(pos + 1));
			req.headers[key] = value;
		}
	}

	auto it = req.headers.find("Cookie");
	if (it != req.headers.end())
	{
		req.cookies = parseCookies(it->second);
	}

	it = req.headers.find("Content-Length");
	if (it != req.headers.end())
	{
		int content_length = std::stoi(it->second);
		if (content_length > 0)
		{
			std::string body(content_length, '\0');
			iss.read(&body[0], content_length);
			req.body = body;
		}
	}

	return req;
}
