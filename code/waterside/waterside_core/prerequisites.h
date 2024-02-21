#pragma once

#include <cassert>

#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <deque>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <valarray>
#include <tuple>
#include <variant>
#include <bitset>
#include <complex>
#include <bit>

#include <limits>
#include <algorithm>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <new>
#include <numeric>
#include <random>
#include <regex>
#include <sstream>
#include <type_traits>

#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#include <filesystem>
#include <source_location>
#include <concepts>

//using namespace std::literals;
using namespace std::literals::string_view_literals;


namespace waterside
{
	using std::string;
	using std::string_view;
	using std::vector;
	using std::deque;
	using std::list;
	using std::map;
	using std::multimap;
	using std::set;
	using std::multiset;
	using std::unordered_map;
	using std::unordered_multimap;
	using std::unordered_set;
	using std::unordered_multiset;
	using std::stack;
	using std::queue;
	using std::priority_queue;

	struct string_hash
	{
		using is_transparent = void;

		using hash_type = std::hash<string_view>;
		std::size_t operator()(string_view str) const { return hash_type{}(str); }
		std::size_t operator()(const string& str) const { return hash_type{}(str); }
		std::size_t operator()(const char* str) const { return hash_type{}(str); }
	};

	struct case_insensitive_string_hash
	{
		using is_transparent = void;

		std::size_t operator()(string_view str) const;
		std::size_t operator()(const string& str) const
		{
			return operator()(string_view{ str });
		}
		std::size_t operator()(const char* str) const
		{
			return operator()(string_view{ str });
		}
	};

	struct case_insensitive_string_equal_to
	{
		using is_transparent = void;

		bool operator()(string_view lhs, string_view rhs) const;
		bool operator()(const string& lhs, const string& rhs) const
		{
			return operator()(string_view{ lhs }, string_view{ rhs });
		}
		bool operator()(const char* lhs, const char* rhs) const
		{
			return operator()(string_view{ lhs }, string_view{ rhs });
		}
	};

	template <typename T>
	using string_map = unordered_map<string, T, string_hash, std::equal_to<>>;

	template <typename T>
	using istring_map = unordered_map<string, T, case_insensitive_string_hash, case_insensitive_string_equal_to>;

	using string_set = unordered_set<string, string_hash, std::equal_to<>>;

	using istring_set = unordered_set<string, case_insensitive_string_hash, case_insensitive_string_equal_to>;


	typedef int64_t unique_id;
	typedef uint32_t SessionID;


	template <typename T>
	class TLazySingleton
	{
		TLazySingleton(const TLazySingleton&) = delete;
		const TLazySingleton& operator =(const TLazySingleton&) = delete;
	public:
		TLazySingleton() = default;
		~TLazySingleton() = default;

		// 得到唯一实例
		inline static T& instance()
		{
			static T msInstance;
			return msInstance;
		}
	};
}
