﻿#include "HtmlClient.h"

std::wstring HtmlClient::do_request(std::string_view urlStr)
{
    // Парсинг строки ссылки
    auto parseResult = boost::urls::parse_uri(urlStr);
    auto& url = parseResult.value();
    //std::wcout << L"URL parse... " << std::endl;
    //std::wcout << L"protocol: " << utf82wideUtf(url.scheme()) << "\n";
    //std::wcout << L"domain:   " << utf82wideUtf(url.host()) << "\n";
    //std::wcout << L"path:     " << utf82wideUtf(url.path()) << "\n";

    // Список конечных точек
    // host: en.wikipedia.org, scheme: https
    tcp::resolver resolver{ ioc };
    tcp::resolver::query query(url.host(), url.scheme());
    tcp::resolver::results_type sequenceEp = resolver.resolve(query);
    int port = sequenceEp->endpoint().port();
    //std::wcout << L"port:     " << port << '\n';
    //std::wcout << L"----------------" << "\n";

    // Настройка запроса HTTP GET
    http::request<http::string_body> request;
    request.method(http::verb::get);
    request.target(url.path()); // path: /wiki/Main_Page
    request.version(11);
    request.keep_alive(true);
    request.set(http::field::host, url.host()); // host: en.wikipedia.org
    request.set(http::field::user_agent, "Spider");
    
    //std::stringstream ss;
    //ss << request;
    //std::wcout << L"Request: " << utf82wideUtf(ss.str()) << "\n";

    std::wstring str = (port == 443) ?
        httpsRequest(sequenceEp, request) :
        httpRequest(sequenceEp, request);

    return std::move(str);
}

std::wstring HtmlClient::httpsRequest(const tcp::resolver::results_type& sequenceEp,
    const http::request<http::string_body>& req)
{
    ssl::context ctx(ssl::context::sslv23);
    ctx.set_default_verify_paths();
    ctx.set_options(ssl::context::default_workarounds | ssl::context::verify_none);

    ssl::stream<tcp::socket> sslSocket(ioc, ctx);
    sslSocket.set_verify_mode(ssl::context::verify_none);
    //socket.set_verify_callback([](bool, ssl::verify_context&) {return true; });

    boost::asio::connect(sslSocket.lowest_layer(), sequenceEp);
    //std::wcout << L">>> Подключаюсь к " << sslSocket.lowest_layer().remote_endpoint() << L" <<<\n";

    sslSocket.handshake(ssl::stream<tcp::socket>::client);
    sslSocket.lowest_layer().set_option(tcp::no_delay(true));

    int bytes_sent = http::write(sslSocket, req);
    //std::wcout << bytes_sent << L" байт отправлено\n";

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    auto bytes_received = http::read(sslSocket, buffer, res);
    //std::wcout << bytes_received << L" байт получено\n";
    
    // Аккуратно закройте сокет
    beast::error_code ec;
    sslSocket.shutdown(ec);
    sslSocket.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
    sslSocket.lowest_layer().close();

    //std::wcout << ansi2wideUtf(ec.message()) << std::endl;

    // not_connected иногда случается, так что не беспокойтесь об этом
    if (ec && ec != beast::errc::not_connected)
        throw beast::system_error{ ec };

    return checkResult(res);
}

std::wstring HtmlClient::httpRequest(const tcp::resolver::results_type& sequenceEp,
    const http::request<http::string_body>& req)
{
    beast::tcp_stream stream{ ioc };
    // Установите соединение по IP-адресу
    stream.connect(sequenceEp);
    //std::wcout << L">>> Подключаюсь к " << stream.socket().lowest_layer().remote_endpoint() << L" <<<\n";

    // Отправьте HTTP-запрос на удаленный хост
    int bytes_sent = http::write(stream, req);
    //std::wcout << bytes_sent << L" байт отправлено\n";
    // Этот буфер используется для чтения и должен быть сохранен
    beast::flat_buffer buffer;
    // Объявите контейнер для хранения ответа
    http::response<http::dynamic_body> res;
    // Получите HTTP-ответ
    int bytes_received = http::read(stream, buffer, res);
    //std::wcout << bytes_received << L" байт получено\n";

    // Аккуратно закройте сокет
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    stream.socket().close();
    
    //std::wcout << ansi2wideUtf(ec.message()) << std::endl;

    // not_connected иногда случается, так что не беспокойтесь об этом
    if (ec && ec != beast::errc::not_connected)
        throw beast::system_error{ ec };

    return checkResult(res);
}

std::wstring HtmlClient::checkResult(http::response<http::dynamic_body> res)
{
    std::wstring ws;
    switch (res.base().result_int())
    {
        case 301:
            //std::wcout << L"Перенаправлено.....\n";
            ws = do_request(res.base()["Location"]);
            break;
        case 200:
        {
            std::stringstream ss;
            ss << res;
            std::string s(ss.str());
            ws = { s.begin(), s.end() };
            break;
        }
        default:
            //std::wcout << L"Unexpected HTTP status " << res.result_int() << "\n";
            break;
    }

    return std::move(ws);
}

std::wstring HtmlClient::getRequest(std::string_view urlStr)
{
	return do_request(urlStr);
}
