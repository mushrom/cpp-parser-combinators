#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>

namespace p_comb {

parser one_or_more(parser p) {
	return [=] (autolist<char>::ptr ptr) {
		if (!ptr) {
			return RESULT_NO_MATCH;
		}

		struct result res;
		token ret;
		autolist<char>::ptr end = ptr;
		autolist<char>::ptr last = ptr;
		unsigned i = 0;

		do {
			last = end;
			res = p(end);
			end = res.next;
			i += res.matched;

			if (res.matched) {
				ret.tokens.push_back(res.tokens);
			}
		} while (res.matched);

		// TODO: wrap return token stream, so we can actually
		//       get tokens returned when parsing
		if (i >= 1) {
			return (struct result) { last, ret, true, };
		}

		return RESULT_NO_MATCH;
	};
}

parser zero_or_more(parser p) {
	return [=] (autolist<char>::ptr ptr) {
		if (!ptr) {
			return RESULT_NO_MATCH;
		}

		struct result res;
		token ret;
		autolist<char>::ptr end = ptr;
		autolist<char>::ptr last = ptr;
		unsigned i = 0;

		do {
			last = end;
			res = p(end);
			end = res.next;
			i += res.matched;

			if (res.matched) {
				ret.tokens.push_back(res.tokens);
			}
		} while (res.matched);

		return (struct result) { last, ret, true, };
	};
}

parser zero_or_one(parser p) {
	return [=] (autolist<char>::ptr ptr) {
		if (!ptr) {
			return RESULT_NO_MATCH;
		}

		struct result res = p(ptr);

		return res.matched? res : (struct result){ ptr, {{}, 0}, true};
	};
}

parser ignore(parser p) {
	return [=] (autolist<char>::ptr ptr) {
		struct result foo = p(ptr);

		// return result info, except for any returned tokens.
		return (struct result){ foo.next, {{}, 0}, foo.matched };
	};
}


parser string_parser(std::string str) {
	return [=] (autolist<char>::ptr ptr) {
		auto temp = ptr;

		for (unsigned i = 0; i < str.size(); i++) {
			if (!temp || temp->data != str[i]) {
				return RESULT_NO_MATCH;
			}

			temp = temp->next();
		}

		std::list<token> tok;

		// XXX
		for (unsigned i = 0; i < str.size(); i++) {
			tok.push_back({{}, str[i]});
		}

		// avoid cluttering the token tree with single-character
		// string lists, when we just about always want the character itself
		// to be a single return token
		if (str.size() > 1) {
			return (struct result){ temp, {tok, 's'}, true, };
		}

		else {
			return (struct result){ temp, {{}, str[0]}, true, };
		}
	};
}

parser operator+(parser a, parser b) {
	return [=] (autolist<char>::ptr ptr) {
		struct result first, second, ret;

		if (!ptr) return ret;

		first = a(ptr);

		if (!first.matched) {
			return ret;
		}

		second = b(first.next);
		ret.next = second.next;
		ret.tokens.tokens.push_back(first.tokens);
		ret.tokens.tokens.push_back(second.tokens);
		ret.matched = first.matched && second.matched;

		return ret;
	};
}

parser operator|(parser a, parser b) {
	return [=] (autolist<char>::ptr ptr) {
		struct result foo = a(ptr);

		if (foo.matched) {
			return foo;
		}

		return b(ptr);
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

// namespace p_comb
}
