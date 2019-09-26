#pragma once

#include <p_comb/autolist.hpp>
#include <stdint.h>

namespace p_comb {

typedef uint32_t token;

autolist<token>::ptr
make_token_stream(autolist<char>::ptr cstream) {
	if (!cstream) {
		return nullptr;
	}

	autolist<char>::ptr next = cstream->next();

	token foo = (char)*cstream;

	return autolist<token>::ptr(
		new autolist<token>(
			[=] () { return make_token_stream(next); },
			foo)
		);
}

// TODO: can we template this?
autolist<token>::ptr operator+(autolist<token>::ptr a, autolist<token>::ptr b) {
	if (a && b) {
		return autolist<token>::ptr(new autolist<token>(
			[=] () { return a->next() + b; },
			a->data
		));
	}

	else return a? a : b;
}

// namespace p_comb
}
