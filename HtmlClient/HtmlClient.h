#pragma once

#include <iostream>
#include <boost/beast.hpp>
#include <boost/url.hpp>
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/asio/ssl.hpp>
#include "SecondaryFunction.h"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = net::ssl;
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


class HtmlClient
{
private:
	// io_context требуется для всех операций ввода-вывода.
	net::io_context ioc;

	std::string do_request(std::string_view urlStr);
	std::string httpsRequest(const tcp::resolver::results_type& sequenceEp, const http::request<http::string_body>& req);
	std::string httpRequest(const tcp::resolver::results_type& sequenceEp, const http::request<http::string_body>& req);
	std::string checkResult(http::response<http::dynamic_body> res);

public:
	HtmlClient() = default;
	std::string getRequest(std::string_view urlStr);
};
