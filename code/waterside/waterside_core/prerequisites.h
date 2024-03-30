#pragma once

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include <boost/asio.hpp>
#include "boost/noncopyable.hpp"
#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
#endif

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

#ifdef _MSC_VER
#	ifdef _DEBUG
#		define _CRTDBG_MAP_ALLOC
#		include <crtdbg.h>
#		define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#		define malloc(s) _malloc_dbg(s, _CLIENT_BLOCK, __FILE__, __LINE__)
#		define calloc(c, s) _calloc_dbg(c, s, _CLIENT_BLOCK, __FILE__, __LINE__)
#		define realloc(p, s) _realloc_dbg(p, s, _CLIENT_BLOCK, __FILE__, __LINE__)
#	else
#		include "tbb/tbbmalloc_proxy.h"
#		define DEBUG_NEW new
#	endif
#else
#	define DEBUG_NEW new
#endif

//using namespace std::literals;
using namespace std::literals::string_view_literals;


namespace waterside
{
	struct string_hash
	{
		using is_transparent = void;

		using hash_type = std::hash<std::string_view>;
		std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
		std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
		std::size_t operator()(const char* str) const { return hash_type{}(str); }
	};

	struct case_insensitive_string_hash
	{
		using is_transparent = void;

		std::size_t operator()(std::string_view str) const;
		std::size_t operator()(const std::string& str) const
		{
			return operator()(std::string_view{ str });
		}
		std::size_t operator()(const char* str) const
		{
			return operator()(std::string_view{ str });
		}
	};

	struct case_insensitive_string_equal_to
	{
		using is_transparent = void;

		bool operator()(std::string_view lhs, std::string_view rhs) const;
		bool operator()(const std::string& lhs, const std::string& rhs) const
		{
			return operator()(std::string_view{ lhs }, std::string_view{ rhs });
		}
		bool operator()(const char* lhs, const char* rhs) const
		{
			return operator()(std::string_view{ lhs }, std::string_view{ rhs });
		}
	};

	template <typename T>
	using string_map = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

	template <typename T>
	using istring_map = std::unordered_map<std::string, T, case_insensitive_string_hash, case_insensitive_string_equal_to>;

	using string_set = std::unordered_set<std::string, string_hash, std::equal_to<>>;

	using istring_set = std::unordered_set<std::string, case_insensitive_string_hash, case_insensitive_string_equal_to>;


	typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;

	template <class E>
	void throw_with_trace(const E& e)
	{
		throw boost::enable_error_info(e) << traced(boost::stacktrace::stacktrace());
	}

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

	template <typename T>
	class TSingleton
	{
		TSingleton(const TSingleton&) = delete;
		const TSingleton& operator =(const TSingleton&) = delete;
	public:
		TSingleton()
		{
			assert(!msInstance);
			msInstance = static_cast<T*>(this);
		}

		~TSingleton()
		{
			assert(msInstance);
			msInstance = nullptr;
		}

		// 得到唯一实例
		inline static T* instance()
		{
			return msInstance;
		}

	protected:
		static T* msInstance;
	};

	template<typename T> T* TSingleton<T>::msInstance = nullptr;
}
