#pragma once
#include "Array.h"
#include "Base/Memory.h"
#include "Base/utils.h"

namespace PH::Base {


	template<typename T, typename allocator>
	class FracturedArrayList {
	public:
		using type = T;
		
		static FracturedArrayList create(sizeptr capacity) {
			return FracturedArrayList(capacity);
		}

		FracturedArrayList() {
			m_Occupancy = nullptr;
			m_Data = nullptr;
		}

		/// <summary>
		/// adds an element in the array at the first open spot and returns the index at which it is inserted
		/// </summary>
		/// <param name="el"></param>
		/// <returns></returns>
		sizeptr add(const type& el) {
			sizeptr index = m_FirstOpenSpot;

			if (m_Count >= m_Capacity) {
				sizeptr newcapacity = m_Capacity * 2;
				type* newdata = (type*)allocator::alloc(sizeof(type) * newcapacity);
				bool* newoccupancy = (bool*)allocator::alloc(sizeof(bool) * newcapacity);
				
				for (uint32 i = 0; i < newcapacity; i++) {
					newoccupancy[i] = false;
				}

				copyMemory(m_Data, newdata, m_Capacity * sizeof(type));
				copyMemory(m_Occupancy, newoccupancy, m_Capacity * sizeof(bool));

				allocator::dealloc(m_Data);
				allocator::dealloc(m_Occupancy);

				m_Data = newdata;
				m_Occupancy = newoccupancy;

				m_Capacity = newcapacity;
			}

			m_Data[m_FirstOpenSpot] = el;
			m_Occupancy[m_FirstOpenSpot] = true;

			while (m_Occupancy[m_FirstOpenSpot] == true && m_FirstOpenSpot < m_Capacity) {
				m_FirstOpenSpot++;
			}

			m_End = Base::maxVal<sizeptr>(m_End, m_FirstOpenSpot);
			m_Count++;

			return index;
		}

		/// <summary>
		/// removes the element at index index. and settles the firstopenspot member.
		/// </summary>
		/// <param name="index"></param>
		void remove(sizeptr index) {
			PH_DEBUG_ASSERT(index < m_Capacity && m_Occupancy[index], "trying to remove an element that doenst exist!");
			PH_DEBUG_ASSERT(m_Occupancy[index] == true, "trying to remove an element that is already removed");

			m_Occupancy[index] = false;
			m_FirstOpenSpot = Base::minVal<sizeptr>(m_FirstOpenSpot, index);
			m_Count--;
		}

		type& operator[](sizeptr index) {
			PH_DEBUG_ASSERT(index < m_Capacity && m_Occupancy[index], "trying to access a onoccupied member!");
			return m_Data[index];
		}

		const type& operator[](sizeptr index) const {
			PH_DEBUG_ASSERT(index < m_Capacity && m_Occupancy[index], "trying to access a onoccupied member!");
			return m_Data[index];
		}


	private:
		FracturedArrayList(sizeptr capacity) {

			m_Data = (type*)allocator::alloc(sizeof(type) * capacity);
			m_Occupancy = (bool*)allocator::alloc(sizeof(bool) * capacity);
			m_FirstOpenSpot = 0;
			m_Capacity = capacity;
			m_Count = 0;
			m_End = 0;
			for (uint32 i = 0; i < m_Capacity; i++) {
				m_Occupancy[i] = false;
			}
		}

		sizeptr m_Capacity;
		sizeptr m_Count;
		sizeptr m_End;
		sizeptr m_FirstOpenSpot;
		type* m_Data;
		bool* m_Occupancy;
	};
}
