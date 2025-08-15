/**
* @file http_response_builder.cpp
* @brief  http响应构造
* @author liushisheng
* @date 2025-08-15
*/

#include "http_response_builder.h"

std::string HttpResponseBuilder::build(const HttpResponse& res)
{
	std::ostringstream oss;

	oss << "HTTP/1.1" << toInt(res.status) << " " << HttpStatusReason(res.status) << "\r\n";

	for (const auto& kv : res.headers)
	{
		oss << kv.first << ": " << kv.second << "\r\n";
	}

	oss << "\r\n";

	oss << res.body;

	return oss.str();
};