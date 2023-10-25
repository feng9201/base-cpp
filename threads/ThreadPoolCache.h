#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <thread>

/* @brief: 线程池cache,类似QT的线程池使用，业务类继承Task，实现Run
*  class MyTask : public Task
{
public:
	MyTask(int begin, int end)
		:begin_(begin)
		, end_(end)
	{}
	Any run()
	{
		std::this_thread::sleep_for(std::chrono::seconds(4));
		//std::cout << "tid:" << std::this_thread::get_id() << "end!" << std::endl;
		int sum = 0;
		for (int i = begin_; i <= end_; i++)
		{
			sum += i;
		}
		return sum;
	}
private:
	int begin_;
	int end_;
};
* 
* 
	ThreadPoolCache pool;
	pool.setMode(PoolMode::MODE_CACHE);
	pool.start(4);
	Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100));
	unsigned long long s = res1.get().cast_<unsigned long long>();//获取返回结果，如线程未执行完，则等待
*/


class Any
{
public:
	Any() = default;
	~Any() = default;
	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;
	Any(Any&&) = default;
	Any& operator=(Any&&) = default;

	template<typename T>
	Any(T data) : base_(std::make_unique<Derive<T>>(data))
	{}

	//这个方法能把Any对象里面存储的data_数据提取出来
	template<typename T>
	T cast_()
	{
		//多定义Base和Derive类的目的是为了能识别获取类型是否安全，方便使用dynamic_cast判断抛异常
		//从base_找到它所指的Derive对象，从它里面取出data_成员变量
		Derive<T>* pd = dynamic_cast<Derive<T>*>(base_.get());
		if (pd == nullptr)
		{
			throw "type is unmatch!";
		}
		return pd->data_;
	}

private:
	//基类类型
	class Base
	{
	public:
		virtual ~Base() = default;
	};

	template<typename T>
	class Derive : public Base
	{
	public:
		Derive(T data) : data_(data)
		{}
		T data_;
	};

private:
	//定义一个积累的指针
	std::unique_ptr<Base> base_;
};

//信号量类
class Semaphore
{
public:
	Semaphore(int limit = 0) : resLimit_(limit)
	{}
	~Semaphore() = default;

	//获取一个信号量资源
	void wait()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		//等待信号量有资源，没有资源的话阻塞当前线程
		cond_.wait(lock, [&]()->bool { return resLimit_ > 0; });
		resLimit_--;
	}

	//增加一个信号量资源
	void post()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		resLimit_++;
		cond_.notify_all();
	}

private:
	int resLimit_;
	std::mutex mtx_;
	std::condition_variable cond_;
};

//任务抽象类前置声明
class Task;

//接收提交到线程池的task任务执行完成后的返回值类型Result
class Result
{
public:
	Result(std::shared_ptr<Task> task, bool isValid = true);
	~Result() = default;

	//setVal方法，获取任务执行完的返回值
	void setVal(Any any);

	//get方法，调用这个方法获取task的返回值
	Any get();
private:
	Any any_; //存储任务的返回值
	Semaphore sem_; //线程通信信号量
	std::shared_ptr<Task> task_; //指向对应获取返回值的任务对象
	std::atomic_bool isValid_;	//返回值是否有效
};

//任务抽象基类
class Task
{
public:
	Task();
	~Task() = default;
	//用户可以自定义任意任务类型，从Task继承，重写run方法，实现自定义任务处理
	virtual Any run() = 0;

	void exec();
	void setResult(Result* res);

private:
	Result* result_;
};

enum class PoolMode
{
	MODE_FIXED,		//固定数量的线程
	MODE_CACHE		//线程可动态增长
};

//线程类型
class Thread
{
public:
	//线程 函数对象 类型
	using ThreadFunc = std::function<void(int)>;

	Thread(ThreadFunc);
	~Thread();
	//启动线程
	void start();

	//获取线程ID
	int getId()const;

private:
	ThreadFunc func_;
	static int generateId_;
	int threadId_;	//保存线程id
};

class ThreadPoolCache
{
public:
	ThreadPoolCache();
	~ThreadPoolCache();

	ThreadPoolCache(const ThreadPoolCache&) = delete;
	ThreadPoolCache& operator=(const ThreadPoolCache&) = delete;

	//设置线程池工作模式
	void setMode(PoolMode mode);

	//设置task任务队列上限阈值
	void setTaskQueMaxThreadHold(int threshhold);
	//设置线程池cache模式下线程阈值
	void setThreadMaxThreshHold(int threshhold);

	//给线程池提交任务
	Result submitTask(std::shared_ptr<Task> sp);

	//开启线程池
	void start(int initThreadSize = std::thread::hardware_concurrency());

private:
	//定义线程函数
	void threadFunc(int threadId);

	//检查pool运行状态
	bool checkRunningState() const
	{
		return isPoolRuning_;
	}

private:
	std::unordered_map<int, std::unique_ptr<Thread>> threads_; //线程列表

	int initThreadSize_;	//初始的线程数量
	int threadSizeThreshHold_; //线程数量上限阈值
	std::atomic_int curThreadSize_; //当前线程池里线程的总数量
	std::atomic_int idleThreadSize_; //记录空闲线程的数量

	std::queue<std::shared_ptr<Task>> taskQue_; //任务队列
	std::atomic_int taskSize_; //任务的数量
	int taskQueMaxThreadHold_; //任务队列数量上限阈值

	std::mutex taskQueMtx_; //保证任务队列的线程安全
	std::condition_variable notFull_; //表示任务队列不满
	std::condition_variable notEmpty_; //表示任务队列不空
	std::condition_variable exitCond_; //等到线程资源全部回收

	PoolMode poolMode_; //当前线程池的工作模式
	std::atomic_bool isPoolRuning_;	//表示当前线程池的启动状态

};


