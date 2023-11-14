#include <iostream>
#include <pqxx/pqxx>
#include "HtmlClient.h"
#include "Clientdb.h"
#include "ConfigFile.h"
#include "wordSearch.h"
#include "SecondaryFunction.h"

int main(int argc, char** argv)
{
    setRuLocale();
    consoleClear();

    try
    {
	    ConfigFile config("../config.ini");
        std::string firstLink = config.getConfig<std::string>("Spider", "startWeb");
        int recurse = config.getConfig<int>("Spider", "recurse");

        //std::wcout << utf82wideUtf(firstLink) << " - " << recurse << '\n';
        HtmlClient client;
        std::string htmlAnswer = client.getRequest(firstLink);
	
	    WordSearch words;
        auto [wordAmount, links](words.getWordMap(utf82wideUtf(htmlAnswer)));



        uint32_t listNum(0);
        for (const auto& link : links) {
            std::wcout << ++listNum << ") " << link << '\n';
        }
        std::wcout << '\n';

        listNum = 0;
        for (const auto& [word, amount] : wordAmount)
        {
            std::wcout << ++listNum << ") " << word << " - " << amount << '\n';
        }
    }
    catch (const std::exception& err)
    {
        consoleCol(col::br_red);
        std::wcerr << L"\nИсключение типа: " << typeid(err).name() << '\n';
        std::wcerr << utf82wideUtf(err.what()) << '\n';
        consoleCol(col::cancel);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
