#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>
#include <p_comb/ebnfish.hpp>
#include <map>

using namespace p_comb;

// little expression test parser
parser op = string_parser("+") | "-" | "*" | "/";
parser identifier = tag("identifier", letter + zero_or_more(letter | digit | "_" | "-"));

struct result expression(autolist<char>::ptr ptr) {
	static const parser temp = 
		  ("(" >> (parser)expression >> ")")
		| (number >> op >> expression)
		| number;

	return tag("expression", temp)(ptr);
}

parser expression_list =
	tag("expression-list",
		"[" >> zero_or_more((parser)expression
				>> zero_or_one(string_parser(",")
					+ ignore_whitespace))
			>> "]");

parser assignment = tag("assignment",
	whitewrap(identifier >> "=" >> expression_list >> ";"));

// SGF parser
parser prop_string = tag("string",
	one_or_more(string_parser("\\]") | "\\\\" | "\\:" | blacklist("]")));

parser prop_real = tag("real",
	(one_or_more(digit) + "." + one_or_more(digit))
	| (one_or_more(digit) + ".")
	| ("." + one_or_more(digit)));

parser prop_stone = tag("stone", letter + letter);

parser prop_value = tag("prop-value",
	whitewrap(
		("[" >>  prop_real >> "]")
		| ("[" >>  number >> "]")
		| ("[" >>  prop_stone >> "]")
		| ("[" >>  prop_string >> "]")
		// todo: "None" type
	));

parser prop_ident = tag("prop-ident", whitewrap(one_or_more(letter)));
parser property = tag("property", whitewrap(prop_ident >> one_or_more(prop_value)));

parser node = tag("node",
		whitewrap(";" >> zero_or_more(property)));

parser sequence = tag("sequence", one_or_more(node));

struct result game_tree(autolist<char>::ptr ptr) {
	parser temp = whitewrap("(" >> sequence >> zero_or_more(game_tree) >> ")");

	return tag("game-tree", temp)(ptr);
}

parser SGF_collection = tag("collection", one_or_more(game_tree));

void dump_tokens(std::list<token>& tokens, unsigned indent = 0) {
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

		dump_tokens(tok.tokens, indent + 1);
	}
}

void debug_trace(struct result& res) {
	for (auto& dbg : res.debug) {
		printf(" => ");
		auto temp = dbg;

		for (unsigned i = 0; i < 30 && temp; i++, temp = temp->next()) {
			if (temp->data == '\n') {
				printf("(\\n)");
			} else {
				printf("%c", temp->data);
			}
		}

		printf("\n");
	}
}

int main(int argc, char *argv[]) {
	FILE *fp = fopen((argc > 1)? argv[1] : "data/test.par", "r");
	//struct result meh = ebnfish(make_fstream(stdin));
	struct result meh = ebnfish(make_fstream(fp));
	fclose(fp);

	dump_tokens(meh.tokens);

	if (meh.matched) {
		puts("successfully matched.");
		puts("compiling...");

		cparser p = compile_parser(meh.tokens);
		struct result yo = p["main"](make_fstream(stdin));
		dump_tokens(yo.tokens);

		if (yo.matched) {
			puts("hey it works");

		} else {
			puts("nop sorry.");
		}

	} else {
		puts("didn't match.");
		puts("debug stack:");
		debug_trace(meh);
	}

	return 0;
}
