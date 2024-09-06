#ifndef DODD_STREAM_H
#define DODD_STREAM_H

#include <algorithm>
#include <iterator>
#include <utility>
#include <type_traits>

#define EXPAND(...) __VA_ARGS__

#define FORWARD(X) std::forward<decltype(X)>(X)

#define STD_ITERATOR(TYPE, MEMBER, ...)\
template<__VA_ARGS__>\
auto MEMBER(dodd::TYPE const& stream) {\
	return stream.MEMBER();\
}\

#define STD_METHODS(TYPE, ...)\
namespace std {\
	STD_ITERATOR(TYPE, begin, __VA_ARGS__)\
	STD_ITERATOR(TYPE, end, __VA_ARGS__)\
	STD_ITERATOR(TYPE, cbegin, __VA_ARGS__)\
	STD_ITERATOR(TYPE, cend, __VA_ARGS__)\
}\

#define NOT_EQUAL(TYPE)\
bool operator!=(TYPE other) const {\
	return !(*this == other);\
}\

#define STREAM_BEGIN_T(TYPE) decltype(std::declval<std::remove_cvref_t<TYPE>>().cbegin())
#define STREAM_END_T(TYPE) decltype(std::declval<std::remove_cvref_t<TYPE>>().cend())

#define STREAM_METHODS \
auto begin() {\
	return cbegin();\
}\
\
auto end() {\
	return cend();\
}\
\
template<typename F>\
auto map(F&& function) {\
	return Map<std::remove_cvref_t<decltype(*this)>, F>(std::move(*this), FORWARD(function));\
}\
\
template<typename F>\
void for_each(F&& function) {\
	auto first = cbegin();\
	auto last = cend();\
	for (; first != last; ++first) {\
		function(*first);\
	}\
}\

namespace dodd {

	template<typename ITERATOR>
	class Iter;

	template<typename STREAM, typename FUNCTION>
	class Map;

	template<typename ITERATOR>
	class Iter {
		ITERATOR first, last;

	public:
		Iter(ITERATOR begin, ITERATOR end) : first{ begin }, last{ end } {}

		auto cbegin() {
			return first;
		}

		auto cend() {
			return last;
		}

		STREAM_METHODS;
	};

	template<typename STREAM, typename FUNCTION>
	class Map {
		STREAM stream;
		FUNCTION function;

		struct Iterator {
			using Begin = STREAM_BEGIN_T(STREAM);
			using End = STREAM_END_T(STREAM);

			Begin inner;
			FUNCTION* f;

			Iterator(Begin inner, FUNCTION* f) : inner{ inner }, f{ f } {}

			bool operator==(End other) const {
				return inner == other;
			}

			NOT_EQUAL(End);

			Iterator& operator++() {
				++inner;
				return *this;
			}
			
			auto operator*() const {
				return (*f)(*inner);
			}
		};

	public:
		template<typename S, typename F>
		Map(S&& stream, F&& function) : stream{ FORWARD(stream) }, function{ FORWARD(function) } {}

		auto cbegin() {
			return Iterator(stream.cbegin(), &function);
		}

		auto cend() {
			return stream.cend();
		}

		STREAM_METHODS;
	};

	template<typename ITERATOR>
	auto stream(ITERATOR begin, ITERATOR end) {
		return Iter<ITERATOR>(begin, end);
	}

	template<typename ITERABLE>
	auto stream(ITERABLE const& iterable) {
		return stream(std::cbegin(iterable), std::cend(iterable));
	}
}

STD_METHODS(Iter<ITERATOR>, typename ITERATOR)

STD_METHODS(Map<EXPAND(STREAM, FUNCTION)>, typename STREAM, typename FUNCTION)

#endif
