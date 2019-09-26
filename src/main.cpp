#include <p_comb/autolist.hpp>
#include <p_comb/tokenizer.hpp>
#include <p_comb/parser.hpp>

using namespace p_comb;

struct result digit(autolist<token>::ptr ptr) {
	if (!ptr) {
		return RESULT_NO_MATCH;
	}

	// ascii characters are mapped to their own integer types, so we don't need to
	if (ptr->data >= '0' && ptr->data <= '9') {
		return (struct result) {
			ptr->next(),
			ptr->take(1),
			true,
		};
	}

	return RESULT_NO_MATCH;
}

parser one_or_more(parser p) {
	return [=] (autolist<token>::ptr ptr) {
		if (!ptr) {
			return RESULT_NO_MATCH;
		}

		struct result res;
		autolist<token>::ptr end = ptr;
		autolist<token>::ptr last = ptr;
		unsigned i = 0;

		do {
			last = end;
			res = p(end);
			end = res.next;
			i += res.matched;
		} while (res.matched);

		// TODO: wrap return token stream, so we can actually
		//       get tokens returned when parsing
		if (i >= 1) {
			return (struct result) {
				last,
				ptr->take(i),
				true,
			};
		}

		return RESULT_NO_MATCH;
	};
}

parser zero_or_more(parser p) {
	return [=] (autolist<token>::ptr ptr) {
		if (!ptr) {
			return RESULT_NO_MATCH;
		}

		struct result res;
		autolist<token>::ptr end = ptr;
		autolist<token>::ptr last = ptr;
		unsigned i = 0;

		do {
			last = end;
			res = p(end);
			end = res.next;
			i += res.matched;
		} while (res.matched);

		return (struct result) {
			last,
			ptr->take(i),
			true,
		};
	};
}

struct result dummy_parser(autolist<token>::ptr ptr) {
	if (!ptr) {
		//return (struct result) { ptr, ptr, false, };
		return (struct result) { nullptr, nullptr, true, };
	}

	return (struct result) {
		ptr->next(),
		ptr->take(1),
		true,
	};
}

parser number = one_or_more(digit);

struct result expression(autolist<token>::ptr ptr) {
	static const parser temp = 
		  ("(" + (parser)expression + ")")
		| (number + "+" + expression)
		| (number + "-" + expression)
		| (number + "*" + expression)
		| (number + "/" + expression)
		| number;

	return temp(ptr);
}

parser expression_list = "[" + zero_or_more((parser)expression + ",") + "]";

/*
// TODO: so close to having ML-ish syntax working...
parser expression =
	  ("[" + number + "+" + expression + "]")
	| ("[" + number + "-" + expression + "]")
	| ("[" + number + "*" + expression + "]")
	| ("[" + number + "/" + expression + "]")
	| number;
*/

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

	/*
	parser pa = dummy_parser;
	parser pb = pa + pa + pa;
	parser pc = pb + pb;
	parser pd = pc | pb;
	*/

	/*
	parser expression =
		  ("[" + number + "+" + number + "]")
		| ("[" + number + "-" + number + "]")
		| ("[" + number + "*" + number + "]")
		| ("[" + number + "/" + number + "]")
		| number;
		*/

	//parser expression = "[" + number + "]";
	//parser expression = ("[" + number) + "]";

	//auto lel = foo + baz;
	auto lel = foo;
	//lel = lel->join(lel->join(lel->join(lel)));

	printf("character stream (first read):\n");
	for (; asdf; asdf = asdf->next()) {
		token& blarg = asdf->data;

		printf("have token of type %u\n", blarg);
	}

	struct result meh = expression_list(qwerty);
	auto bleh = meh.tokens;
	printf("parser token stream: (matched: %u)\n", meh.matched);

	for (; bleh; bleh = bleh->next()) {
		token& blarg = bleh->data;

		printf("have token of type %u (%c)\n", blarg, blarg);
	}

	printf("autolist testing:\n");
	for (; lel; lel = lel->next()) {
		printf("have '%c'\n", (char)*lel);
	}

	return 0;
}
