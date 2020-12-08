/*
	MIT License
	
	Copyright (c) 2020 Marcel Pi Nacy
	
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#include <iterator>
#include <cassert>

namespace osn
{
	namespace detail
	{
		template <typename Iterator>
		constexpr bool is_random_access_iterator = std::is_same<std::iterator_traits<Iterator>::iterator_category, std::random_access_iterator_tag>::value;

		struct alignas(sizeof(size_t) * 2) index_pair
		{
			size_t left;
			size_t right;
		};

		template <typename T>
		constexpr auto array_size(T& array)
		{
			return sizeof(T) / sizeof(array[0]);
		}

		template <typename Iterator>
		constexpr void compare_swap(Iterator left, Iterator right)
		{
			using T = typename std::iterator_traits<Iterator>::value_type;

			constexpr bool is_fundamental = std::is_fundamental<T>::value;
			constexpr bool cmov_capable = (is_fundamental || std::is_trivial<T>::value) && (sizeof(T) <= sizeof(size_t));

			const bool cmp = (*right) < (*left);
			if constexpr (cmov_capable)
			{
				T tmp = *left;
				if (cmp)
					*left = *right;
				if (cmp)
					*right = tmp;
			}
			else
			{
				if (cmp)
					std::iter_swap(left, right);
			}
		}

		template <size_t N>
		struct network_info;

		template <>
		struct network_info<2>
		{
			static constexpr index_pair swap_sequence[] =
			{
				{ 0, 1 }
			};

			static constexpr size_t depth = 1;
		};

		template <>
		struct network_info<3>
		{
			static constexpr index_pair swap_sequence[] =
			{
				{ 1, 2 },
				{ 0, 1 },
				{ 1, 2 },
			};

			static constexpr size_t depth = 3;
		};

		template <>
		struct network_info<4>
		{
			static constexpr index_pair swap_sequence[] =
			{
				{ 0, 1 }, { 2, 3 },
				{ 1, 3 }, { 0, 2 },
				{ 1, 2 }
			};

			static constexpr size_t depth = 3;
		};

		template <>
		struct network_info<5>
		{
			static constexpr index_pair swap_sequence[] =
			{
				{ 1, 2 }, { 3, 4 },
				{ 1, 3 }, { 0, 2 },
				{ 2, 4 }, { 0, 3 },
				{ 0, 1 }, { 2, 3 },
				{ 1, 2 }
			};

			static constexpr size_t depth = 5;
		};

		template <>
		struct network_info<6>
		{
			static constexpr index_pair swap_sequence[] =
			{
				{ 0, 5 }, { 1, 3 }, { 2, 4 },
				{ 1, 2 }, { 3, 4 },
				{ 0, 3 }, { 2, 5 },
				{ 0, 1 }, { 2, 3 }, { 4, 5 },
				{ 1, 2 }, { 3, 4 },
			};

			static constexpr size_t depth = 5;
		};

		template <>
		struct network_info<7>
		{
			static constexpr index_pair swap_sequence[] =
			{
				{ 1, 2 }, { 3, 4 }, { 5, 6 },
				{ 0, 2 }, { 3, 5 }, { 4, 6 },
				{ 2, 6 }, { 1, 5 }, { 0, 4 },
				{ 2, 5 }, { 0, 3 },
				{ 2, 4 }, { 1, 3 },
				{ 0, 1 }, { 2, 3 }, { 4, 5 }
			};

			static constexpr size_t depth = 6;
		};

		template <>
		struct network_info<8>
		{
			static constexpr index_pair swap_sequence[] =
			{
				{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
				{ 0, 2 }, { 1, 3 }, { 4, 6 }, { 5, 7 },
				{ 2, 4 }, { 3, 5 }, { 0, 1 }, { 6, 7 },
				{ 2, 3 }, { 4, 5 },
				{ 1, 4 }, { 3, 6 },
				{ 1, 2 }, { 3, 4 }, { 5, 6 }
			};

			static constexpr size_t depth = 6;
		};
	}



	constexpr size_t max_supported_array_size = 8;

	template <size_t N>
	struct network_traits
	{
		static_assert(N <= max_supported_array_size);

		enum : size_t
		{
			required_swap_count = detail::array_size(detail::network_info<N>::swap_sequence),
			depth = detail::network_info<N>::depth
		};
	};

	template <size_t N, typename RandomAccessIterator>
	constexpr void sort(RandomAccessIterator begin)
	{
		static_assert(
			detail::is_random_access_iterator<RandomAccessIterator>,
			"OSN: Only random access iterators are supported.");

		static_assert(
			N <= max_supported_array_size,
			"OSN: Invalid array size, only sizes 0 to 8 (inclusive) are supported.");

		if constexpr (N > 1)
		{
			using network_info = detail::network_info<N>;
			using detail::index_pair;

			for (index_pair ip : network_info::swap_sequence)
			{
				detail::compare_swap(begin + ip.left, begin + ip.right);
			}
		}
	}

	template <typename RandomAccessIterator>
	constexpr void sort(RandomAccessIterator begin, RandomAccessIterator end)
	{
		static_assert(
			detail::is_random_access_iterator<RandomAccessIterator>,
			"OSN: Only random access iterators are supported.");

		auto size = std::distance(begin, end);
		if (size < 2)
			return;

#ifdef OSN_USE_JUMP_TABLE
		size -= 2;
		using function_pointer_type = void(*)(RandomAccessIterator);
		constexpr function_pointer_type jmp[] =
		{
			sort<2>,
			sort<3>,
			sort<4>,
			sort<5>,
			sort<6>,
			sort<7>,
			sort<8>
		};
		return jmp[size](begin);
#else
		switch (size)
		{
		case 2:
			sort<2>(begin);
			break;
		case 3:
			sort<3>(begin);
			break;
		case 4:
			sort<4>(begin);
			break;
		case 5:
			sort<5>(begin);
			break;
		case 6:
			sort<6>(begin);
			break;
		case 7:
			sort<7>(begin);
			break;
		case 8:
			sort<8>(begin);
			break;
		default:
			assert(false);
			break;
		}
#endif
	}
}