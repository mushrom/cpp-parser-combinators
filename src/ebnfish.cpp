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

struct result ebnfish_expr(autolist<char>::ptr ptr);

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
	;

struct result ebnfish_expr(autolist<char>::ptr ptr) {
	static const parser temp = whitewrap(
		((ebnfish_compound_expr | ebnfish_value) >> "|" >> ebnfish_expr)
		| ebnfish_compound_expr
		| ebnfish_value
	);

	return tag("ebnfish-expr", temp)(ptr);
}

struct result end_of_stream(autolist<char>::ptr ptr) {
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
std::string collect(std::list<token>& tokens) {
	std::string ret = "";

	for (auto tok : tokens) {
		// only add untagged data, (base characters)
		if (tok.tag.size() == 0) {
			ret += tok.data;
		}
	}

	return ret;
}

struct result error_and_abort(autolist<char>::ptr ptr) {
	std::cerr << "ERROR: invalid state!" << std::endl;
	std::cerr << "       something something compiler bug..." << std::endl;

	return (struct result){ptr, {}, false, {ptr}};
}

struct result uninitialized_abort(autolist<char>::ptr ptr) {
	std::cerr << "ERROR: uninitialized rule!" << std::endl;
	std::cerr << "       something something compiler bug..." << std::endl;

	return (struct result){ptr, {}, false, {ptr}};
}

void initialize_names(cparser& ret, std::list<token>& tokens) {
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
                           std::list<token>::iterator it,
                           std::list<token>::iterator end);
parser compile_expression(cparser& ret,
                          std::list<token>::iterator it,
                          std::list<token>::iterator end);

parser compile_expression(cparser& ret,
                          std::list<token>::iterator it,
                          std::list<token>::iterator end)
{
	if (it->tag == "identifier") {
		std::string id = collect(it->tokens);

		if (ret.find(id) == ret.end()) {
			std::cerr << "WARNING: reference to undefined rule \""
			          << id << "\"" << std::endl;
		}

		// XXX TODO remove
		if (id == "identifier") {
			return identifier;
		}

		return [=, &ret] (autolist<char>::ptr ptr) {
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
		if (it->tokens.front().data == '{') {
			return one_or_more(
				compile_expressions(ret,
				                    it->tokens.begin(),
				                    it->tokens.end()));
		}

		if (it->tokens.front().data == '[') {
			return zero_or_one(
				compile_expressions(ret,
				                    it->tokens.begin(),
				                    it->tokens.end()));
		}

		if (it->tokens.front().data == '<') {
			return whitewrap(
				compile_expressions(ret,
				                    it->tokens.begin(),
				                    it->tokens.end()));
		}

		return compile_expressions(ret, it->tokens.begin(),
		                           it->tokens.end());
	}

	std::cerr << "WARNING: unknown state at " << it->tag << ", " << it->data << std::endl;
	// TODO: we should probably return an error before we
	//       start parsing...
	return error_and_abort;
}

parser compile_expressions(cparser& ret,
                           std::list<token>::iterator it,
                           std::list<token>::iterator end)
{
	parser p = nullptr;

	// symbols that indicate the end of an expression
	for (; it != end; it++) {
		if (it->data == ';' || it->data == ')'
		    || it->data == '}' || it->data == ']'
		    || it->data == '<' || it->data == '>'
			|| it->data == '{' || it->data == '('
			|| it->data == '[')
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

void compile_rules(cparser& ret, std::list<token>& tokens) {
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

cparser compile_parser(std::list<token>& tokens) {
	cparser ret;
	initialize_names(ret, tokens);
	compile_rules(ret, tokens);

	return ret;
}

// namespace p_comb
}
