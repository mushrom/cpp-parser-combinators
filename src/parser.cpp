#include <p_comb/parser.hpp>

namespace p_comb {

int32_t next_char(std::string_view v) {
	return (v.empty())? -1 : v[0];
}

std::string_view increment(std::string_view v) {
	return (v.empty())? v : v.substr(1);
}

parser one_or_more(parser p) {
	return [=] (std::string_view ptr, parserState& state) {
		result res;

		std::string_view end = ptr;
		std::string_view last = ptr;
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
	return [=] (std::string_view ptr, parserState& state) {
		result res;

		std::string_view end = ptr;
		std::string_view last = ptr;
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
	return [=] (std::string_view ptr, parserState& state) {
		if (ptr.empty()) {
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
	return [=] (std::string_view ptr, parserState& state) {
		auto saved = state.capture();
		result foo = p(ptr, state);

		// XXX: restore to erase any added tokens from the parser,
		//      would be more efficient to not add them to begin with
		state.restore(saved);

		// return result info, except for any returned tokens.
		return foo;
	};
}

parser tag(std::string type, parser p) {
	return [=] (std::string_view ptr, parserState& state) {
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
		return [=] (std::string_view ptr, parserState& state) {
			return result {ptr, true};
		};
	}

	return [=] (std::string_view ptr, parserState& state) {
		auto temp = ptr;

		for (unsigned i = 0; i < str.size(); i++) {
			if (temp.empty() || next_char(temp) != str[i]) {
				return result {temp, false};
			}

			temp = increment(temp);
		}

		// successfully matched
		if (str.size() > 1) {
			// avoid cluttering the token tree with single-character
			// string lists, when we just about always want the character itself
			// to be a single return token
			state.pushTag("string-literal");

			// XXX
			for (unsigned i = 0; i < str.size(); i++) {
				state.pushToken({ .data = str[i] });
			}

			state.popTag();
			return result {temp, true};
		}


		else {
			state.pushToken({ .data = str[0] });
			return result {temp, true};
		}
	};
}

parser codepoint_range(int32_t start, int32_t end) {
	return [=] (std::string_view ptr, parserState& state) {
		if (ptr.empty()) {
			return RESULT_NO_MATCH;
		}

		int32_t data = next_char(ptr);

		if (data < start || data > end) {
			return result {ptr, false};
		}

		state.pushToken({ .data = data });

		return result {increment(ptr), true};
	};
}

parser blacklist(std::string blacklist) {
	return [=] (std::string_view ptr, parserState& state) {
		if (ptr.empty()) {
			return RESULT_NO_MATCH;
		}

		int32_t data = next_char(ptr);

		for (int32_t c : blacklist) {
			if (c == data) {
				return result {ptr, false};
			}
		}

		state.pushToken({ .data = data });
		return result { increment(ptr), true, };
	};
}

parser whitewrap(parser a) {
	return ignore_whitespace + (a + ignore_whitespace);
}

parser operator+(parser a, parser b) {
	return [=] (std::string_view ptr, parserState& state) {
		result first, second, ret;

		if (ptr.empty()) {
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
	return [=] (std::string_view ptr, parserState& state) {
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

void dump_tokens_tree(container& tokens, unsigned indent) {
	for (auto& tok : tokens) {
		for (unsigned i = 0; i < indent; i++) {
			std::cout << "   :";
		}

		//printf("token %u : \"%s\" ('%c')\n", tok.data, tok.tag.c_str(), tok.data);
		if (tok.tag.size() > 0) {
			printf("\"%s\"\n", tok.tag.c_str());

		} else {
			printf("'%c'\n", tok.data);
		}

		dump_tokens_tree(tok.subtokens, indent + 1);
	}
}

// output eseentially S-exps but as valid json
void dump_tokens_json(container& tokens, unsigned indent) {
	printf("[");
	for (auto it = tokens.begin(); it != tokens.end(); it++) {
		auto tok = *it;

		if (tok.tag.size() > 0) {
			printf("[");
			printf("\"%s\"", tok.tag.c_str());
			printf(",");

			if (!tok.subtokens.empty()) {
				dump_tokens_json(tok.subtokens, indent + 1);
			} else {
				printf("null");
			}
			printf("]");

		} else {
			if (tok.data) {
				// assume there's no lower-level data
				printf(" \"%c\"", tok.data);
			}
		}


		if (it + 1 != tokens.end()) {
			printf(",");
		}
	}
	printf("]");
}

void dump_tokens_sexps(container& tokens, unsigned indent) {
	printf("(");
	for (auto it = tokens.begin(); it != tokens.end(); it++) {
		auto tok = *it;

		if (tok.tag.size() > 0) {
			printf("(");
			printf("%s", tok.tag.c_str());
			printf(" . ");

			if (!tok.subtokens.empty()) {
				dump_tokens_sexps(tok.subtokens, indent + 1);
			} else {
				printf("'()");
			}
			printf(")");

		} else {
			if (tok.data) {
				// assume there's no lower-level data
				printf(" \"%c\"", tok.data);
			}
		}
	}
	printf(")");
}

// namespace p_comb
}
