/*
 * This file is part of Micro Bench
 * <https://github.com/duckmaestro/Micro-Bench>.
 * 
 * Micro Bench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Micro Bench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#pragma once

#include <iterator>
#include <algorithm>
#include <cassert>

class Mathx
{
public:
	
	template<typename TValue, typename TItem, typename TContainer, typename TSelector> 
	static TValue Mean(const TContainer& container, TSelector selector)
	{
		TValue value = 0;

		auto begin = std::begin(container);
		auto end = std::end(container);
		int count = 0;

		for(auto iter = begin; iter != end; ++iter)
		{
			TValue temp = selector(*iter);
			value += temp;
			++count;
		}

		value /= (TValue)count;
		return value;
	}

	template<typename TValue, typename TItem, typename TContainer, typename TSelector>
	static TValue Median(const TContainer& container, TSelector selector)
	{
		// todo: this is probably slow
		TContainer container2 = container;

		if(container2.empty())
		{
			return 0;
		}

		TContainer::size_type middleIndex = container2.size() / 2;
		auto begin = std::begin(container2);
		auto end = std::end(container2);
		auto middle = begin + middleIndex;
		std::nth_element(
			begin, 
			middle, 
			end, 
			[&selector](TItem& item1, TItem& item2)
			{
				return selector(item1) < selector(item2);
			}
		);
		auto middle2 = begin + middleIndex;
		return selector(*middle2);
	}

	template<typename TValue, typename TItem, typename TContainer, typename TSelector>
	static TValue Minimum(const TContainer& container, TSelector selector)
	{
		// todo: this is probably slow
		TContainer container2 = container;

		auto begin = std::begin(container2);
		auto end = std::end(container2);
		std::sort(
			begin, 
			end, 
			[&selector](TItem& item1, TItem& item2)
			{
				return selector(item1) < selector(item2);
			}
		);
		auto min = *(container2.begin());
		return selector(min);
	}

	template<typename TValue, typename TItem, typename TContainer, typename TSelector>
	static TValue Maximum(const TContainer& container, TSelector selector)
	{
		// todo: this is probably slow
		TContainer container2 = container;

		auto begin = std::begin(container2);
		auto end = std::end(container2);
		std::sort(
			begin, 
			end, 
			[&selector](TItem& item1, TItem& item2)
			{
				return selector(item1) < selector(item2);
			}
		);
		auto max = *(container2.rbegin());
		return selector(max);
	}

	template<typename TValue, typename TItem, typename TContainer, typename TSelector>
	static TValue Percentile(const TContainer& container, TSelector selector, int percentile)
	{
		assert(percentile <= 100 && percentile >= 0);

		if(percentile == 100)
		{
			return Maximum<TValue, TItem>(container, selector);
		}
		else if(percentile == 0)
		{
			return Minimum<TValue, TItem>(container, selector);
		}
		else if(percentile == 50)
		{
			return Median<TValue, TItem>(container, selector);
		}

		// todo: this is probably slow
		TContainer container2 = container;

		if(container2.empty())
		{
			return 0;
		}

		TContainer::size_type indexToUse = percentile * container2.size() / 100;
		auto begin = std::begin(container2);
		auto end = std::end(container2);
		auto middle = begin + indexToUse;
		std::nth_element(
			begin, 
			middle, 
			end, 
			[&selector](TItem& item1, TItem& item2)
			{
				return selector(item1) < selector(item2);
			}
		);
		auto middle2 = begin + indexToUse;
		return selector(*middle2);
	}
};

