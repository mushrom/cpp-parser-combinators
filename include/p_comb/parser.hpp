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

typedef std::function<struct result (autolist<token>::ptr)> parser;

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
		ret.tokens = first.tokens->join(second.tokens);
		ret.matched = first.matched && second.matched;

		return ret;
	};
}

parser operator|(parser a, parser b) {
	return [=] (autolist<token>::ptr ptr) {
		if (!ptr) return (struct result){};

		struct result foo = a(ptr);

		if (foo.matched) {
			return foo;
		}

		return b(ptr);
	};
}

// namespace p_comb
}
