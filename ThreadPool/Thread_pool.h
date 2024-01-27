#pragma once

#include <iostream>
#include <future>
#include <vector>
#include <thread>
#include "Safe_queue.hpp"

using task_t = std::function<void()>;

class Thread_pool
{
private:
	enum class thread_mode { free, busy };
	struct Status {
		thread_mode mode;
		std::chrono::steady_clock::time_point start;
	};
	std::vector<std::thread> pool;	// пул потоков
	std::vector<Status> status;		// состояние потоков 
	Safe_queue<task_t> squeue;		// безопасная очередь задач

	// выбирает из очереди очередную задачу и выполняет ее
	// данный метод передается конструктору потоков для исполнения
	void work(const unsigned idx);
	// возвращает true если в очереди или хоть в одном работающем потоке есть задачи
	// принимает время для ожидания зависшего потока в секундах
	bool isBusy(const std::chrono::seconds& sec);

public:
	Thread_pool(const unsigned numThr);
	~Thread_pool();
	// помещает в очередь очередную задачу, метод принимает
	// объект шаблона std::function или 
	// объект шаблона std::packaged_task
	void add(const task_t& task);
	// ждет пока все потоки освободятся
	void wait(const std::chrono::seconds sec = std::chrono::seconds(2));
};