#pragma once
#include "../Base.h"
#include <initializer_list>

namespace PH::Base {

	template<typename T, typename allocator>
	struct DoublyLinkedList {

		using ValueType = T;

		struct Node {
			T element;
			Node* next;
			Node* prev;
		};

		struct Iterator {
			void operator++() {
				ptr = ptr->next;
			}
			void operator--() {
				ptr = ptr->prev;
			}

			ValueType& operator*() {
				return (ptr->element);
			}

			bool operator==(Iterator other) {
				return ptr == other.ptr;
			}

			bool operator!=(Iterator other) {
				return !(*this == other);
			}
			Node* ptr;
		};

		static DoublyLinkedList create(std::initializer_list<T> init) {
			DoublyLinkedList result = create();

			for (auto elem : init) {
				result.push_back(elem);
			}

			return result;
		}

		static DoublyLinkedList create() {
			DoublyLinkedList result{};

			result.firstnode = nullptr;
			result.lastnode = nullptr;

			return result;
		}

		void remove(sizeptr index) {
			uint32 i = 0;
			Node* ptr = firstnode;

			while (i < index) {
				ptr = ptr->next;
				i++;
			}

			Node* next = ptr->next;
			Node* prev = ptr->prev;
			if (prev) {
				prev->next = next;
			}
			if (next) {
				next->prev = prev;
			}

			allocator::dealloc(ptr);
		}

		void add(const T& elem) {
			push_back(elem);
		}

		void contains(const T& elem) {

			for (const T& elem_ : *this) {
				if (elem_ == elem) {
					return true;
				}
			}
			return false;
		}

		void pushBack(const T& elem) {
			Node newnode{};

			newnode.next = nullptr;
			newnode.prev = lastnode;
			newnode.element = elem;

			Node* newnodeptr = (Node*)allocator::alloc(sizeof(Node));
			*newnodeptr = newnode;

			if (lastnode) { lastnode->next = newnodeptr };

			lastnode = newnodeptr;
			if (!firstnode) { firstnode = newnodeptr };
		}

		void removeNode(Node* node) {
			if (node->prev) { node->prev->next = node->next }
			if (node->next) { node->next.prev = node->prev }

			allocator::dealloc(node);
		}

		void pushFront(const T& elem) {
			Node newnode{};

			newnode.next = firstnode;
			newnode.prev = nullptr;
			newnode.element = elem;


			Node* newnodeptr = (Node*)allocator::alloc(sizeof(Node));
			*newnodeptr = newnode;

			if (firstnode) { firstnode->prev = newnodeptr; }

			firstnode = newnodeptr;
			if (!lastnode) { lastnode = newnodeptr; }
		}

		void release() {
			while (firstnode) {
				Node* tobedeleted = firstnode;
				firstnode = firstnode->next;
				allocator::dealloc(tobedeleted);
			}
		}

		Node* firstnode;
		Node* lastnode;

		Iterator begin() { return { firstnode }; }
		Iterator end() { return { nullptr }; }
	};
}

