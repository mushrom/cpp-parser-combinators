#include <p_comb/autolist.hpp>

namespace p_comb {

autolist<int32_t>::ptr make_fstream_ascii(FILE *fp) {
	int32_t c = fgetc(fp);

	if (feof(fp)) {
		return nullptr;
	}

	return autolist<int32_t>::ptr(
		new autolist<int32_t>(
			[=] () { return make_fstream_ascii(fp); },
			c)
		);
}


// namespace p_comb
}
