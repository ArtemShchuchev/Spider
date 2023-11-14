#pragma once

#include <iostream>
#include <regex>
#include <boost/locale.hpp> // перед "SecondaryFunction.h"
#include "SecondaryFunction.h"

using WordMap = std::unordered_map<std::wstring, int>;
using LinkVect = std::vector<std::wstring>;

class WordSearch
{
private:
	static const std::wregex
		space_reg,
		body_reg,
		url_reg,
		title_reg,
		token_reg,
		punct_reg,
		number_reg;

public:
	WordSearch() = default;
	std::pair<WordMap, LinkVect> getWordMap(std::wstring str);
};
