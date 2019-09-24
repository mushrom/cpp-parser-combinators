#include <p_comb/autolist.hpp>
#include <p_comb/tokenizer.hpp>
#include <p_comb/parser.hpp>

using namespace p_comb;

struct result dummy_parser(autolist<token>::ptr ptr) {
	if (!ptr) {
		return (struct result) { ptr, ptr, false, };
	}

	return (struct result) {
		ptr->next(),
		ptr,
		true,
	};
}

int main(void) {
	auto foo = make_fstream(stdin);
	auto baz = foo;
	auto asdf = make_token_stream(foo);
	auto qwerty = asdf;

	/*
	auto pa = [] (autolist<token>::ptr ptr) {
		return (struct result) {
			ptr->next(),
			ptr->next(),
			true,
		};
	};
	*/

	parser pa = dummy_parser;
	parser pb = pa + pa;
	parser pc = pb + pb;
	parser pd = pc | pb;

	auto lel = foo->join(baz);
	//lel = lel->join(lel->join(lel->join(lel)));

	printf("character stream (first read):\n");
	for (; asdf; asdf = asdf->next()) {
		token& blarg = asdf->data;

		printf("have token '%s' of type %u\n",
			blarg.data.c_str(),
			blarg.type);
	}

	printf("parser token stream:\n");
	struct result meh = pc(qwerty);
	auto bleh = meh.tokens;

	for (; bleh; bleh = bleh->next()) {
		token& blarg = bleh->data;

		printf("have token '%s' of type %u\n",
			blarg.data.c_str(),
			blarg.type);
	}

	printf("autolist testing:\n");
	for (; lel; lel = lel->next()) {
		printf("have '%c'\n", (char)*lel);
	}

	return 0;
}
