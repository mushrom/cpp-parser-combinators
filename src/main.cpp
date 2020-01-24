#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>
#include <p_comb/ebnfish.hpp>
#include <map>

using namespace p_comb;

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

cparser load_parser(const char *fname) {
	FILE *file = fopen(fname, "r");

	if (!file) {
		throw "asdf";
	}

	struct result meh = ebnfish(make_fstream(file));

	if (!meh.matched) {
		debug_trace(meh);
		fclose(file);
		throw "asdf foo";
	}

	fclose(file);
	return compile_parser(meh.tokens);
}

int main(int argc, char *argv[]) {
	const char *fname = (argc > 1)? argv[1] : "data/test.par";

	cparser p = load_parser(fname);
	auto meh = p["main"](make_fstream(stdin));

	if (meh.matched) {
		dump_tokens(meh.tokens);

	} else {
		debug_trace(meh);
		std::cerr << "didn't match." << std::endl;
		return 1;
	}

	return 0;
}
