#include "ThreadPoolCache.h"
#include <functional>
#include <thread>
#include <iostream>

const int TASK_MAX_THREADHOLD = INT32_MAX;

const int THREAD_MAX_THRESHHOLD = 1024;

const int THREAD_MAX_IDLE_TIME = 5; //单位：秒

ThreadPoolCache::ThreadPoolCache()
	:initThreadSize_(4)
	, threadSizeThreshHold_(THREAD_MAX_THRESHHOLD)
	, curThreadSize_(0)
	, idleThreadSize_(0)
	, taskSize_(0)
	, taskQueMaxThreadHold_(TASK_MAX_THREADHOLD)
	, poolMode_(PoolMode::MODE_FIXED)
	, isPoolRuning_(false)
{ }

ThreadPoolCache::~ThreadPoolCache()
{
	isPoolRuning_ = false;
	/*
	notify在前有三种情况，
	1、在等待notEmpty_.wait(),可以正常回收
	2、正在执行任务，执行完发现isPoolRuning_ = false，也可以正常结束
	3、任务线程isPoolRuning_ = true进入循环，在获取taskQueMtx_前，~ThreadPoolCache执行notEmpty_.notify_all()并获取taskQueMtx_成功，
		最后~ThreadPoolCache()::exitCond_.wait，任务线程notEmpty_.wait(lock)，造成死锁
	解决方法：锁+双重判断
	*/
	//notEmpty_.notify_all();	

	//等待线程池里面所有的线程返回	有两种状态：阻塞 & 正在执行任务
	std::unique_lock<std::mutex> lock(taskQueMtx_);
	notEmpty_.notify_all();
	exitCond_.wait(lock, [&]()->bool {
		return threads_.size() == 0;
		});
}

//设置线程池工作模式
void ThreadPoolCache::setMode(PoolMode mode)
{
	if (checkRunningState())
	{
		return;
	}
	poolMode_ = mode;
}

//设置线程池cache模式下线程阈值
void ThreadPoolCache::setThreadMaxThreshHold(int threshhold)
{
	if (checkRunningState() || poolMode_ != PoolMode::MODE_CACHE)
	{
		return;
	}
	threadSizeThreshHold_ = threshhold;
}

//设置task任务队列上限阈值
void ThreadPoolCache::setTaskQueMaxThreadHold(int threadhold)
{
	if (checkRunningState())
	{
		return;
	}
	taskQueMaxThreadHold_ = threadhold;
}

//开启线程池
void ThreadPoolCache::start(int initThreadSize)
{
	isPoolRuning_ = true;
	//记录初始线程个数
	initThreadSize_ = initThreadSize;
	curThreadSize_ = initThreadSize;

	//创建线程对象
	for (int i = 0; i < initThreadSize; i++)
	{
		//创建thread线程对象的时候，把线程函数给到thread线程对象
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPoolCache::threadFunc, this, std::placeholders::_1));
		//threads_.emplace_back(std::move(ptr));
		int threadId = ptr->getId();

		threads_.emplace(threadId, std::move(ptr));
	}

	//启动所有线程
	for (int i = 0; i < initThreadSize; i++)
	{
		threads_[i]->start();
		idleThreadSize_++;	//记录初始空闲线程的数量
	}
}

//给线程池提交任务 用户调用该接口，传入任务对象，生产任务
Result ThreadPoolCache::submitTask(std::shared_ptr<Task> sp)
{
	//获取锁
	std::unique_lock<std::mutex> lock(taskQueMtx_);

	//线程的通信	等待任务队列有空余
	//while (taskSize_ >= taskQueMaxThreadHold_)
	//{
	//	notFull_.wait(lock);
	//}
	if (!notFull_.wait_for(lock, std::chrono::seconds(1),
		[&]()->bool { return taskQue_.size() < (size_t)taskQueMaxThreadHold_; }))
	{
		//等待1s之后条件还是没有满足
		std::cerr << "task queue is full, submit task fail." << std::endl;
		return Result(sp, false);
	}

	//如果有空余，把任务放入任务队列中
	taskQue_.emplace(sp);
	taskSize_++;

	//新放任务，任务队列肯定不空，notEmpty_上通知
	notEmpty_.notify_all();

	//cache模式 场景：小而快的任务，任务处理比较紧急，不适合任务比较多且比较耗时的情况
	//需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
	if (poolMode_ == PoolMode::MODE_CACHE
		&& taskSize_ > idleThreadSize_
		&& curThreadSize_ < threadSizeThreshHold_)
	{
		std::cout << "create new thread " << std::endl;
		//创建新线程
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPoolCache::threadFunc, this, std::placeholders::_1));
		int threadId = ptr->getId();
		threads_.emplace(threadId, std::move(ptr));
		threads_[threadId]->start();
		curThreadSize_++;
		idleThreadSize_++;
	}

	//返回任务的Result对象
	return Result(sp);
}

