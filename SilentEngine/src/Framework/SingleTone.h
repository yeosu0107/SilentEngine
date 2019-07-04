#pragma once
#include <memory>
#include <mutex>

using namespace std;

template<typename T>
class SingleTone
{
public:
	static T& Instance() {
		// call_once 함수를 통해 람다 함수를 1번만 호출
		call_once(m_OnceFlag, []() { 
			// Instance 객체 초기화 및 생성
			m_Instance.reset(new T); 
			});
		
		return *m_Instance.get();
	};

protected:
	SingleTone() = default;
	SingleTone<T>& operator=(const SingleTone<T>& other) = delete;
	
private:
	static unique_ptr<T> m_Instance;
	static once_flag m_OnceFlag;
};

template<typename T>
unique_ptr<T> SingleTone<T>::m_Instance;

template<typename T>
once_flag SingleTone<T>::m_OnceFlag;