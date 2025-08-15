/**
* @file http_response_builder.h
* @brief  http响应构造
* @author liushisheng
* @date 2025-08-15
*/

#ifndef HTTP_RESPONSE_BUILDER_H
#define HTTP_RESPONSE_BUILDER_H

#include <unordered_map>
#include <string>
#include <sstream>

enum class HttpStatus
{
	OK					= 200,
	Created				= 201,
	NoContent			= 204,

	BadRequest			= 400,
	Unauthorized		= 401,
	Forbidden			= 403,
	NotFound			= 404,

	InternalServerError = 500,
	NotImplemented		= 501,
	BadGateway			= 502,
	ServiceUnavailable	= 503
};

inline int toInt(HttpStatus status)
{
	return static_cast<int>(status);
}

inline std::string HttpStatusReason(HttpStatus status) 
{
	switch (status) {
	case HttpStatus::OK: return "OK";
	case HttpStatus::Created: return "Created";
	case HttpStatus::NoContent: return "No Content";
	case HttpStatus::BadRequest: return "Bad Request";
	case HttpStatus::Unauthorized: return "Unauthorized";
	case HttpStatus::Forbidden: return "Forbidden";
	case HttpStatus::NotFound: return "Not Found";
	case HttpStatus::InternalServerError: return "Internal Server Error";
	case HttpStatus::NotImplemented: return "Not Implemented";
	case HttpStatus::BadGateway: return "Bad Gateway";
	case HttpStatus::ServiceUnavailable: return "Service Unavailable";
	default: return "Unknown";
	}
}

struct HttpResponse
{
	HttpStatus status = HttpStatus::OK;
	std::unordered_map<std::string, std::string> headers;
	std::string body;

	inline void setHeader(const std::string& key, const std::string& value)
	{
		headers[key] = value;
	}

	inline void setBody(const std::string& b, const std::string& contentType = "text/plain")
	{
		body = b;
		headers["Content-Type"] = contentType;
		headers["Content-Length"] = std::to_string(body.size());
	}

	inline void setStatus(HttpStatus s) 
	{
		status = s;
	}
};

class HttpResponseBuilder
{
public:
	static std::string build(const HttpResponse& res);
};

#endif // !HTTP_RESPONSE_BUILDER_H
