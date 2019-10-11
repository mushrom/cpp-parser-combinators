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
