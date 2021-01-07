#pragma once
#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>
#include <map>

namespace p_comb {

typedef std::map<std::string, parser> cparser;
std::string collect(token::container& tokens);
cparser compile_parser(token::container& tokens);

extern parser ebnfish;

// namespace p_comb
}
