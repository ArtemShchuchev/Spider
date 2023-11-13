#include "wordSearch.h"

const std::regex
WordSearch::space_reg{ R"(\s+)" },
WordSearch::body_reg{ R"(< ?body ?>(.+)< ?/ ?body>)" },
WordSearch::url_reg{ R"!!(<\s*A\s+[^>]*href\s*=\s*"([^"]*)")!!", std::regex::icase },
WordSearch::title_reg{ R"(< ?title ?>(.+)< ?/ ?title>)" },
WordSearch::token_reg{ R"(<[^>]*>)" },
WordSearch::punct_reg{ R"([[:punct:]])" };

std::pair<WordMap, LinkVect> WordSearch::getWordMap(std::string str)
{
    // Убрал пробельные символы [ \f\n\r\t\v]
    str = std::regex_replace(str, space_reg, " ");
    // Нашел title
    auto it = std::sregex_token_iterator(str.begin(), str.end(), title_reg, 1);
    std::string title(*it);
    // Нашел body
    it = std::sregex_token_iterator(str.begin(), str.end(), body_reg, 1);
    str = *it;
    // Ищу ссылки
    std::vector<std::string> links{
        std::sregex_token_iterator{str.begin(), str.end(), url_reg, 1},
        std::sregex_token_iterator{}
    };
    // добавил к body, title
    str += title;
    // Убрал токены
    str = std::regex_replace(str, token_reg, "");
    // Убрал знаки пунктуации
    str = std::regex_replace(str, punct_reg, "  ");
    // Строку в нижний регистр
    // Create system default locale
    boost::locale::generator gen;
    std::locale loc = gen("");
    // Make it system global
    std::locale::global(loc);
    std::wstring wstr = boost::locale::to_lower(ansi2wideUtf(str));
    str = wideUtf2ansi(wstr);

    // Разделяю на слова от 3х до 32х символов, добавляю в словарь
    std::unordered_map<std::string, int> wordmap;
    std::stringstream stream(str);
    std::string word;
    while (std::getline(stream, word, ' ')) {
        if (word.size() > 2 && word.size() < 33)
            ++wordmap[word];
    }

    return { wordmap, links };
}
