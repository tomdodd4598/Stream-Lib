#ifndef DODD_STREAM_H
#define DODD_STREAM_H

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>
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
	return MapStream<std::remove_cvref_t<decltype(*this)>, F>(std::move(*this), FORWARD(function));\
}\
\
template<typename P>\
auto filter(P&& predicate) {\
	return FilterStream<std::remove_cvref_t<decltype(*this)>, P>(std::move(*this), FORWARD(predicate));\
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
\
template<typename T>\
std::vector<T> to_vector() {\
	std::vector<T> vec;\
	for_each([&vec](auto&& x) { vec.push_back(FORWARD(x)); });\
	return vec;\
}\

namespace dodd {

	template<typename ITERATOR>
	class RefStream;

	template<typename ITERABLE>
	class ValueStream;

	template<typename STREAM, typename FUNCTION>
	class MapStream;

	template<typename STREAM, typename PREDICATE>
	class FilterStream;

	template<typename ITERATOR>
	class RefStream {
		ITERATOR first, last;

	public:
		template<typename F, typename L>
		RefStream(F&& first, L&& last) : first{ FORWARD(first) }, last{ FORWARD(last) } {}

		auto cbegin() {
			return first;
		}

		auto cend() {
			return last;
		}

		STREAM_METHODS;
	};

	template<typename ITERABLE>
	class ValueStream {
		ITERABLE iterable;

	public:
		template<typename I>
		ValueStream(I&& iterable) : iterable{ FORWARD(iterable) } {}

		auto cbegin() {
			return std::cbegin(iterable);
		}

		auto cend() {
			return std::cend(iterable);
		}

		STREAM_METHODS;
	};

	template<typename STREAM, typename FUNCTION>
	class MapStream {
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
		MapStream(S&& stream, F&& function) : stream{ FORWARD(stream) }, function{ FORWARD(function) } {}

		auto cbegin() {
			return Iterator(stream.cbegin(), &function);
		}

		auto cend() {
			return stream.cend();
		}

		STREAM_METHODS;
	};

	template<typename STREAM, typename PREDICATE>
	class FilterStream {
		STREAM stream;
		PREDICATE predicate;

		struct Iterator {
			using Begin = STREAM_BEGIN_T(STREAM);
			using End = STREAM_END_T(STREAM);

			Begin inner;
			End end;
			PREDICATE* p;

			void filter() {
				while (inner != end && !(*p)(*inner)) {
					++inner;
				}
			}

			Iterator(Begin inner, End end, PREDICATE* p) : inner{ inner }, end{ end }, p{ p } {
				filter();
			}

			bool operator==(End other) const {
				return inner == other;
			}

			NOT_EQUAL(End);

			Iterator& operator++() {
				++inner;
				filter();
				return *this;
			}
			
			auto operator*() const {
				return *inner;
			}
		};

	public:
		template<typename S, typename P>
		FilterStream(S&& stream, P&& predicate) : stream{ FORWARD(stream) }, predicate{ FORWARD(predicate) } {}

		auto cbegin() {
			return Iterator(stream.cbegin(), stream.cend(), &predicate);
		}

		auto cend() {
			return stream.cend();
		}

		STREAM_METHODS;
	};

	template<typename ITERATOR>
	auto stream(ITERATOR begin, ITERATOR end) {
		return RefStream<ITERATOR>(begin, end);
	}

	template<typename ITERABLE>
	auto stream(ITERABLE const& iterable) {
		return stream(std::cbegin(iterable), std::cend(iterable));
	}

	template<typename ITERABLE, std::enable_if_t<!std::is_reference<ITERABLE>::value, int> = 0>
	auto stream(ITERABLE&& iterable) {
		return ValueStream<ITERABLE>(FORWARD(iterable));
	}
}

STD_METHODS(RefStream<ITERATOR>, typename ITERATOR)

STD_METHODS(ValueStream<ITERABLE>, typename ITERABLE)

STD_METHODS(MapStream<EXPAND(STREAM, FUNCTION)>, typename STREAM, typename FUNCTION)

STD_METHODS(FilterStream<EXPAND(STREAM, PREDICATE)>, typename STREAM, typename PREDICATE)

#endif
