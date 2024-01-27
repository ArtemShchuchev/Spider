#include "Thread_pool.h"

Thread_pool::Thread_pool(const unsigned numThr)
{
	pool.resize(numThr);	// устанавливаю размер вектора
	status.assign(numThr, { thread_mode::free, {} });
	
	// выборка задач из очереди, происходит в потоках
	for (unsigned i(0); i < numThr; ++i) {
		pool[i] = std::thread([this, i] { work(i); });
		pool[i].detach();
	}
}

Thread_pool::~Thread_pool()
{
	wait();	// ожидание завершения работы потоками
}

// выбирает из очереди очередную задачу и выполняет ее
void Thread_pool::work(const unsigned idx)
{
	while (true)
	{
		const task_t task = squeue.pop();
		
		status[idx].start = std::chrono::steady_clock::now();
		status[idx].mode = thread_mode::busy;
		task();
		status[idx].mode = thread_mode::free;
	}
}

// помещает в очередь очередную задачу
void Thread_pool::add(const task_t& task)
{
	if (task)
		squeue.push(task);
}

// возвращает true если в очереди или
// хоть в одном работающем потоке есть задачи
bool Thread_pool::isBusy(const std::chrono::seconds& sec)
{
	bool free_f(false);	// все потоки заняты?
	for (auto& [mode, start] : status) {
		if (mode == thread_mode::busy) {
			auto diff = std::chrono::steady_clock::now() - start;
			if (diff < sec) return true;	// поток работает
		}
		else free_f = true;					// хоть 1 поток свободен
	}
	// если все потоки висят или очередь пуста -> false
	return (!squeue.empty() && free_f);
}

// ждет пока все потоки освободятся или все потоки timeout
void Thread_pool::wait(const std::chrono::seconds sec)
{
	while (isBusy(sec)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	};
}
