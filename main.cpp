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
	
	//ConfigFile config("../config.ini");
	//WordSearch words;
	
	//auto [wordAmount, links](words.getWordMap(s));

    try
    {
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
