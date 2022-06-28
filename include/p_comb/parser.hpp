#pragma once

#include <functional>
//#include <list>
#include <vector>
#include <string_view>
#include <string>
#include <iostream>
#include <stdio.h>

namespace p_comb {


typedef struct token {
	typedef std::vector<token> container;

	container tokens;
	int32_t data = 0;
	std::string tag = "";
} token;

struct result {
	//autolist<int32_t>::ptr next;
	std::string_view next;
	token::container tokens;
	bool matched = false;
	std::vector<std::string_view> debug;
};

#define RESULT_NO_MATCH ((struct result) { "", {}, false, })

typedef std::function<struct result (std::string_view)> parser;

// rules for matching characters from the stream
parser string_parser(std::string str);
parser codepoint_range(int32_t start, int32_t end);
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

// treat all control range characters as whitespace
inline parser whitespace_char = codepoint_range(0x01, 0x20);

//inline parser whitespace_char = string_parser(" ")|"\t"|"\v"|"\n"|"\r"|"\a";
inline parser whitespace = one_or_more(whitespace_char);
inline parser ignore_whitespace = ignore(zero_or_more(whitespace_char));

/*
inline parser digit = string_parser("0") | "1" | "2"
                    | "3" | "4" | "5" | "6"
                    | "7" | "8" | "9";
*/

inline parser digit = codepoint_range('0', '9');

inline parser number = tag("number", zero_or_one(string_parser("-"))
                                     + one_or_more(digit));
inline parser lowercase = codepoint_range('a', 'z');
inline parser uppercase = codepoint_range('A', 'Z');
/*
inline parser lowercase = string_parser("a")|"b"|"c"|"d"|"e"|"f"|"g"|"h"
                        |"i"|"j"|"k"|"l"|"m"|"n"|"o"|"p"|"q"|"r"|"s"|"t"
                        |"u"|"v"|"w"|"x"|"y"|"z";

inline parser uppercase = string_parser("A")|"B"|"C"|"D"|"E"|"F"|"G"|"H"
                        |"I"|"J"|"K"|"L"|"M"|"N"|"O"|"P"|"Q"|"R"|"S"|"T"
                        |"U"|"V"|"W"|"X"|"Y"|"Z";
*/

inline parser letter = lowercase | uppercase;


// namespace p_comb
}
