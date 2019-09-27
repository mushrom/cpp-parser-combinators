#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>

using namespace p_comb;

parser digit = string_parser("0") | "1" | "2"
             | "3" | "4" | "5" | "6"
             | "7" | "8" | "9";

parser lowercase = string_parser("a")|"b"|"c"|"d"|"e"|"f"|"g"|"h"
                 |"i"|"j"|"k"|"l"|"m"|"n"|"o"|"p"|"q"|"r"|"s"|"t"
                 |"u"|"v"|"w"|"x"|"y"|"z";

parser uppercase = string_parser("A")|"B"|"C"|"D"|"E"|"F"|"G"|"H"
                 |"I"|"J"|"K"|"L"|"M"|"N"|"O"|"P"|"Q"|"R"|"S"|"T"
                 |"U"|"V"|"W"|"X"|"Y"|"Z";

parser letter = lowercase | uppercase;

parser whitespace_char = string_parser(" ")|"\t"|"\v"|"\n"|"\r"|"\a";
parser whitespace = one_or_more(whitespace_char);
parser ignore_whitespace = ignore(zero_or_more(whitespace_char));

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

parser number = one_or_more(digit);
parser op = string_parser("+") | "-" | "*" | "/";
parser identifier = letter + zero_or_more(letter | digit);

struct result expression(autolist<char>::ptr ptr) {
	static const parser temp = 
		  ("(" >> (parser)expression >> ")")
		| (number >> op >> expression)
		| number;

	return temp(ptr);
}

parser expression_list =
	"[" >> zero_or_more((parser)expression
			>> zero_or_one(string_parser(",")
				+ ignore_whitespace))
	    >> "]";

parser assignment = identifier >> "=" >> expression_list >> ";";

void dump_tokens(token& tok, unsigned indent = 0) {
	for (unsigned i = 0; i < indent; i++) {
		std::cout << "   :";
	}

	printf("token %u ('%c')\n", tok.data, tok.data);

	for (auto& t : tok.tokens) {
		dump_tokens(t, indent + 1);
	}
}

int main(void) {
	struct result meh = assignment(make_fstream(stdin));

	if (meh.matched) {
		puts("parser tokens:");
		dump_tokens(meh.tokens);

	} else {
		puts("no matches.");
	}

	return 0;
}
