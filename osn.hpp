#pragma once
#include <iterator>



namespace osn
{
	namespace detail
	{
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
			using T = typename std::iterator_traits<I>::value_type;

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
				{ 1, 3 },
				{ 0, 2 },
				{ 1, 2 }
			};

			static constexpr size_t depth = 4;
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
				{ 0, 1 }, { 2, 3 }, { 4, 5 },
				{ 0, 2 }, { 3, 5 },
				{ 2, 4 },
				{ 0, 1 }, { 2, 3 }, { 4, 5 },
				{ 1, 2 }, { 3, 4 },
				{ 2, 3 }
			};

			static constexpr size_t depth = 6;
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
				{ 0, 7 }, { 1, 6 }, { 2, 5 }, { 3, 4 },
				{ 0, 3 }, { 1, 2 }, { 4, 7 }, { 5, 6 },
				{ 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 },
				{ 3, 5 }, { 2, 4 },
				{ 1, 2 }, { 3, 4 }, { 5, 6 },
				{ 2, 3 }, { 4, 5 }
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
		static_assert(N <= max_supported_size);

		if constexpr (N > 1)
		{
			using detail::index_pair;
			using network_info = detail::network_info<N>;

			for (index_pair ip : network_info::swap_sequence)
			{
				detail::compare_swap(begin + ip.left, begin + ip.right);
			}
		}
	}

	template <typename RandomAccessIterator>
	constexpr void sort(RandomAccessIterator begin, RandomAccessIterator end)
	{
		auto size = std::distance(begin, end);
		if (size < 2)
			return;
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
			break;
		}
	}
}