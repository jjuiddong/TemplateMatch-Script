#pragma once


namespace common
{
	template <class T>
	class cSingleton
	{
	public:
		static T* m_pInstance;
		static T* Get()
		{
			if (!m_pInstance)
				m_pInstance = new T;
			return m_pInstance;
		}
		static void Release()
		{
			if (m_pInstance)
			{
				delete m_pInstance;
				m_pInstance = NULL;
			}
		}
	};
	template <class T>
	T* cSingleton<T>::m_pInstance = NULL;
}



class cAutoCS
{
public:
	cAutoCS(CRITICAL_SECTION &cs) :
		m_cs(cs)
		, m_isLeave(false)
	{
		EnterCriticalSection(&cs);
	}

	virtual ~cAutoCS()
	{
		if (!m_isLeave)
			LeaveCriticalSection(&m_cs);
		m_isLeave = true;
	}

	void Enter()
	{
		if (m_isLeave)
			EnterCriticalSection(&m_cs);
		m_isLeave = false;
	}

	void Leave()
	{
		LeaveCriticalSection(&m_cs);
		m_isLeave = true;
	}

	CRITICAL_SECTION &m_cs;
	bool m_isLeave;
};


namespace common
{
	/// Auto Lock, Unlock
	template<class T>
	class AutoLock
	{
	public:
		AutoLock(T& t) : m_t(t) { m_t.Lock(); }
		~AutoLock() { m_t.Unlock(); }
		T &m_t;
	};



	/// Critical Section auto initial and delete
	class CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();
		void Lock();
		void Unlock();
	protected:
		CRITICAL_SECTION m_cs;
	};

	inline CriticalSection::CriticalSection() {
		InitializeCriticalSection(&m_cs);
	}
	inline CriticalSection::~CriticalSection() {
		DeleteCriticalSection(&m_cs);
	}
	inline void CriticalSection::Lock() {
		EnterCriticalSection(&m_cs);
	}
	inline void CriticalSection::Unlock() {
		LeaveCriticalSection(&m_cs);
	}


	/// auto critical section lock, unlock
	class AutoCSLock : public AutoLock<CriticalSection>
	{
	public:
		AutoCSLock(CriticalSection &cs) : AutoLock(cs) { }
	};

}



namespace common
{
	void replaceAll(string& str, const string& from, const string& to);
	string& trim(string &str);
	void trimw(wstring &str);

	string& lowerCase(string &str);
	string& upperCase(string &str);
	wstring& lowerCasew(wstring &str);
	wstring& upperCasew(wstring &str);

	string wstr2str(const wstring &wstr);
	wstring str2wstr(const string &str);

	string format(const char* fmt, ...);
	wstring formatw(const char* fmt, ...);

	void tokenizer(const string &str, const string &delimeter, const string &ignoreStr, OUT vector<string> &out);
	void wtokenizer(const wstring &str, const wstring &delimeter, const wstring &ignoreStr, OUT vector<wstring> &out);

	void tokenizer2(const string &str, const string &delimeters, OUT vector<string> &out);
}





namespace common
{

	/**
	@brief this function only avaible to unique ty value
	remove ty and then rotate vector to sequence elements
	*/
	template <class Seq>
	bool removevector(Seq &seq, const typename Seq::value_type &ty)
	{
		for (size_t i = 0; i < seq.size(); ++i)
		{
			if (seq[i] == ty)
			{
				if ((seq.size() - 1) > i) // elements를 회전해서 제거한다.
					std::rotate(seq.begin() + i, seq.begin() + i + 1, seq.end());
				seq.pop_back();
				return true;
			}
		}
		return false;
	}


	// elements를 회전해서 제거한다.
	template <class Seq>
	void rotatepopvector(Seq &seq, const unsigned int idx)
	{
		if ((seq.size() - 1) > idx)
			std::rotate(seq.begin() + idx, seq.begin() + idx + 1, seq.end());
		seq.pop_back();
	}


	template <class Seq>
	void rotateright(Seq &seq)
	{
		if (seq.size() > 1)
			std::rotate(seq.rbegin(), seq.rbegin() + 1, seq.rend());
	}


	template <class T>
	void putvector(std::vector<T> &seq, size_t putIdx, const T &ty)
	{
		if (seq.size() > putIdx)
			seq[putIdx] = ty;
		else
			seq.push_back(ty);
	}

}



namespace common {
	namespace dbg
	{
		void Print(const std::string &str);
		void Print(const char* fmt, ...);

		void Log(const char* fmt, ...);
		void Log2(const char *fileName, const char* fmt, ...);
		void ErrLog(const char* fmt, ...);

		void RemoveLog();
		void RemoveLog2(const char *fileName);
		void RemoveErrLog();
	}
}


namespace common {

	bool CompareExtendName(const char *srcFileName, int srcStringMaxLength, const char *compareExtendName);
	bool CollectFiles(const list<string> &findExt, const string &searchPath, OUT list<string> &out);

}
