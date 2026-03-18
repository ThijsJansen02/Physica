#pragma once
#include "Engine.h"
#include <typeinfo>

namespace PH::Engine::Assets {

	struct Asset;

	template<uint64(*hash)(UUID id), bool32(*compare)(UUID l, UUID r), typename allocator>
	struct AssetStorage {
	
		/// <summary>
		/// a linked list that can hold assets of any type
		/// </summary>
		struct LList {
		public:

			template<typename type>
			struct Link {
				static_assert(std::is_base_of<Asset, type>::value, "type is not an asset");

				Link<Asset>* next;
				Link<Asset>* prev;

				type storage;
			};

			static LList create() {
				LList result{};

				result.head = nullptr;
				result.tail = nullptr;

				return result;
			}

			template<typename type>
			void pushBack(const type& asset) {
				static_assert(std::is_base_of<Asset, type>::value,
					"type is not an asset");

				Link<type> newnode{};

				newnode.next = nullptr;
				newnode.prev = tail;
				newnode.storage = asset;

				Link<type>* newnodeptr = (Link<type>*)allocator::alloc(sizeof(Link<type>));
				*newnodeptr = newnode;

				if (tail) { tail->next = (Link<Asset>*)newnodeptr; };

				tail = (Link<Asset>)newnodeptr;
				if (!head) { head = (Link<Asset>*)newnodeptr; };
			}


			void removeNode(Link<Asset>* node) {
				if (node->prev) { node->prev->next = node->next; }
				if (node->next) { node->next.prev = node->prev; }

				allocator::dealloc(node);
			}

			template<typename type>
			type* pushFront(const type& asset) {
				Link<type> newnode{};

				newnode.next = head;
				newnode.prev = nullptr;
				newnode.storage = asset;


				Link<type>* newnodeptr = (Link<type>*)allocator::alloc(sizeof(Link<type>));
				*newnodeptr = newnode;

				if (head) { head->prev = (Link<Asset>*)newnodeptr; }

				head = (Link<Asset>*)newnodeptr;
				if (!tail) { tail = (Link<Asset>*)newnodeptr; }

				return &newnodeptr->storage;
			}

			void pushLinkFront(Link<Asset>* link) {
				if (head) { head->prev = link; }
				head = link;

				if (!tail) { tail = link; }
			}

			void release() {
				while (head) {
					Link<Asset>* tobedeleted = head;
					head = head->next;
					allocator::dealloc(tobedeleted);
				}
			}

			struct Iterator {
				void operator++() {
					ptr = ptr->next;
				}
				void operator--() {
					ptr = ptr->prev;
				}

				Asset& operator*() {
					return (ptr->storage);
				}

				const Asset& operator*() const {
					return (ptr->storage);
				}


				bool operator==(Iterator other) {
					return ptr == other.ptr;
				}

				bool operator!=(Iterator other) {
					return !(*this == other);
				}
				Link<Asset>* ptr;
			};

			Iterator begin() const {
				return { head };
			}

			Iterator end() const {
				return { nullptr };
			} 

			Link<Asset>* head;
			Link<Asset>* tail;
		};


		//the table that holds the linked lists of assets, is not of standard form because the linked lists can hold assets of any type, so the table is just an array of linked lists
		using Table = Base::DynamicArray<LList, allocator>;
		using GroupsMap = Base::ChainedHashMap<uint64, Base::ArrayList<Asset*, allocator>, Base::uint64Hash, Base::uint64Compare, allocator>;

		static AssetStorage create(sizeptr tablesize) {
			AssetStorage<hash, compare, allocator> result;

			result.count = 0;
			result.groups = GroupsMap::create(10);
			result.table = Table::create(tablesize, LList::create());
			return result;
		}

	private:
		void release() {
			for (LList& list : table) {
				list.release();
			}
			
			groups.forEach([](Base::ArrayList<Asset*, allocator>& group, const uint64& id, void* userdata) {
				
				group.release();

				}, nullptr);


			table.release();
			GroupsMap::destroy(&groups);
			*this = {};
		}

		static typename LList::template Link<Asset>* castAssetToLink(Asset* asset) {
			return (typename LList::template Link<Asset>*)((uint8*)asset - (sizeof(typename LList::template Link<Asset>*) * 2));
		}

	public:

		void resize(sizeptr newcapacity) {

			AssetStorage newstorage = create(newcapacity);

			this->forEach([](UUID id, Asset* asset, void* userdata) {
				AssetStorage* storage = (AssetStorage*)userdata;
				storage->addLink(castAssetToLink(asset), id);
			}, &newstorage);
			
			release();

			*this = newstorage;
		}
		

		uint64 getIndex(UUID id) {
			return hash(id) % table.getCapacity();
		}

		template<typename type>
		type* add(UUID id, const type& asset) {

			static_assert(std::is_base_of<Asset, type>::value, "type is not an asset");

			if (count >= (real32)table.getCapacity() * 0.75f) {
				resize(count * 3);
			}
			
			//find the list that the asset should be put in
			uint64 tableindex = getIndex(id);
			LList* list = &table[tableindex];

			//insert the asset into the list
			type* insert = list->pushFront<type>(asset);

			//add a reference in the its corresponding group
			uint64 groupid = typeid(type).hash_code();
			Base::ArrayList<Asset*, allocator>* group = groups.get_last(groupid);
			if (!group) {
				group = groups.add(groupid, Base::ArrayList<Asset*, allocator>::create(1));
			}
			
			group->pushBack((Asset*)insert);
		
			//increment the amount of assets placed in the storage
			count++;

			return insert;
		}

		template<typename type, typename allocator_ = allocator>
		Base::ArrayList<type*, allocator_> getAllOfType() {

			uint64 groupid = typeid(type).hash_code();
			Base::ArrayList<Asset*, allocator>* group = groups.get_last(groupid);
			
			if (!group) {
				return Base::ArrayList<type*, allocator_>::create(0);
			}

			Base::ArrayList<type*, allocator_> result = Base::ArrayList<type*, allocator_>::create(group->getCount());
			for (Asset* asset : *group) {
				result.pushBack((type*)asset);
			}
			return result;
		}

		template<typename type> 
		type* get(UUID id) {
			uint64 tableindex = getIndex(id);
			const LList& list = table[tableindex];

			for (Asset& asset : list) {
				if (compare(id, asset.id)) {
					return (type*)&asset;
				}
			}
			return nullptr;
		}

		bool32 has(UUID id) {
			uint64 tableindex = getIndex(id);
			const LList& list = table[tableindex];
			for (Asset& asset : list) {
				if (compare(id, asset.id)) {
					return true;
				}
			}
			return false;
		}

		inline void forEach(void (*method)(UUID id, Asset* asset, void*), void* userdata) {

			for (auto chain : table) {
				for (Asset& asset : chain) {
					method(asset.id, &asset, userdata);
				}
			}
		}

		GroupsMap groups;
		Table table;
		sizeptr count;
		
	private:
		void addLink(typename LList::template Link<Asset>* link, UUID id) {
			
			uint64 tableindex = getIndex(id);
			LList* list = &table[tableindex];

			list->pushLinkFront(link);
		}
	};



}