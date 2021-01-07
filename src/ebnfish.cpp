#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>
#include <p_comb/ebnfish.hpp>
#include <map>
#include <assert.h>

namespace p_comb {

parser identifier = tag("identifier", letter + zero_or_more(letter | digit | "_" | "-"));

// ebnf-ish parser parser
parser ebnfish_string = tag("ebnfish-string",
	ignore(string_parser("\""))
	+ one_or_more(string_parser("\\\"") | "\\\\" | "\\:" | blacklist("\""))
	+ ignore(string_parser("\"")));

parser ebnfish_value = whitewrap(
	identifier
	| ebnfish_string);

struct result ebnfish_expr(autolist<int32_t>::ptr ptr);

parser ebnfish_expr_list = one_or_more((parser)ebnfish_expr);

parser ebnfish_compound_expr =
	// regular compound (a) = a
	("(" >> ebnfish_expr_list >> ")")
	// one or more repetitions, {a} = a...
	| ("{" >> ebnfish_expr_list >> "}")
	// ignores surrounding whitespace
	| ("<" >> ebnfish_expr_list >> ">")
	// zero or one matches, [a] = a|empty.
	| ("[" >> ebnfish_expr_list >> "]")
	// any number of matches, greedy
	| ("+" >> ebnfish_expr_list >> "]");

parser ebnfish_blacklist_expr = tag("ebnfish-blacklist",
	("![" + one_or_more("\\]" | blacklist("]")) + "]"));

struct result ebnfish_expr(autolist<int32_t>::ptr ptr) {
	static const parser temp = whitewrap(
		((ebnfish_compound_expr | ebnfish_value) >> "|" >> ebnfish_expr)
		| ebnfish_compound_expr
		| ebnfish_blacklist_expr
		| ebnfish_value
	);

	return tag("ebnfish-expr", temp)(ptr);
}

struct result end_of_stream(autolist<int32_t>::ptr ptr) {
	if (ptr) {
		return RESULT_NO_MATCH;
	}

	return (struct result){ ptr, {}, true, {}};
}

parser ebnfish_comment = tag("ebnfish-comment",
	"#" + zero_or_more(blacklist("\n")) + "\n");

parser ebnfish_assign_op = tag("ebnfish-assign-op",
	zero_or_one(string_parser("*"))
	+ zero_or_one(string_parser(":"))
	+ "=");

parser ebnfish_rule = tag("ebnfish-rule", whitewrap(
	(identifier >> ebnfish_assign_op >> ebnfish_expr_list >> ";")
	| ebnfish_comment));

parser ebnfish = tag("ebnfish", one_or_more(ebnfish_rule) + end_of_stream);
// end of grammar

// compiler

// TODO: more efficient
std::string unescape(std::string str) {
	std::string ret = "";

	for (unsigned i = 0; i < str.size(); i++) {
		if (str[i] == '\\') {
			int32_t esc = str[++i];

			// TODO: handle unicode escapes
			switch (esc) {
				case 'n': ret += '\n'; break;
				case 't': ret += '\t'; break;
				case 'r': ret += '\r'; break;
				case 'a': ret += '\a'; break;
				case 'v': ret += '\v'; break;

				default: ret += esc; break;
			}
		}

		else {
			ret += str[i];
		}
	}

	return ret;
}

std::string collect(token::container& tokens) {
	std::string ret = "";

	for (auto tok : tokens) {
		if (tok.tag == "string-literal") {
			ret += unescape(collect(tok.tokens));
		}

		// handle base characters
		if (tok.tag.size() == 0) {
			ret += tok.data;
		}
	}

	return ret;
}

struct result error_and_abort(autolist<int32_t>::ptr ptr) {
	std::cerr << "ERROR: invalid state!" << std::endl;
	std::cerr << "       something something compiler bug..." << std::endl;

	return (struct result){ptr, {}, false, {ptr}};
}

struct result uninitialized_abort(autolist<int32_t>::ptr ptr) {
	std::cerr << "ERROR: uninitialized rule!" << std::endl;
	std::cerr << "       something something compiler bug..." << std::endl;

