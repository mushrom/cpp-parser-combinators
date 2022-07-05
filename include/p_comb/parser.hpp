#pragma once

#include <functional>
//#include <list>
#include <vector>
#include <string_view>
#include <string>
#include <iostream>
#include <optional>
#include <stdio.h>

namespace p_comb {

// TODO: would be cool to have this be templated,
//       so that any iterator type could be used
//using viewPosition = std::string_view::iterator;
//using viewPair     = std::pair<viewPosition, viewPosition>;

struct token;
typedef std::vector<token> container;

struct token {
	int32_t data = 0;
	std::string tag = "";
	std::optional<container> subtokens;
};

struct parserState {
	using stackType = std::vector<container>;
	//using frameType = container::iterator;
	using frameType = size_t;

	stackType tokenStack;
	std::vector<std::string> tagStack;

	// ensure that there's a bottom to the stack, if no tag is ever pushed then
	// all tokens will end up in this container
	//container untaggedTokens;

	parserState() {
		//push(&untaggedTokens);
		tokenStack.push_back({});
	}

	const container& front() {
		return tokenStack.front();
	}

	void pushTag(const std::string& tag) {
		tokenStack.push_back({});
		tagStack.push_back(tag);
	}

	void popTag() {
		if (!tagStack.empty()) {
			auto& tag = tagStack.back();
			container& ret = tokenStack.back();
			
			token t;
			t.tag = tag;
			t.subtokens = std::move(ret);

			tokenStack.pop_back();
			tagStack.pop_back();
			pushToken(t);
		}
	}

	void discardTag() {
		if (!tagStack.empty()) {
			tagStack.pop_back();
			tokenStack.pop_back();
		}
	}

	frameType capture() {
		//return tokenStack.back().end() - 1;
		return tokenStack.back().size();
	}

	void restore(frameType& frame) {
		//tokenStack.back().erase(frame + 1, tokenStack.back().end());
		auto& t = tokenStack.back();
		if (frame < t.size()) {
			t.erase(t.begin() + frame, t.end());
			//t.resize(frame);
		}
	}

	void pushToken(const token& t) {
		tokenStack.back().push_back(t);
	}
};

struct result {
	std::string_view next;
	//token::container tokens;
	bool matched = false;
	//autolist<int32_t>::ptr next;
	//viewPair
	//token::container tokens;
	//std::vector<std::string_view> debug;
};

#define RESULT_NO_MATCH ((struct result) { "", false })

struct evalResult {
	bool      matched;
	container tokens;
};

typedef std::function<result (std::string_view, parserState&)> parser;

static inline
evalResult evaluate(parser p, std::string_view view) {
	parserState state;
	result res = p(view, state);

	return {
		res.matched,
		state.front(),
	};
}

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
