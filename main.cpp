#include <iostream>
#include <pqxx/pqxx>
#include "HtmlClient.h"
#include "Clientdb.h"
#include "ConfigFile.h"
#include "wordSearch.h"
#include "Thread_pool.h"
#include "SecondaryFunction.h"
#include "Types.h"

struct Lock {
    std::mutex console;
    std::mutex parse;
    std::mutex db;
};
static void spiderTask(const Link url, Lock& lock, Thread_pool& tp);

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

        Lock lock;
        int numThr(std::thread::hardware_concurrency());
        Thread_pool tp(numThr);

        Link url{ std::move(firstLink), recurse };
        spiderTask(url, lock, tp);

        std::this_thread::sleep_for(std::chrono::seconds(1));
        tp.wait(std::chrono::seconds(40));
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


static void spiderTask(const Link url, Lock& lock, Thread_pool& tp)
{
    // Загрузка очередной странички
    if (url.recLevel > 0) {
        lock.console.lock();
        std::wcout << L"   url: " << utf82wideUtf(url.link_str) << " (" << url.recLevel << ")\n";
        lock.console.unlock();
        HtmlClient client;
        std::wstring page = client.getRequest(url.link_str); // url -> page

        // Поиск слов/ссылок на страничке
        if (page.empty() == false) {
            std::pair<WordMap, LinkList> wordlinks;
            try
            {
                {
                    std::lock_guard<std::mutex> lg(lock.parse);
                    WordSearch words;
                    wordlinks = words.getWordLink(std::move(page), url.recLevel); // page, recurse -> word, amount, listLink
                }

                for (const auto& link : wordlinks.second) {
                    tp.add([link, &lock, &tp] {spiderTask(link, lock, tp); });
                }
            }
            catch (const std::exception& err)
            {
                std::lock_guard<std::mutex> lg(lock.console);
                consoleCol(col::br_red);
                std::wcerr << L"\nИсключение типа: " << typeid(err).name() << '\n';
                std::wcerr << L"Ссылка: " << url.link_str << '\n';
                std::wcerr << L"Ошибка: " << ansi2wideUtf(err.what()) << std::endl;
                consoleCol(col::cancel);
            }

            // Сохранение найденных слов/ссылок в БД
            if (wordlinks.first.empty() == false) {
                try
                {
                    std::lock_guard<std::mutex> lg(lock.db);
                    Clientdb db;
                    int idLink(db.addLink(url.link_str));
                    idWordAm_vec idWordAm(db.addWords(std::move(wordlinks.first)));
                    db.addLinkWords(idLink, idWordAm);
                }
                catch (const pqxx::broken_connection& err)
                {
                    std::string err_str(err.what());
                    throw std::runtime_error("Ошибка PostgreSQL: " + err_str);
                }
                catch (const std::exception& err)
                {
                    std::wstring werr(L"Ошибка PostgreSQL: " + utf82wideUtf(err.what()));
                    throw std::runtime_error(wideUtf2ansi(werr));
                }
            }
        }
    }
}
