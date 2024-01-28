﻿#include <iostream>
#include <pqxx/pqxx>
#include "HtmlClient.h"
#include "Clientdb.h"
#include "ConfigFile.h"
#include "wordSearch.h"
#include "Thread_pool.h"
#include "SecondaryFunction.h"
#include "Types.h"

static void spider(LinkList& links);
static void spiderTask(const Link& url, Thread_pool& tp);

int main(int argc, char** argv)
{
    setRuLocale();
    consoleClear();

    try
    {
	    ConfigFile config("../config.ini");
        std::string firstLink(config.getConfig<std::string>("Spider", "startWeb"));
        if (firstLink.empty()) {
            throw std::logic_error("Вызов не содержит ссылки!");
        }
        unsigned int recurse(config.getConfig<unsigned int>("Spider", "recurse"));
        LinkList links{ {std::move(firstLink), recurse} };

        int numThr(std::thread::hardware_concurrency());
        //const int numThr(7);
        //Thread_pool tp(numThr);

        spider(links);
    }
    catch (const std::exception& err)
    {
        consoleCol(col::br_red);
        std::wcerr << L"\nИсключение типа: " << typeid(err).name() << '\n';
        std::wcerr << ansi2wideUtf(err.what()) << '\n';
        consoleCol(col::cancel);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


static void spider(LinkList& links)
{
    while (links.empty() == false)
    {
        // Загрузка очередной странички
        Link url(links.front());
        links.pop_front();
        if (url.recLevel > 0) {
            //std::wcout << L"1. Поиск-чтение страницы...\n";
            std::wcout << L"   url: " << utf82wideUtf(url.link_str) << " (" << url.recLevel << ")\n";
            HtmlClient client;
            std::wstring page = client.getRequest(url.link_str); // url -> page

            // Поиск слов/ссылок на страничке
            if (page.empty() == false) {
                //std::wcout << L"2. Парсер слов и ссылок...\n";
                WordSearch words;
                auto [wordAmount, listLinks](words.getWordLink(std::move(page), url.recLevel)); // page, recurse -> word, amount, listLinks
                links.splice(links.end(), listLinks);

                /*
                uint32_t listNum(0);
                for (const auto& link : links) {
                    std::wcout << ++listNum << ") " << link.link_str << " (" << link.recLevel << ")\n";
                }

                listNum = 0;
                for (const auto& [word, amount] : wordAmount) {
                    std::wcout << ++listNum << ") " << word << " - " << amount << '\n';
                }
                */

                /*
                // Сохранение найденных слов/ссылок в БД
                if (wordAmount.empty() == false) {
                    //std::wcout << L"3. Сохраниние в БД...\n";
                    Clientdb db;
                    int idLink(db.addLink(url.link_str));
                    idWordAm_vec idWordAm(db.addWords(std::move(wordAmount)));
                    db.addLinkWords(idLink, idWordAm);
                }
                */
            }
        }
    }
}

static void spiderTask(const Link& url, Thread_pool& tp)
{
    // Загрузка очередной странички
    if (url.recLevel > 0) {
        //std::wcout << L"1. Поиск-чтение страницы...\n";
        //std::wcout << L"   url: " << utf82wideUtf(url.link_str) << " (" << url.recLevel << ")\n";
        HtmlClient client;
        std::wstring page = client.getRequest(url.link_str); // url -> page

        // Поиск слов/ссылок на страничке
        if (page.empty() == false) {
            //std::wcout << L"2. Парсер слов и ссылок...\n";
            WordSearch words;
            auto [wordAmount, links](words.getWordLink(std::move(page), url.recLevel)); // page, recurse -> word, amount, listLink

            for (const auto& link : links) {
                tp.add([&link, &tp] {spiderTask(link, tp); });
            }

            /*
            // Сохранение найденных слов/ссылок в БД
            if (wordAmount.empty() == false) {
                //std::wcout << L"3. Сохраниние в БД...\n";
                Clientdb db;
                int idLink(db.addLink(url.link_str));
                idWordAm_vec idWordAm(db.addWords(std::move(wordAmount)));
                db.addLinkWords(idLink, idWordAm);
            }
            */
        }
    }
}
