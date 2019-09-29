#pragma once

#include <p_comb/autolist.hpp>
#include <functional>
#include <list>

namespace p_comb {

typedef struct token {
	std::list<token> tokens;
	char data = 0;
	std::string tag = "";
} token;

struct result {
	autolist<char>::ptr next;
	std::list<token> tokens;
	bool matched = false;
	std::list<autolist<char>::ptr> debug;
};

#define RESULT_NO_MATCH ((struct result) { nullptr, {}, false, })

typedef std::function<struct result (autolist<char>::ptr)> parser;

// rules for matching characters from the stream
parser string_parser(std::string str);
parser blacklist(std::string blacklist);

// parser concatenation
parser operator+(parser a, parser b);
parser operator+(std::string a, parser b);
parser operator+(parser a, std::string b);

// parser or, match a or match b.
parser operator|(parser a, parser b);
parser operator|(std::string a, parser b);
parser operator|(parser a, std::string b);

// whitespace-ignoring concatenation
parser operator>>(parser a, parser b);
parser operator>>(std::string a, parser b);
parser operator>>(parser a, std::string b);
parser whitewrap(parser a);

// other helpers
parser one_or_more(parser p);
parser zero_or_more(parser p);
parser zero_or_one(parser p);
parser ignore(parser p);
parser tag(std::string type, parser p);

// some commonly-used base parser rules
// TODO: should find a better place to put this...
static parser whitespace_char = string_parser(" ")|"\t"|"\v"|"\n"|"\r"|"\a";
static parser whitespace = one_or_more(whitespace_char);
static parser ignore_whitespace = ignore(zero_or_more(whitespace_char));

static parser digit = string_parser("0") | "1" | "2"
                    | "3" | "4" | "5" | "6"
                    | "7" | "8" | "9";

static parser number = tag("number", zero_or_one(string_parser("-"))
                                     + one_or_more(digit));

static parser lowercase = string_parser("a")|"b"|"c"|"d"|"e"|"f"|"g"|"h"
                        |"i"|"j"|"k"|"l"|"m"|"n"|"o"|"p"|"q"|"r"|"s"|"t"
                        |"u"|"v"|"w"|"x"|"y"|"z";

static parser uppercase = string_parser("A")|"B"|"C"|"D"|"E"|"F"|"G"|"H"
                        |"I"|"J"|"K"|"L"|"M"|"N"|"O"|"P"|"Q"|"R"|"S"|"T"
                        |"U"|"V"|"W"|"X"|"Y"|"Z";

static parser letter = lowercase | uppercase;

// namespace p_comb
}
