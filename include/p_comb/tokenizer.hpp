#pragma once

#include <p_comb/autolist.hpp>
#include <stdint.h>

namespace p_comb {

class token {
	public:
		uint32_t type;
		std::string data;
};

autolist<token>::ptr
make_token_stream(autolist<char>::ptr cstream) {
	if (!cstream) {
		return nullptr;
	}

	autolist<char>::ptr next = cstream->next();

	token foo;
	foo.type = (char)*cstream;
	foo.data = (char)*cstream;

	return autolist<token>::ptr(
		new autolist<token>(
			[=] () { return make_token_stream(next); },
			foo)
		);
}

// namespace p_comb
}
