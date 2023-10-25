#ifndef SINGLETON_H_
#define SINGLETON_H_
#include <mutex>

static std::mutex m_sigMutex;
template<class T>
class Singleton
{
public:
	static T* getInstance()
	{
		std::unique_lock<std::mutex>  mlock(m_sigMutex);
		if (m_pInstance == nullptr)
		{
			m_pInstance = new T();
		}
		return m_pInstance;
	}

protected:
	Singleton() {}
	Singleton(const Singleton&) {}//阻止copy构造
	Singleton& operator=(const Singleton&) {}//阻止赋值

	virtual ~Singleton() {}

	static void Destroy()
	{
		if (m_pInstance)
		{
			delegate m_pInstance;
			m_pInstance = nullptr;
		}
	}
private:
	static T* m_pInstance;
};
template<class T> T* Singleton<T>::m_pInstance = nullptr;

#endif //SINGLETON_H_