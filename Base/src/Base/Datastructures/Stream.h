#pragma once
#include <Base/Base.h>
#include <Base/Memory.h>
#include <Base/Datastructures/String.h>
#include <Base/Datastructures/Array.h>

namespace PH::Base {

	template<typename allocator>
	class Stream {
	public:
		static Stream create(sizeptr basesize) {

			Stream<allocator> result;
			result.m_Data = allocator::alloc(basesize);
			result.m_Size = basesize;
			result.m_Writeptr = 0;
			return result;
		}

		static void destroy(Stream* stream) {
			stream->m_Size = 0;
			stream->m_Writeptr = 0;
			allocator::dealloc(stream->m_Data);
		}

		template<typename T>
		Stream& operator<<(Base::Array<T> data) {
			//resize when 

			sizeptr size = data.count * sizeof(T);

			while (m_Writeptr + size > m_Size) {
				resize(2 * m_Size);
			}

			copyMemory((void*)data.data, (void*)((uint8*)m_Data + m_Writeptr), size);

			m_Writeptr += size;
			return *this;
		}

		Stream& operator<<(const char* str) {
			sizeptr length = Base::stringLength(str) + 1;
			*this << Array<char>{(char*)str, length};
			m_Writeptr--;
			return *this;
		}

		Stream& operator<<(char character) {
			*this << Array<char>{&character, 1};
			return *this;
		}

		Stream& operator<<(uint64 value) {
			*this << Array<uint64>{&value, 1};
			return *this;
		}

		void* raw() {
			return m_Data;
		}

		void* getSize() {
			return m_Writeptr;
		}

		template<typename Allocator> 
		PH::Base::String<Allocator> getString() {
			return PH::Base::String<Allocator>::create((char*)m_Data);
		}

	private:
		void resize(sizeptr newsize) {

			if (newsize < m_Size) {
				m_Size = newsize;
			}

			void* newdata = allocator::alloc(newsize);
			copyMemory(m_Data, newdata, m_Size);
			allocator::dealloc(m_Data);
			m_Data = newdata;
			m_Size = newsize;
		}

		sizeptr m_Writeptr;
		sizeptr m_Size;
		void* m_Data;
	};
}
