#pragma once
#include "Base/Base.h"

namespace PH::Base {

	template<typename type>
	class ArrayIterator {
	public:
		static ArrayIterator<type> create(type* ptr) {
			ArrayIterator<type> it;
			it.ptr = ptr;
			return it;
		}

		ArrayIterator<type>& operator++() {
			++ptr;
			return *this;
		}

		ArrayIterator<type> operator++(int) {
			type* prev = ptr;
			++ptr;
			return create(prev);
		}

		void operator--() {
			ptr--;
		}

		type& operator*() {
			return *ptr;
		}

		const type& operator*() const {
			return *ptr;
		}

		ArrayIterator<type> operator+(sizeptr addition) const {
			return create(ptr + addition);
		}

		ArrayIterator<type> operator-(sizeptr subtraction) const {
			return create(ptr + subtraction);
		}

		sizeptr operator-(ArrayIterator<type> other) const {
			return this->ptr - other.ptr;
		}

		bool32 operator==(ArrayIterator<type> other) {
			return this->ptr == other.ptr;
		}

		bool32 operator!=(ArrayIterator<type> other) {
			return this->ptr != other.ptr;
		}
		type* ptr;
	};

	template<typename type>
	class constArrayIterator {
	public:
		static constArrayIterator<type> create(type* ptr) {
			constArrayIterator<type> it;
			it.ptr = ptr;
			return it;
		}

		const constArrayIterator<type>& operator++() {
			ptr++;
			return *this;
		}

		constArrayIterator<type> operator++(int) {
			type* prev = ptr;
			++ptr;
			return create(prev);
		}

		void operator--() {
			ptr--;
		}

		const type& operator*() const {
			return *ptr;
		}

		constArrayIterator<type> operator+(sizeptr addition) {
			return create(ptr + addition);
		}

		constArrayIterator<type> operator-(sizeptr subtraction) {
			return create(ptr + subtraction);
		}

		sizeptr operator-(constArrayIterator<type> other) const {
			return this->ptr - other.ptr;
		}

		bool32 operator==(constArrayIterator<type> other) {
			return this->ptr == other.ptr;
		}

		bool32 operator!=(constArrayIterator<type> other) {
			return this->ptr != other.ptr;
		}
		type* ptr;
	};

	template<typename type>
	sizeptr operator-(ArrayIterator<type> left, constArrayIterator<type> right) {
		return left.ptr - right.ptr;
	}


	template<typename T>
	struct Array {
		using type = T;

		using iterator = ArrayIterator<type>;
		using constiterator = constArrayIterator<type>;

		static Array create(type* data, sizeptr count) {
			Array array;
			array.data = data;
			array.count = count;
			return array;
		}

		static const Array create(const type* data, sizeptr count) {
			Array array;
			array.data = (type*)data;
			array.count = count;
			return array;
		}

		type& operator[](sizeptr index) {
			PH_DEBUG_ASSERT(index < count, "trying to index memory that is not allocated!")
			return data[index];
		}

		const type& operator[](sizeptr index) const {
			PH_DEBUG_ASSERT(index < count, "trying to index memory that is not allocated!")
			return data[index];
		}

		iterator begin() {
			return iterator::create(data);
		}

		iterator end() {
			return iterator::create(data + count);
		}

		constiterator begin() const {
			return constiterator::create(data);
		}

		constiterator end() const {
			return constiterator::create(data + count);
		}

		type* data;
		sizeptr count;
	};

	template<typename T, typename allocator>
	class DynamicArray {
	public:
		using type = T;

		using iterator = ArrayIterator<type>;
		using constiterator = constArrayIterator<type>;

		static DynamicArray create(sizeptr count) {
			if (count <= 0) {
				return {};
			}
			DynamicArray array;
			array.m_Data = (type*)allocator::alloc(count * sizeof(type));
			array.m_Count = count;

			return array;
		}

		static DynamicArray create(sizeptr count, type value) {
			DynamicArray array;
			array.m_Data = (type*)allocator::alloc(count * sizeof(type));
			array.m_Count = count;

			for (auto& v : array) {
				v = value;
			}

			return array;
		}

		DynamicArray() : m_Data(nullptr), m_Count(0) {

		}

		void release() {
			if (m_Data) {
				allocator::dealloc(m_Data);
			}
			m_Count = 0;
		}

		void resize(sizeptr count) {

			//if the array is shrunk than we should not copy up untill m_Count
			if (count < m_Count) {
				m_Count = count;

			}
			//might want to de the set memory a bit more sophisticated
			type* newarray = (type*)allocator::alloc(count * sizeof(type));

			if (count > 0) {
				PH::Base::setMemoryUint8(newarray, sizeof(type) * count, 0);
				PH::Base::copyMemory(m_Data, newarray, m_Count * sizeof(type));
			}

			allocator::dealloc(m_Data);
			m_Count = count;
			m_Data = newarray;
		}

