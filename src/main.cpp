#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>
#include <p_comb/ebnfish.hpp>
#include <map>
#include <sstream>
#include <fstream>

using namespace p_comb;

void dump_tokens(container& tokens, unsigned indent = 0) {
	for (auto& tok : tokens) {
		for (unsigned i = 0; i < indent; i++) {
			std::cout << "   :";
		}

		//printf("token %u : \"%s\" ('%c')\n", tok.data, tok.tag.c_str(), tok.data);
		if (tok.tag.size() > 0) {
			printf("\"%s\"\n", tok.tag.c_str());

		} else {
			//printf("'%c'\n", tok.data);
			std::cout << "'" << tok.get() << "'" << '\n';
			//printf("'%s'\n", tok.get().c_str());
		}

		if (tok.subtokens) {
			dump_tokens(*tok.subtokens, indent + 1);
		}
	}
}

void debug_trace(struct result& res) {
	/*
	for (auto& dbg : res.debug) {
		std::cout << " => " << dbg << "\n";
	}
	*/
}

std::string read_file(const char *filename) {
	std::ifstream f(filename);
	std::stringstream strm;

	strm << f.rdbuf();
	return strm.str();
}

cparser load_parser(const char *fname) {
	std::string buf = read_file(fname);
	//struct result meh = ebnfish(buf);
	auto meh = evaluate(ebnfish, buf);

	dump_tokens(meh.tokens);
	if (!meh.matched) {
		throw "asdf foo";
	}

	return compile_parser(meh.tokens);
}

parser numberparse(void) {
	return zero_or_one(string_parser("+") | "-")
		+ one_or_more(codepoint_range('0', '9'));
}

int main(int argc, char *argv[]) {
	const char *fname = (argc > 1)? argv[1] : "data/test.par";

	std::string data = read_file("/dev/stdin");
	std::string_view foo = data;
	cparser p = load_parser(fname);
	//auto meh = p["main"](foo);
	auto meh = evaluate(p["main"], foo);
	//auto meh = evaluate(tag("asdf", numberparse()), foo);

	if (meh.matched) {
		std::cerr << "matched:" << std::endl;
		dump_tokens(meh.tokens);

	} else {
		//debug_trace(meh);
		std::cerr << "didn't match." << std::endl;
		return 1;
	}

	return 0;
}
