#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>
#include <p_comb/ebnfish.hpp>
#include <map>
#include <sstream>
#include <fstream>
#include <cstring>

using namespace p_comb;

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

	if (!meh.matched) {
		//debug_trace(meh);
		throw "asdf foo";
	}

	return compile_parser(meh.tokens);
}

void debug_trace(struct result& res) {
	/*
	for (auto& dbg : res.debug) {
		std::cout << " => " << dbg << "\n";
	}
	*/
}


parser numberparse(std::string_view v) {
	return zero_or_one(string_parser("+") | "-")
		+ one_or_more(codepoint_range('0', '9'));
}

int main(int argc, char *argv[]) {
	enum modes {
		outputTree,
		outputJson,
		outputSexps,
	} mode;

	int argstart = 1;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			argstart = i;
			break;
		}

		switch (argv[i][1]) {
			case 'j':
				mode = outputJson;
				break;

			case 's':
				mode = outputSexps;
				break;

			case 't':
				mode = outputTree;
				break;

			case 'h':
				printf("Usage: [-jst] [parser specification]");
				break;

			case '-':
				     if (strcmp(argv[i] + 2, "json")  == 0) mode = outputJson;
				else if (strcmp(argv[i] + 2, "sexps") == 0) mode = outputSexps;
				else if (strcmp(argv[i] + 2, "tree")  == 0) mode = outputTree;
				else fprintf(stderr, "unknown option '%c' (try -h for help)\n");
				break;

			default:
				fprintf(stderr, "unknown option '%c' (try -h for help)\n", argv[i][1]);
				break;
		}
	}

	if (argstart >= argc) {
		printf("Missing parser specification (try -h for help)");
		return 1;
	}

	//const char *fname = (argc > 1)? argv[1] : "data/test.par";
	const char *fname = argv[argstart];

	cparser p = load_parser(fname);
	std::string data = read_file("/dev/stdin");
	std::string_view foo = data;
	//auto meh = p["main"](foo);
	auto meh = evaluate(p["main"], foo);

	if (meh.matched) {
		switch (mode) {
			case outputJson:
				dump_tokens_json(meh.tokens);
				break;

			case outputSexps:
				dump_tokens_sexps(meh.tokens);
				break;

			case outputTree:
			default:
				dump_tokens_tree(meh.tokens);
				break;
		}

	} else {
		//debug_trace(meh);
		std::cerr << "didn't match." << std::endl;
		return 1;
	}

	return 0;
}
