#pragma once 
#include "Base/Memory.h"

namespace PH::Base {

	//returns the length of the string excluding the nulltermination character
	inline sizeptr stringLength(const char* str) {
		sizeptr strlen = 0;
		while (str[strlen] != 0) {
			strlen++;
		}
		return strlen;
	}

	inline bool32 stringCompare(const char* str1, const char* str2) {
		while (*str1 != 0 && *str2 != 0) {
			if (*str1 != *str2) {
				return false;
			}
			str1++;
			str2++;
		}
		return *str1 == *str2;
	}

	//copies string from src to dst and returns the amount of bytes copied including the nulltermination character
	inline sizeptr stringCopy(const char* src, char* dst, sizeptr maxsize) {

		sizeptr _maxsize = maxsize;

		//decrement the maxsize to include the null termination character
		maxsize--;

		while (*src != 0 && maxsize > 0) {
			*dst = *src;
			src++;
			dst++;
			maxsize--;
		}
		*dst = 0;

		return _maxsize - maxsize;
	}

	/// <summary>
	/// a non owning string class
	/// </summary>
	class SubString {
	private:
	public:
		SubString() {
			m_Str = nullptr;
			m_Length = 0;
		}

		static SubString create(const char* str) {
			SubString string;
			string.m_Length = stringLength(str);
			string.m_Str = (char*)str;
			return string;

		}

		static SubString create(const char* str, sizeptr size) {
			SubString string;
			string.m_Length = stringLength(str);
			string.m_Str = (char*)str;
			return string;
		}

		static SubString create(char* str, sizeptr size) {
			SubString string;
			string.m_Length = size;
			string.m_Str = str;
			return string;
		}

		static SubString create(char* str) {
			SubString string;
			string.m_Length = stringLength(str);
			string.m_Str = str;
			return string;
		}

		SubString getTill(char c) {

			char* str = m_Str;
			while (*str != 0) {
				if (*str == c) {
					break;
				}
				str++;
			}
			return SubString::create(m_Str, str - m_Str + 1);
		}

		SubString operator-(uint32 l) {
			SubString copy = *this;
			copy.m_Length -= l;
			return copy;
		}

		/// <summary>
		/// returns the substring from the last found char c, if no such character is found the return is the whole string
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		SubString getTillLast(char c) {

			char* str = m_Str;
			char* lastfound = m_Str + m_Length;

			while (*str != 0 && (str - m_Str) < (PH::int64)m_Length) {
				if (*str == c) {
					lastfound = str;
				}
				str++;
			}
			return SubString::create(m_Str, lastfound - m_Str + 1);
		}

		/// <summary>
		/// returns the substring from the last found char c, if no such character is found the return is the whole string
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		SubString getTillLastExcluding(char c) {

			char* str = m_Str;
			char* lastfound = m_Str + m_Length;

			while (*str != 0 && (str - m_Str) < m_Length) {
				if (*str == c) {
					lastfound = str;
				}
				str++;
			}
			return SubString::create(m_Str, lastfound - m_Str);
		}


		/// <summary>
		/// returns the substring from the last found char c, if no such character is found the return is an empty substring
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		SubString getFromLast(char c) {
			char* str = m_Str;
			char* lastfound = m_Str + m_Length;

			while (*str != 0 && (str - m_Str) < m_Length) {
				if (*str == c) {
					lastfound = str;
				}
				str++;
			}
			return SubString::create(lastfound, (m_Str + m_Length) - lastfound);
		}

		SubString moveHead(sizeptr length) {
			SubString result = *this;
			result.m_Str += length;
			result.m_Length -= length;
			return result;
		}

		SubString getFrom(char c) {

			char* str = m_Str;
			while (*str != 0 && (str - m_Str) < m_Length) {
				if (*str == c) {
					break;
				}
				str++;
			}
			return SubString::create(str, (m_Str + m_Length) - str);
		}

		bool32 compare(const char* str) {

			uint32 count = 0;
			while (*str != 0 && count < m_Length) {
				if (*str != m_Str[count]) {
					return false;
				}
				count++;
				str++;
			}

			if (*str == '\0' && count == m_Length) {
				return true;
			}
			return false;
		}

		SubString(const char* str) {
			m_Str = (char*)str;
			m_Length = stringLength(str);
		}

		const sizeptr getLength() const { return m_Length; }
		const char* getC_Str() const { return m_Str; }
		char getChar(sizeptr index) const { return m_Str[index]; }

	protected:
		char* m_Str;
		sizeptr m_Length;
	};

	inline bool32 operator==(SubString left, const char* right) {
		return left.compare(right);
	}

	inline sizeptr minSizeptr(sizeptr l, sizeptr r) {
		return (l < r) ? l : r;
	}

	inline bool32 subStringCopy(const SubString& substring, char* dst, sizeptr maxsize) {
		sizeptr end = minSizeptr(substring.getLength() + 1, maxsize);
		Base::copyMemory((void*)substring.getC_Str(), (void*)dst, end - 1);
		dst[end] = '\0';
		return true;
	}
	
	template<typename allocator>
	class String {
	public:
		static String create(sizeptr size) {
			String string;
			string.m_Size = size;
			string.m_Str = (char*)allocator::alloc(size);

			Base::setMemoryUint8(string.m_Str, string.m_Size, 0);
			return string;
		}

