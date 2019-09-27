#include <p_comb/autolist.hpp>

namespace p_comb {

autolist<char>::ptr make_fstream(FILE *fp) {
	char c = fgetc(fp);

	if (feof(fp)) {
		return nullptr;
	}

	return autolist<char>::ptr(
		new autolist<char>(
			[=] () { return make_fstream(fp); },
			c)
		);
}


// namespace p_comb
}
