#pragma once
#include "../Base.h"
#include "Array.h"
#include "LinkedList.h"
#include "Base/random.h"


namespace PH::Base {

	inline uint64 uint32Hash(const uint32& v) {
		return pcg_hash(v);
	}

	inline bool32 uint32Compare(const uint32& l, const uint32& r) {
		return l == r;
	}

	inline uint64 uint64Hash(const uint64& v) {
		return v;
	}

	inline bool32 uint64Compare(const uint64& l, const uint64& r) {
		return l == r;
	}

	template<
		typename Key, 
		typename Value, 
		uint64 hash(const Key& key), 
		bool32 compare(const Key& k1, const Key&k2), 
		typename allocator
	>

	struct ChainedHashMap {

		using ValueType = Value;
		using KeyType = Key;

		struct KeyValuePair {
			Key key;
			Value value;
		};

		using Chain = DoublyLinkedList<KeyValuePair, allocator>;
		using Table = DynamicArray<Chain, allocator>;

		static ChainedHashMap create(uint32 basesize) {
			ChainedHashMap result;

			result.count = 0;
			result.table = Table::create(basesize, Chain::create());

			return result;
		}

		static void destroy(ChainedHashMap* map) {
			for (auto chain : map->table) {
				chain.release();
			}
			map->table.release();
			*map = {};
		}

		/// <summary>
		/// resizes the hashmap to another size. Note the bigger the hashmap the lower the chance of a collision making the map faster. the map also dynamically scales up when inserting new elements.
		/// 
		/// TODO resize copies all the nodes into new memory at every resize, nodes should be recycled to avoid memory fragmentation
		/// </summary>
		/// <param name="newsize">the new size of the hashtable</param>
		inline void resize(sizeptr newsize) {

			ChainedHashMap newmap = create(newsize);

			for (Chain& chain : table) {
				for (auto& [key, value] : chain) {
					newmap.add(key, value);
				}
			}

			for (Chain& chain : table) {
				chain.release();
			}

			table.release();

			*this = newmap;
		}

		/// <summary>
		/// returns wether the key is present in the map
		/// </summary>
		/// <param name="key">the key</param>
		/// <returns>wether the key is present in the map</returns>
		inline bool32 contains_key(const KeyType& key) {
	
			Chain& chain = *getChain(key);

			for (auto& [key_, value_] : chain) {
				if (compare(key_, key)) {
					return true;
				}
			}
			return false;
		}

		/// <summary>
		/// returns the last inserted value that the key maps to. nullptr if key doesnt exist.
		/// </summary>
		/// <param name="key">the key that maps to the value</param>
		/// <returns>the value that belongs to the key</returns>
		ValueType* get_last(const KeyType& key) {
			uint64 hash_ = hash(key);
			hash_ = hash_ % table.getCapacity();

			Chain* chain = &table[hash_];
			for (auto& [key_, value_] : *chain) {
				if (compare(key_, key)) {
					return &value_;
				}
			}
			return nullptr;
		}
		/// <summary>
		/// returns all values that the given key maps to.
		/// </summary>
		/// <typeparam name="ALLOCATOR">the allocator for the resulting arraylist</typeparam>
		/// <typeparam name="DEALLOCATOR">the deallocator for the resulting arraylist</typeparam>
		/// <param name="key">the key on which to search</param>
		/// <returns>an array with all the values the key maps to</returns>
		template<typename allocator>
		ArrayList<ValueType*, allocator> getAll(const KeyType& key) {
			ArrayList<ValueType*, allocator> result = ArrayList<ValueType*, allocator>::create(1);

			uint64 hash_ = hash(key);
			hash_ = hash_ % table.getCapacity();

			Chain* chain = &table[hash_];
			for (auto& [key_, value_] : *chain) {
				if (compare(key_, key)) {
					result.pushBack(&value_);
				}
			}
			return result;
		}

		/// <summary>
		/// returns all values
		/// </summary>
		/// <typeparam name="ALLOCATOR">the allocator for the resulting arraylist</typeparam>
		/// <typeparam name="DEALLOCATOR">the deallocator for the resulting arraylist</typeparam>
		/// <param name="key">the key on which to search</param>
		/// <returns>an array with all the values the key maps to</returns>
		template<typename allocator>
		ArrayList<ValueType*, allocator> getAll() const {
			ArrayList<ValueType*, allocator> result = ArrayList<ValueType*, allocator>::create(1);
			for (auto chain : table) {
				for (auto& [key, value] : chain) {
					result.pushBack(&value);
				}
			}
			return result;
		}

		/// <summary>
		/// returns all values
		/// </summary>
		/// <typeparam name="ALLOCATOR">the allocator for the resulting arraylist</typeparam>
		/// <typeparam name="DEALLOCATOR">the deallocator for the resulting arraylist</typeparam>
		/// <param name="key">the key on which to search</param>
		/// <returns>an array with all the values the key maps to</returns>
		template<typename allocator>
		ArrayList<KeyValuePair*, allocator> getAllPairs() const {
			ArrayList<KeyValuePair*, allocator> result = ArrayList<KeyValuePair*, allocator>::create(1);
			for (auto chain : table) {
				for (auto& val : chain) {
					result.pushBack(&val);
				}
			}
			return result;
		}

		Chain* getChain(const Key& key) {
			uint64 hash_ = hash(key);
			hash_ = hash_ % table.getCapacity();
			return &table[hash_];
		}

		/// <summary>
		/// adds a new key value mapping
		/// </summary>
		/// <param name="key">the key the maps to the value</param>
		/// <param name="value">the value that the key should map to</param>
		inline ValueType* add(const KeyType& key, const ValueType& value) {

			if (count >= (real32)table.getCapacity() * 0.75f) {
				resize(count * 3);
			}

			uint64 hash_ = hash(key);
			hash_ = hash_ % table.getCapacity();

			Chain* chain = &table[hash_];

			KeyValuePair insert{};
			insert.key = key;
			insert.value = value;

			chain->pushFront(insert);
			count++;

			return &chain->firstnode->element.value;
		}

		inline void forEach(void (*method)(const Value&, const Key&, void*), void* userdata) {

			for (auto chain : table) {
				for (auto& [key, value] : chain) {
					method(value, key, userdata);
				}
			}
		}

		inline bool32 remove(const Key& key) {
			Chain& chain = *getChain(key);
			
			uint32 index = 0;
			for (auto& [key_, value] : chain) {
				if (compare(key, key_)) {
					//should figure out a more effective way to remove elements from a linked list
					chain.remove(index);
					return true;
				}
				index++;
			}
			return false;
		}

		/// <summary>
		/// adds the key value mapping only if no other value is already mapped to the same key
		/// </summary>
		/// <param name="key">the key that must map to the value</param>
		/// <param name="value">the value that the key maps to</param>
		inline void addDistinct(const KeyType& key, const ValueType& value) {

			if (count >= (real32)table.getCapacity() * 0.75f) {
				resize(count * 3);
			}

			uint64 hash_ = hash(key);
			hash_ = hash_ % table.count;

			Chain* chain = &table[hash_];

			KeyValuePair insert;
			insert.key = key;
			insert.value = value;

			chain->pushFront(insert);
			count++;
		}

		Table table;
		sizeptr count;
	};
}