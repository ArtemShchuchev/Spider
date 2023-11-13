#pragma once

#include <regex>
#include <boost/locale.hpp> // перед "SecondaryFunction.h"
#include "SecondaryFunction.h"

using WordMap = std::unordered_map<std::string, int>;
using LinkVect = std::vector<std::string>;

class WordSearch
{
private:
	static const std::regex
		space_reg,
		body_reg,
		url_reg,
		title_reg,
		token_reg,
		punct_reg;

public:
	WordSearch() = default;
	std::pair<WordMap, LinkVect> getWordMap(std::string str);
};