	return (struct result){ptr, {}, false, {ptr}};
}

void initialize_names(cparser& ret, token::container& tokens) {
	assert(tokens.front().tag == "ebnfish");

	for (auto& ruletok : tokens.front().tokens) {
		assert(ruletok.tag == "ebnfish-rule");
		std::string& tag = ruletok.tokens.front().tag;

		if (tag == "identifier") {
			std::string s = collect(ruletok.tokens.front().tokens);

			// initialize to a dummy rule to begin with 
			ret[s] = uninitialized_abort;
		}
	}
}

parser compile_expressions(cparser& ret,
                           token::container::iterator it,
                           token::container::iterator end);
parser compile_expression(cparser& ret,
                          token::container::iterator it,
                          token::container::iterator end);

parser compile_expression(cparser& ret,
                          token::container::iterator it,
                          token::container::iterator end)
{
	if (it->tag == "identifier") {
		std::string id = collect(it->tokens);

		if (ret.find(id) == ret.end()) {
			std::cerr << "WARNING: reference to undefined rule \""
			          << id << "\"" << std::endl;
		}

		return [=, &ret] (autolist<int32_t>::ptr ptr) {
			auto foo = ret.find(id);

			if (foo != ret.end()) {
				return foo->second(ptr);
			}

			// passive-aggressive errors for maximum effect
			std::cerr << "ERROR: hey remember that warning about "
			          << "``undefined rule \""
			          << id << "\"''?" << std::endl;

			std::cerr << "       well yeah now we've reached that rule "
			          << "and it's still undefined"
			          << std::endl;

			std::cerr << "       so there's not much I can do here, ok?"
			          << std::endl;

			return RESULT_NO_MATCH;
		};
	}

	if (it->tag == "ebnfish-string") {
		std::string lit = collect(it->tokens);
		return string_parser(lit);
	}

	if (it->tag == "ebnfish-expr") {
		parser temp = compile_expressions(ret,
		                                  it->tokens.begin(),
		                                  it->tokens.end());

		switch (it->tokens.front().data) {
			case '{': temp = one_or_more(temp); break;
			case '[': temp = zero_or_one(temp); break;
			case '+': temp = zero_or_more(temp); break;
			case '<': temp = whitewrap(temp);   break;
			default: break;
		}

		return temp;
	}

	if (it->tag == "ebnfish-blacklist") {
		token::container meh(std::next(it->tokens.begin()),
		                     std::prev(it->tokens.end()));
		std::string chrs = unescape(collect(meh));

		std::cerr << "have blacklist with " << chrs << std::endl;
		// TODO:  maybe it should be a string blacklist, rather than
		//        a character blacklist
		return blacklist(chrs);
	}

	std::cerr << "WARNING: unknown state at " << it->tag << ", "
	          << it->data << std::endl;

	// TODO: we should probably return an error before we
	//       start parsing...
	return error_and_abort;
}

parser compile_expressions(cparser& ret,
                           token::container::iterator it,
                           token::container::iterator end)
{
	parser p = nullptr;

	// symbols that indicate the end of an expression
	for (; it != end; it++) {
		if (it->data == ';' || it->data == ')'
		    || it->data == '}' || it->data == ']'
		    || it->data == '<' || it->data == '>'
			|| it->data == '{' || it->data == '('
			|| it->data == '[' || it->data == '+')
		{
			// ignore terminal characters
			// TODO: string index instead of a bunch of conditions
			continue;
		}

		if (p == nullptr) {
			p = compile_expression(ret, it, end);

		} else if (it->data == '|') {
			p = p | compile_expression(ret, ++it, end);

		} else {
			p = p + compile_expression(ret, it, end);
		}
	}

	// XXX:
	p = (p == nullptr)? error_and_abort : p;
	return p;
}

void compile_rules(cparser& ret, token::container& tokens) {
	for (auto& ruletok : tokens.front().tokens) {
		std::string& toktag = ruletok.tokens.front().tag;

		if (toktag == "identifier") {
			std::string id = collect(ruletok.tokens.front().tokens);
			auto it = ruletok.tokens.begin();
			it++; 
			auto assign_type = it++;

			parser p = compile_expressions(ret, it, ruletok.tokens.end());

			for (auto& tok : assign_type->tokens) {
				switch (tok.data) {
					case '*':
						p = whitewrap(p);
						break;
					case ':':
						p = tag(id, p);
						break;
					default:
						break;
				}
			}

			ret[id] = p;
		}
	}
}

// initialial set of base parsers that are already defined,
// so that we don't have to redefine common rules in every
// grammar file
//
// (also leaves open the option of optimizing lower-level parsers)
static std::map<std::string, parser> builtin_parsers = {
	{ "digit", digit },
	{ "lowercase", lowercase },
	{ "uppercase", uppercase },
	{ "letter", letter },
	{ "whitespace_char", whitespace_char },
	{ "whitespace", whitespace },
	{ "identifier", identifier },
	{ "number", number },
	{ "EOF", end_of_stream },
};

cparser compile_parser(token::container& tokens) {
	cparser ret = builtin_parsers;
	initialize_names(ret, tokens);
	compile_rules(ret, tokens);

	return ret;
}

// namespace p_comb
}
