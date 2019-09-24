#pragma once

#include <memory>
#include <functional>
#include <iostream>

namespace p_comb {

template <class T>
class autolist {
	public:
		typedef std::shared_ptr<autolist<T>> ptr;
		typedef std::function<ptr ()> generator;

		autolist(generator ngen, T ndata) {
			gen = ngen;
			data = ndata;
		}

		operator T () {
			return data;
		}

		ptr join(ptr other) {
			return ptr(new autolist<T>(
				[=] () {
					// sanity check
					if (!this) return other;

					ptr foo = next();

					return foo? foo->join(other) : other;
				},
				data));
		}

		ptr next(void) {
			// TODO: locking for thread safety
			if (!have_cached) {
				have_cached = true;
				cached_next = gen();
			}

			return cached_next;
		}

		T data;

	private:
		ptr cached_next = nullptr;
		generator gen = nullptr;
		bool have_cached = false;
};

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