		type& operator[](sizeptr index) {
			PH_DEBUG_ASSERT(index < m_Count, "trying to index memory that is not allocated!")

			return m_Data[index];
		}

		const type& operator[](sizeptr index) const {
			PH_DEBUG_ASSERT(index < m_Count, "trying to index memory that is not allocated!")

			return m_Data[index];
		}

		bool32 empty() const {
			return m_Count == 0;
		}

		Array<type> getArray() const {
			return Array<type>::create(m_Data, m_Count);
		}

		sizeptr getCapacity() const {
			return m_Count;
		}

		iterator begin() {
			return iterator::create(m_Data);
		}

		iterator end() {
			return iterator::create(m_Data + m_Count);
		}

		constiterator begin() const {
			return constiterator::create(m_Data);
		}

		constiterator end() const {
			return constiterator::create(m_Data + m_Count);
		}

		type* raw() {
			return m_Data;
		}
		const type* raw() const {
			return m_Data;
		}

	protected:
		type* m_Data;
		sizeptr m_Count;
	};

	

	template<typename T, typename allocator>
	class ArrayList {
	public:
		using type = T;

		using storage = DynamicArray<type, allocator>;
		using iterator = typename storage::iterator;
		using constiterator = typename storage::constiterator;

		static ArrayList<type, allocator> create(sizeptr capacity) {
			return ArrayList(capacity);
		}

		static ArrayList<type, allocator> createFilled(sizeptr capacity, const T& v) {
			auto r = ArrayList(capacity);
			for (uint32 i = 0; i < capacity; i++) {
				r.pushBack(v);
			}

			return r;

		}

		static ArrayList<type, allocator> create() {
			return ArrayList(1);
		}

		static ArrayList<type, allocator> create(Base::Array<T> copy) {
			 auto a = ArrayList(copy.count);
			 for (const auto& el : copy) {
				 a.pushBack(el);
			 }
			 return a;
		}

		ArrayList() {

		}

		template<typename allocator_ = allocator>
		ArrayList<T, allocator_> createCopy() {
			ArrayList<T, allocator_> copy = ArrayList<T, allocator_>::create(this->getCount());
			for (auto& el : *this) {
				copy.pushBack(el);
			}

			return copy;
		}

		iterator begin() {
			return m_Storage.begin();
		}

		iterator end() {
			return m_End;
		}

		sizeptr getCapacity() const {
			return m_Storage.getCapacity();
		}

		sizeptr getCount() const {
			return m_End - m_Storage.begin();
		}

		void release() {
			m_Storage.release();
		}

		Array<type> getArray() const {
			return Array<type>::create(raw(), getCount());
		}

		bool32 contains(const type& el) const {
			for (const type& item : m_Storage) {
				if (el == item) {
					return true;
				}
			}
			return false;
		}

		void push(const type& el) {
			pushBack(el);
		}

		type& pushBack(const type& el) {
			sizeptr count = getCount();
			if (count >= getCapacity()) {
				this->m_Storage.resize((getCapacity() + 1) * 2);
				this->m_End = m_Storage.begin() + count;
			}
			*m_End = el;
			type& returnv = *m_End;
			++m_End;
			return returnv;
		}

		type& popBack() {
			type& returnv = *m_End;
			--m_End;
			return returnv;
		}

		template<typename allocator_>
		void pushBack(const ArrayList<type, allocator_>& other) {

			for (auto& el : other) {
				pushBack(el);
			}
			return;
		}

		void pushBack(const Array<type>& other) {

			for (auto& el : other) {
				pushBack(el);
			}
			return;
		}

		void pushFront(const type& el) {
			sizeptr count = getCount();
			if (count >= getCapacity()) {
				sizeptr newcapacity = (getCapacity() + 1) * 2;
				type* newarray = allocator::alloc(newcapacity * sizeof(type));
				PH::Base::copyMemory(m_Storage.raw(), newarray + 1, count);
				allocator::dealloc(m_Storage.m_Data);

				m_Storage.m_Data = newarray;
				m_Storage.m_Count = newcapacity;

;				m_end = m_Storage.begin() + count + 1;
			}
			else {
				PH::Base::copyMemoryReverse(m_Storage.raw(), m_Storage.raw() + 1, count);
			}
			*begin() = el;
			return;
		}

		type& operator[](sizeptr index) {
			return m_Storage[index];
		}

		const type& operator[](sizeptr index) const {
			return m_Storage[index];
		}

		void clear() {
			m_End = begin();
		}

		constiterator begin() const {
			return m_Storage.begin();
		}

		constiterator end() const {
			return constiterator::create(m_End.ptr);
		}

		type* raw() {
			return m_Storage.raw();
		}
		const type* raw() const {
			return m_Storage.raw();
		}
	private:
		ArrayList(sizeptr capacity) {
			m_Storage = storage::create(capacity);
			m_End = m_Storage.begin();
		}

		iterator m_End;
		storage m_Storage;
	};
}
