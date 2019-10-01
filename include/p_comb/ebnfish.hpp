#pragma once
#include <p_comb/autolist.hpp>
#include <p_comb/parser.hpp>
#include <map>

namespace p_comb {

typedef std::map<std::string, parser> cparser;
std::string collect(std::list<token>& tokens);
cparser compile_parser(std::list<token>& tokens);

extern parser ebnfish;

// namespace p_comb
}
