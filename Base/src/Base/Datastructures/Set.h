#pragma once
#include "Base/Base.h"

template<class T, class U>
struct is_same {
	enum { value = 0 };
};

template<class T>
struct is_same<T, T> {
	enum { value = 1 };
};

namespace PH::Base {
	
	/// <summary>
	/// a iterable type data structure that only contains distinct items. possible storage types are:
	/// - ArrayList
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="storagetype"></typeparam>
	template<typename T, typename storagetype>
	class Set {
	public:
		using storage = storagetype;
		using type = T;

		using iterator = typename storage::iterator;
		using constiterator = typename storage::constiterator;

		void add(const type& el) {
			if (!m_Storage.contains(el)) {
				m_Storage.push(el);
			}
		}

		void contains(const type& el) {
			return m_Storage.contains(el);
		}

		void release() {
			m_Storage.release();
		}

		constiterator begin() const {
			return m_Storage.begin();
		}

		constiterator end() const {
			return m_Storage.end();
		}

		iterator end() {
			return m_Storage.end();
		}

		iterator begin() {
			return m_Storage.begin();
		}

		static Set<type, storage> create(sizeptr capacity) {
			return Set(capacity);
		}

	private:
		Set(sizeptr capacity) : m_Storage(storage::create(capacity)) {
		}
		storage m_Storage;
	};
}
