#pragma once
#include <p_comb/parser.hpp>
#include <map>

namespace p_comb {

typedef std::map<std::string, parser> cparser;
std::string collect(container& tokens);
cparser compile_parser(container& tokens);

extern parser ebnfish;

// namespace p_comb
}
