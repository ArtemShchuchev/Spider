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
bool Thread_pool::isBusy(const std::chrono::seconds& timeout)
{
	bool free_f(false);	// все потоки заняты?
	int count(0);
	for (auto& [mode, start] : status) {
		++count;
		if (mode == thread_mode::busy) {
			auto diff = std::chrono::steady_clock::now() - start;
			// поток работает и его время превысило таймаут
			if (diff < timeout) {
				//std::wcout << L"Поток занят!\n";
				return true;
			}
			else std::wcout << count << L" - поток висит!\n";
		}
		else free_f = true;					// хоть 1 поток свободен
	}
	// если все потоки висят или очередь пуста -> false
	bool flag = squeue.empty();
	std::wcout << std::boolalpha << L"Очередь пуста: " << flag << '\n';
	return (!flag && free_f);
}

// ждет пока все потоки освободятся или все потоки timeout
void Thread_pool::wait(const std::chrono::seconds timeout)
{
	while (isBusy(timeout)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	};
}
