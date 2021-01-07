#pragma once

#include <memory>
#include <functional>
#include <iostream>
#include <stdint.h>

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

autolist<int32_t>::ptr make_fstream_ascii(FILE *fp);
autolist<int32_t>::ptr make_fstream_utf8(FILE *fp);

// namespace p_comb
}
