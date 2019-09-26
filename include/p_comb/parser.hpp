#pragma once

#include <p_comb/autolist.hpp>
#include <p_comb/tokenizer.hpp>
#include <functional>

namespace p_comb {

struct result {
	autolist<token>::ptr next;
	autolist<token>::ptr tokens;
	bool matched = false;
};

#define RESULT_NO_MATCH ((struct result) { nullptr, nullptr, false, })

typedef std::function<struct result (autolist<token>::ptr)> parser;

parser make_string_const_parser(std::string str) {
	return [=] (autolist<token>::ptr ptr) {
		auto temp = ptr;

		for (unsigned i = 0; i < str.size(); i++) {
			if (!temp || temp->data != str[i]) {
				return RESULT_NO_MATCH;
			}

			temp = temp->next();
		}

		return (struct result) {
			temp,
			ptr->take(str.size()),
			true,
		};
	};
}

parser operator+(parser a, parser b) {
	return [=] (autolist<token>::ptr ptr) {
		struct result first, second, ret;

		if (!ptr) return ret;

		first = a(ptr);

		if (!first.matched) {
			return ret;
		}

		second = b(first.next);
		ret.next = second.next;
		ret.tokens = first.tokens + second.tokens;
		ret.matched = first.matched && second.matched;

		return ret;
	};
}

parser operator|(parser a, parser b) {
	return [=] (autolist<token>::ptr ptr) {
		struct result foo = a(ptr);

		if (foo.matched) {
			return foo;
		}

		return b(ptr);
	};
}

parser operator+(std::string a, parser b) {
	return make_string_const_parser(a) + b;
}

parser operator+(parser a, std::string b) {
	return a + make_string_const_parser(b);
}

// namespace p_comb
}
