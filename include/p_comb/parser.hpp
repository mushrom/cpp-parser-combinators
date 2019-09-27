#pragma once

#include <p_comb/autolist.hpp>
#include <functional>
#include <list>

namespace p_comb {

typedef struct token {
	std::list<token> tokens;
	char data = 0;
} token;

struct result {
	autolist<char>::ptr next;
	token tokens;
	bool matched = false;
};

#define RESULT_NO_MATCH ((struct result) { nullptr, {{}, 0}, false, })

typedef std::function<struct result (autolist<char>::ptr)> parser;

parser string_parser(std::string str);

parser operator+(parser a, parser b);
parser operator|(parser a, parser b);
parser operator+(std::string a, parser b);
parser operator+(parser a, std::string b);
parser operator|(std::string a, parser b);
parser operator|(parser a, std::string b);

parser one_or_more(parser p);
parser zero_or_more(parser p);
parser zero_or_one(parser p);
parser ignore(parser p);

// namespace p_comb
}