		static String create(const char* str) {

			String string;
			string.m_Size = stringLength(str) + 1;
			string.m_Str = (char*)allocator::alloc(string.m_Size);
			copyMemory((void*)str, (void*)string.m_Str, string.m_Size);
			string.m_Str[string.m_Size - 1] = '\0';

			return string;
		}

		static String create(SubString str) {
			String string;
			string.m_Size = str.getLength() + 1;
			string.m_Str = (char*)allocator::alloc(string.m_Size);
			copyMemory((void*)str.getC_Str(), (void*)string.m_Str, str.getLength());
			string.m_Str[string.m_Size - 1] = '\0';
			return string;
		}

		static bool32 destroy(String* str) {
			allocator::dealloc((void*)str->m_Str);
			str->m_Str = nullptr;
			str->m_Size = 0;
			return true;
		}

		SubString getSubString() {
			return SubString::create(m_Str, stringLength(m_Str));
		}

		const SubString getSubString() const {
			return SubString::create(m_Str, stringLength(m_Str));
		}

		void replace(char comperand, char exchange) {

			char* str = m_Str;
			while (*str != '\0') {
				if (*str == comperand) {
					*str = exchange;
				}
				str++;
			}

		}
	

		template<typename allocator_ = allocator>
		String getTill(char c) {
			return String<allocator_>::create(getSubString().getTill(c));
		}

		template<typename allocator_ = allocator>
		String getTillLast(char c) {
			return String<allocator_>::create(getSubString().getTillLast(c));
		}

		template<typename allocator_ = allocator>
		String<allocator_> getTillLastExcluding(char c) {
			return String<allocator_>::create(getSubString().getTillLastExcluding(c));
		}

		template<typename allocator_ = allocator>
		String getFromLast(char c) {
			return String<allocator_>::create(getSubString().getFromLast(c));
		}

		template<typename allocator_ = allocator>
		String getFrom(char c) {
			return String<allocator_>::create(getSubString().getFrom(c));
		}

		bool32 compare(const char* str) const {
			return stringCompare(this->m_Str, str);
		}

		String& append(SubString a) {
			sizeptr newsize = m_Size + a.getLength();
			char* newstring = (char*)allocator::alloc(newsize);

			copyMemory((void*)m_Str, (void*)(newstring), m_Size - 1);
			copyMemory((void*)a.getC_Str(), (void*)(newstring + m_Size - 1), a.getLength());

			newstring[newsize - 1] = '\0';
			allocator::dealloc((void*)m_Str);
			m_Str = newstring;
			m_Size = newsize;

			return *this;
		}

		String& appendFront(const char* string) {

			sizeptr al = stringLength(string);
			sizeptr newsize = m_Size + al;

			char* newstring = (char*)allocator::alloc(newsize);

			copyMemory((void*)string, (void*)newstring, al);
			copyMemory((void*)m_Str, (void*)(newstring + al), m_Size);

			allocator::dealloc(m_Str);

			m_Str = newstring;
			m_Size = newsize;

			return *this;
		}

		bool32 contains(char c) {
			char* strptr = m_Str;

			while (*strptr != '\0') {
				if (*strptr == c) {
					return true;
				}
				strptr++;
			}
			return false;
		}

		const sizeptr getSize() const { return m_Size; }
		const char* getC_Str() const { return m_Str; }
		char getChar(sizeptr index) const { return m_Str[index]; }


		void set(const char* str) {
			sizeptr newsize = stringLength(str) + 1;
			if (newsize <= m_Size) {
				copyMemory((void*)str, (void*)m_Str, newsize);
				m_Str[newsize - 1] = '\0';
				return;
			}
			allocator::dealloc((void*)m_Str);
			m_Size = newsize;
			m_Str = (char*)allocator::alloc(newsize);
			copyMemory((void*)str, (void*)m_Str, newsize);
		}

		char* m_Str;
		sizeptr m_Size;
	};

	template<typename allocator>
	bool32 operator==(const String<allocator>& left, const char* right) {
		return left.compare(right);
	}


	template<typename allocator>
	class FilePath {
	public:
		static FilePath create(const char* path) {
			FilePath result{};

			result.m_Str = String<allocator>::create(path);
			result.m_Str.replace('\\', '/');

			return result;
		}

		template<typename allocator_ = allocator>
		String<allocator_> extension() {
			return m_Str. template getFromLast<allocator_>('.');
		}

		void validate() {
			m_Str.replace('\\', '/');
		}

		const char* getC_str() const {
			return m_Str.getC_Str();
		}

		bool32 isAbsolute() const {
			return m_Str.m_Str[1] == ':';
		}

		bool32 makeAbsolute(const char* workingdir) {
			m_Str.appendFront("/").appendFront(workingdir);
			m_Str.replace('\\', '/');
			return true;
		}

		bool32 useBackSlashes() {
			m_Str.replace('/', '\\');
			return true;
		}

		bool32 isFile() {
			return m_Str.contains('.');
		}

		template<typename allocator_ = allocator>
		String<allocator_> getDirectory() {
			return m_Str.getTillLastExcluding<allocator_>('/');
		}

		template<typename allocator_ = allocator> 
		String<allocator_>  getFileName() {
			return String<allocator_>::create(m_Str.getSubString().getFromLast('/').moveHead(1).getTillLast('.') - 1);
		}


		String<allocator> m_Str;
	private:
	};
}