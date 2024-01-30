#include <iostream>
#include <pqxx/pqxx>
#include "HtmlClient.h"
#include "Clientdb.h"
#include "ConfigFile.h"
#include "wordSearch.h"
#include "Thread_pool.h"
#include "SecondaryFunction.h"
#include "Types.h"

int countglob(0);
struct Lock {
    std::mutex console;
    std::mutex parse;
    std::mutex db;
};

Lock* lock;
Thread_pool* threadPool;

static void spiderTask(const Link url, Lock* lock, Thread_pool* threadPool);

static task_t maketask(Thread_pool& tp, int i)
{
    auto t = [&] { while (i>0) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); tp.add(maketask(tp, --i)); } };
    return t;
}

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
        Link url{ std::move(firstLink), recurse };

        //Lock lock;
        int numThr(std::thread::hardware_concurrency());
        
        //Thread_pool threadPool(numThr);
        lock = new Lock();
        threadPool = new Thread_pool(numThr);
        threadPool->setTimeout(std::chrono::seconds(5));
        spiderTask(url, lock, threadPool);
        //threadPool.add([url, &lock, &threadPool] { spiderTask(url, lock, threadPool); });
        
        //threadPool.add(maketask(threadPool, 5));
        //std::this_thread::sleep_for(std::chrono::seconds(10));
        threadPool->wait();
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


static void spiderTask(const Link url, Lock* lock, Thread_pool* threadPool)
{
    // Загрузка очередной странички
    if (url.recLevel > 0) {
        lock->console.lock();
        ++countglob;
        int count = countglob;
        //std::wcout << count << L"   url: " << utf82wideUtf(url.link_str) << " (" << url.recLevel << ")\n";
        std::wcout << count << "\n";
        lock->console.unlock();
        std::wstring page;
        {
            HtmlClient client;
            page = client.getRequest(url.link_str); // url -> page
        }


        lock->console.lock();
        //std::wcout << count << L" Получена страничка---------\n";
        std::wcout << "\t" << count << "\n";
        lock->console.unlock();



        // Поиск слов/ссылок на страничке
        if (page.empty() == false) {
            std::pair<WordMap, LinkList> wordlinks;
            try
            {
                {
                    std::lock_guard<std::mutex> lg(lock->parse);
                    WordSearch words;
                    wordlinks = words.getWordLink(std::move(page), url.recLevel); // page, recurse -> word, amount, listLink
                }


                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                lock->console.lock();
                std::wcout << "\t\t" << count << "\n";
                lock->console.unlock();



                for (const auto& link : wordlinks.second) {
                    threadPool->add([link, lock, threadPool] { spiderTask(link, lock, threadPool); });
                }
            }
            catch (const std::exception& err)
            {
                std::lock_guard<std::mutex> lg(lock->console);
                consoleCol(col::br_red);
                std::wcerr << L"\nИсключение типа: " << typeid(err).name() << '\n';
                std::wcerr << L"Ссылка: " << url.link_str << '\n';
                std::wcerr << L"Ошибка: " << ansi2wideUtf(err.what()) << std::endl;
                consoleCol(col::cancel);
            }

            lock->console.lock();
            std::wcout << "\t\t\t" << count << "\n";
            lock->console.unlock();
            

            // Сохранение найденных слов/ссылок в БД
            if (wordlinks.first.empty() == false) {
                try
                {
                    std::lock_guard<std::mutex> lg(lock->db);
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



            std::lock_guard<std::mutex> lg(lock->console);
            std::wcout << "\t\t\t\t" << count << "\n";
        }
    }
}
