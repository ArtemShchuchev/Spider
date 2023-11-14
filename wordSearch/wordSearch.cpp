#include "wordSearch.h"

const std::wregex
WordSearch::space_reg{ LR"(\s+)" },
WordSearch::body_reg{ LR"(< ?body[^>]*>(.+)< ?/ ?body>)" },
WordSearch::url_reg{ LR"!!(<\s*A\s+[^>]*href\s*=\s*"(http[^"]*)")!!", std::regex::icase },
WordSearch::title_reg{ LR"(< ?title ?>(.+)< ?/ ?title>)" },
WordSearch::token_reg{ LR"(<[^>]*>)" },
WordSearch::punct_reg{ LR"([[:punct:]])" },
WordSearch::number_reg{ LR"( [0-9][^ ]*)" };

std::pair<WordMap, LinkVect> WordSearch::getWordMap(std::wstring str)
{
    // Убрал пробельные символы [ \f\n\r\t\v]
    str = std::regex_replace(str, space_reg, L" ");
    // Нашел title
    auto it = std::wsregex_token_iterator(str.begin(), str.end(), title_reg, 1);
    std::wstring title;
    if (it != std::wsregex_token_iterator{}) title = *it;
    // Нашел body
    it = std::wsregex_token_iterator(str.begin(), str.end(), body_reg, 1);
    if (it != std::wsregex_token_iterator{}) str = *it;
    else str.clear();
    // Ищу ссылки
    LinkVect links{
        std::wsregex_token_iterator{str.begin(), str.end(), url_reg, 1},
        std::wsregex_token_iterator{}
    };
    // добавил к body, title
    str += title;
    // Убрал токены
    str = std::regex_replace(str, token_reg, L"");
    // Убрал знаки пунктуации
    str = std::regex_replace(str, punct_reg, L" ");
    // Цифры и слова начинающиеся с цифры
    str = std::regex_replace(str, number_reg, L" ");
    // Строку в нижний регистр
    // Create system default locale
    boost::locale::generator gen;
    std::locale loc = gen("");
    // Make it system global
    std::locale::global(loc);
    str = boost::locale::to_lower(str);
    // Разделяю на слова от 3х до 32х символов, добавляю в словарь
    WordMap wordmap;
    std::wstringstream stream(str);
    std::wstring word;
    while (std::getline(stream, word, L' '))
    {
        if (word.size() > 2 && word.size() < 33)
            ++wordmap[word];
    }

    return { wordmap, links };
}
