#pragma once

#include <vector>

/*
 * Binary heap where the lowest element is always on top (could change to maxHeap by just flipping the comparison operators everywhere)
 * Implementation uses an array
 * Nodes are organized in the array using the following properties:
 * The node at index i has:
 * children at indices 2i + 1 and 2i + 2
 * a parent at index floor((i - 1) / 2)
 */

template<class T>
class MinHeap
{
public:
	std::vector<T> m_values;

	unsigned int GetSize() const;

	bool IsEmpty() const;

	// Returns the array index of the value's final resting place
	int Push(const T &value);

	// Returns the array index of the value's final resting place
	int SiftUp(int index);

	void SiftDown(int index);

	T Pop();
};

template<class T>
unsigned int MinHeap<T>::GetSize() const
{
	return m_values.size();
}

template<class T>
bool MinHeap<T>::IsEmpty() const
{
	return m_values.empty();
}

template<class T>
int MinHeap<T>::Push(const T &value)
{
	m_values.push_back(value);

	return SiftUp(m_values.size() - 1);
}

template<class T>
int MinHeap<T>::SiftUp(int index)
{
	while(true)
	{
		// Get the parent of the value at the index
		int parentIndex = static_cast<int>((index - 1) / 2.0f); // Floored by casting to an int
			
		// Check to see that the index exists
		if(parentIndex < 0)
			break;
			
		// Swap if the parent value is greater than the current value
		if(m_values[index] < m_values[parentIndex])
			std::swap(m_values[index], m_values[parentIndex]);
		else
			break;
			
		index = parentIndex;
	}
		
	return index;
}

template<class T>
void MinHeap<T>::SiftDown(int index)
{
	const int numValues = m_values.size();
		
	while(true)
	{
		T indexValue = m_values[index];
			
		// Calculate child indices based on properties given previously
		int firstChildIndex = 2 * index + 1;
		int secondChildIndex = firstChildIndex + 1;
			
		// Alter behavior based on children existence
		if(firstChildIndex < numValues)
		{
			T firstChildValue = m_values[firstChildIndex];
				
			if(secondChildIndex < numValues)
			{
				T secondChildValue = m_values[secondChildIndex];
					
				// Check that the value at index is less than both children
				if(indexValue > firstChildValue || indexValue > secondChildValue)
				{
					// Parent is greater than one of the children, swap it
					// with the smaller one and sift downwards from there
					if(firstChildValue < secondChildValue)
					{
						// Swap
						m_values[index] = firstChildValue;
						m_values[firstChildIndex] = indexValue;
							
						index = firstChildIndex;
					}
					else
					{
						// Swap
						m_values[index] = secondChildValue;
						m_values[secondChildIndex] = indexValue;
							
						index = secondChildIndex;
					}
				}
				else
					return;
			}
			else
			{
				// Compare the index value to the only child value. If it is greater than its child, swap them
				if(indexValue > firstChildValue)
				{
					m_values[index] = firstChildValue;
					m_values[firstChildIndex] = indexValue;
						
					index = firstChildIndex;
				}
				else
					return;
			}
		}
		else
			return;
	}
}

template<class T>
T MinHeap<T>::Pop()
{
	// Check to see that there are values to pop
	if(m_values.empty())
		return T();
		
	T first = m_values[0];
			
	// Get last value and remove it
	T last = m_values.back();
	m_values.pop_back();
		
	if(!m_values.empty())
	{
		// Set the first element to the last one
		m_values[0] = last;
			
		// Sift downwards to re-balance the tree
		SiftDown(0);
	}
		
	return first;
}