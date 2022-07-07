#include <p_comb/parser.hpp>

namespace p_comb {

bool empty(const viewPair& v) {
	return v.first == v.second;
}

int32_t next_char(const viewPair& v) {
	return empty(v)? -1 : *v.first;
}

viewPair increment(const viewPair& v) {
	return empty(v)? v : viewPair {v.first + 1, v.second};
}

parser one_or_more(parser p) {
	return [=] (viewPair ptr, parserState& state) {
		result res;

		//std::string_view end = ptr;
		//std::string_view last = ptr;
		viewPair end = ptr;
		viewPair last = ptr;
		unsigned i = 0;

		do {
			auto saved = state.capture();

			last = end;
			res = p(end, state);
			end = res.next;
			i += res.matched;

			if (!res.matched) {
				state.restore(saved);
			}
		} while (res.matched);

		// TODO: wrap return token stream, so we can actually
		//       get tokens returned when parsing
		if (i >= 1) {
			return result {last, true};
		}

		// if we get here, we failed on the first match, so we can
		// reuse the result struct here since the 'last' pointer should
		// still be the same
		return res;
	};
}

parser zero_or_more(parser p) {
	return [=] (viewPair ptr, parserState& state) {
		result res;

		viewPair end = ptr;
		viewPair last = ptr;
		unsigned i = 0;

		do {
			auto saved = state.capture();

			last = end;
			res = p(end, state);
			end = res.next;
			i += res.matched;

			if (!res.matched) {
				state.restore(saved);
			}

		} while (res.matched);

		return result {last, true};
	};
}

parser zero_or_one(parser p) {
	return [=] (viewPair ptr, parserState& state) {
		if (empty(ptr)) {
			return RESULT_NO_MATCH;
		}

		auto saved = state.capture();
		result res = p(ptr, state);

		if (res.matched) {
			return res;

		} else {
			state.restore(saved);
			return result {ptr, true};
		}
	};
}

parser ignore(parser p) {
	return [=] (viewPair ptr, parserState& state) {
		bool temp = state.ignoring;
		//auto saved = state.capture();
		state.ignoring = true;
		result foo = p(ptr, state);
		state.ignoring = temp;

		// XXX: restore to erase any added tokens from the parser,
		//      would be more efficient to not add them to begin with
		//state.restore(saved);

		// return result info, except for any returned tokens.
		return foo;
	};
}

parser tag(std::string type, parser p) {
	return [=] (viewPair ptr, parserState& state) {
		state.pushTag(type);
		result foo = p(ptr, state);

		if (foo.matched) {
			state.popTag();
		} else {
			state.discardTag();
		}

		return foo;
	};
}

parser string_parser(std::string str) {
	// special case for empty strings, always returns true (just a no-op)
	if (str.size() == 0) {
		return [=] (viewPair ptr, parserState& state) {
			return result {ptr, true};
		};
	}

	return [=] (viewPair ptr, parserState& state) {
		auto temp = ptr;

		for (unsigned i = 0; i < str.size(); i++) {
			if (empty(temp) || next_char(temp) != str[i]) {
				return result {temp, false};
			}

			temp = increment(temp);
		}

		// successfully matched
		for (unsigned i = 0; i < str.size(); i++) {
			state.pushToken(ptr.first + i);
		}

		return result {temp, true};
	};
}

parser codepoint_range(int32_t start, int32_t end) {
	return [=] (viewPair ptr, parserState& state) {
		if (empty(ptr)) {
			return RESULT_NO_MATCH;
		}

		int32_t data = next_char(ptr);

		if (data < start || data > end) {
			return result {ptr, false};
		}

		state.pushToken(ptr.first);
		//state.pushToken({ .data = data });

		return result {increment(ptr), true};
	};
}

parser blacklist(std::string blacklist) {
	return [=] (viewPair ptr, parserState& state) {
		if (empty(ptr)) {
			return RESULT_NO_MATCH;
		}

		int32_t data = next_char(ptr);

		for (int32_t c : blacklist) {
			if (c == data) {
				return result {ptr, false};
			}
		}

		//state.pushToken({ .data = data });
		state.pushToken(ptr.first);
		return result { increment(ptr), true, };
	};
}

parser whitewrap(parser a) {
	return ignore_whitespace + (a + ignore_whitespace);
}

parser operator+(parser a, parser b) {
	return [=] (viewPair ptr, parserState& state) {
		result first, second, ret;

		if (empty(ptr)) {
			return ret;
		}

		auto saved = state.capture();
		first = a(ptr, state);

		if (!first.matched) {
			state.restore(saved);
			return first;
		}

		second = b(first.next, state);
		ret.next = second.next;
		ret.matched = first.matched && second.matched;

		if (!ret.matched) {
			state.restore(saved);
		}

		return ret;
	};
}

parser operator|(parser a, parser b) {
	return [=] (viewPair ptr, parserState& state) {
		auto saved = state.capture();
		result foo = a(ptr, state);

		if (foo.matched) {
			return foo;
		}

		state.restore(saved);
		auto bar = b(ptr, state);

		if (!bar.matched) {
			state.restore(saved);
		}

		return bar;
	};
}

parser operator+(std::string a, parser b) {
	return string_parser(a) + b;
}

parser operator+(parser a, std::string b) {
	return a + string_parser(b);
}

parser operator|(std::string a, parser b) {
	return string_parser(a) | b;
}

parser operator|(parser a, std::string b) {
	return a | string_parser(b);
}

// use >> as a whitespace-ignoring concatenative operator
parser operator>>(parser a, parser b) {
	return a + ignore_whitespace + b;
}

parser operator>>(std::string a, parser b) {
	return string_parser(a) + ignore_whitespace + b;
}

parser operator>>(parser a, std::string b) {
	return a + ignore_whitespace + string_parser(b);
}

// namespace p_comb
}