//定义线程函数  线程池的所有线程从任务队列里面消费任务
void ThreadPoolCache::threadFunc(int threadId)	//线程函数返回，相应的线程也就结束了
{
	auto lastTime = std::chrono::high_resolution_clock().now();

	//所有任务全部执行完成，线程池才可以回收线程资源
	while (1)
	{
		std::shared_ptr<Task> task;
		{
			//先获取锁
			std::unique_lock<std::mutex> lock(taskQueMtx_);

			std::cout << "tid:" << std::this_thread::get_id() << "尝试获取任务。。。" << std::endl;

			//每秒钟返回一次	怎么区分超时返回，还是有任务返回？
			while (taskQue_.size() == 0)
			{
				//线程池结束，回收线程资源
				if (!isPoolRuning_)
				{
					threads_.erase(threadId);
					std::cout << "threadId:" << std::this_thread::get_id() << "exit!" << std::endl;
					exitCond_.notify_all();
					return;
				}
				//cache模式下，有可能已经创建了很多线程，但是空闲时间超过60s，应该把多余的线程结束回收
				//回收目标是超过initThreadSize_数量的线程
				//当前时间 - 上传线程执行的时间 > 60s
				if (poolMode_ == PoolMode::MODE_CACHE)
				{
					//条件变量超时返回
					if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1)))
					{
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
						if (dur.count() >= THREAD_MAX_IDLE_TIME && curThreadSize_ > initThreadSize_)
						{
							//开始回收当前线程
							//记录线程数量相关变量的值修改
							//把线程对象从线程列表容器中删除	没有办法threadFunc 对应 哪个thread对象
							threads_.erase(threadId);
							curThreadSize_--;
							idleThreadSize_--;

							std::cout << "threadId:" << std::this_thread::get_id() << "exit!" << std::endl;
							return;
						}
					}
				}
				else
				{
					//等待notEmpty_通知
					notEmpty_.wait(lock);
				}

				/*if (!isPoolRuning_)
				{
					threads_.erase(threadId);
					std::cout << "threadId:" << std::this_thread::get_id() << "exit!" << std::endl;
					exitCond_.notify_all();
					return;
				}*/
			}

			idleThreadSize_--;

			std::cout << "tid:" << std::this_thread::get_id() << "获取任务成功!" << std::endl;

			//从任务队列中取出一个任务出来
			task = taskQue_.front();
			taskQue_.pop();
			taskSize_--;

			//如果依然有剩余任务，继续通知其他的线程执行任务
			if (taskQue_.size() > 0)
			{
				notEmpty_.notify_all();
			}

			//取出任务，进行通知,可以继续提交任务生产
			notFull_.notify_all();
		}//释放锁，让其他线程可以对任务队列操作

		//当前线程负责执行这个任务
		if (task != nullptr)
		{
			task->exec();
		}
		idleThreadSize_++;
		lastTime = std::chrono::high_resolution_clock().now();
	}
	//threads_.erase(threadId);
	//std::cout << "threadId:" << std::this_thread::get_id() << "exit!" << std::endl;
	//exitCond_.notify_all();
	//return;
}

//线程方法实现
int Thread::generateId_ = 0;

int Thread::getId()const
{
	return threadId_;
}

Thread::Thread(ThreadFunc func)
	:func_(func)
	, threadId_(generateId_++)
{ }

Thread::~Thread()
{ }

// 启动线程
void Thread::start()
{
	//创建一个线程来执行一个线程函数
	std::thread t(func_, threadId_); //C++
	t.detach(); //设置分离线程 Linux::pthread_detach pthread_t设置成分离线程
}

///task方法实现
Task::Task()
	:result_(nullptr)
{}

void Task::exec()
{
	if (result_ != nullptr)
	{
		result_->setVal(run());
	}
}

void Task::setResult(Result* res)
{
	result_ = res;
}

//Result方法的实现
Result::Result(std::shared_ptr<Task> task, bool isValid)
	:isValid_(isValid)
	, task_(task)
{
	task_->setResult(this);
}

//setVal方法，获取任务执行完的返回值
void Result::setVal(Any any)
{
	//存储task的返回值
	this->any_ = std::move(any);
	sem_.post(); //已经获取任务的返回值，增加信号量资源
}

//get方法，用户调用这个方法获取task的返回值
Any Result::get()
{
	if (!isValid_)
	{
		return "";
	}

	sem_.wait();	//task任务如果没有执行完，这里会阻塞用户的线程
	return std::move(any_);
}

