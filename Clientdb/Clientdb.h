﻿#pragma once
#include <iostream>
#include <pqxx/pqxx>
#include <exception>
#include <memory>
#include <vector>
#include "SecondaryFunction.h"
#include "../Types.h"

struct IdWordAm {
	int id;
	std::string word;
	int amount;
};
using idWordAm_vec = std::vector<IdWordAm>;

class Clientdb
{
private:
	std::unique_ptr<pqxx::connection> connect;

	void createTable();
	bool is_open();
	void deleteLink(const int id);
	void deleteNotUseWord();
	int createLink(const std::string& link);
	void getIdWord(idWordAm_vec& idWordAm);

public:
	Clientdb();
	std::wstring dbname();
	int addLink(const std::string& link);
	int getIdLink(const std::string& link);
	idWordAm_vec addWords(const WordMap wordAmount);
	void addLinkWords(const int id, const idWordAm_vec& idWordAm);
};
