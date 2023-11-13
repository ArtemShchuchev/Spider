#pragma once
#include <iostream>
#include <pqxx/pqxx>
#include <exception>
#include <memory>
#include <vector>
#include "SecondaryFunction.h"

struct IdWordAm {
	int id;
	std::string word;
	int amount;
};

class Clientdb
{
private:
	std::unique_ptr<pqxx::connection> connect;

	void createTable();
	bool is_open();
	void deleteLink(const int id);
	void deleteNotUseWord();
	int createLink(const std::string& link);
	void getIdWord(std::vector<IdWordAm>& idWordAmount);

public:
	Clientdb();
	std::wstring dbname();
	int addLink(const std::string& link);
	int getIdLink(const std::string& link);
	void addWords(std::vector<IdWordAm>& idWordAmount);
	void addLinkWords(const int id, const std::vector<IdWordAm>& idWordAmount);
};
