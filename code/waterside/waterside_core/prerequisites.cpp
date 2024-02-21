#include "prerequisites.h"

namespace waterside
{
	std::size_t case_insensitive_string_hash::operator()(string_view str) const
	{
		// BKDR Hash
		size_t hash = 0;
		for (auto ch : str)
		{
			hash = hash * 131 + ::tolower((uint8_t)ch);
		}
		return hash;
	}

	bool case_insensitive_string_equal_to::operator()(string_view lhs, string_view rhs) const
	{
		size_t n = lhs.size();
		if (n != rhs.size())
		{
			return false;
		}
		for (size_t i = 0; i < n; i++)
		{
			if (::tolower((uint8_t)lhs[i]) != ::tolower((uint8_t)rhs[i]))
			{
				return false;
			}
		}
		return true;
	}
}