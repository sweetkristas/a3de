#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>

#include "filesystem.hpp"
#include "formatter.hpp"
#include "json.hpp"
#include "unit_test.hpp"

#include "stdafx.h"

namespace json
{

	class lexer
	{
	public:
		enum json_token
		{
			LEFT_BRACE,
			RIGHT_BRACE,
			LEFT_BRACKET,
			RIGHT_BRACKET,
			COLON,
			COMMA,
			STRING_LITERAL,
			INTEGER,
			FLOAT,
			PERIOD,
			LIT_TRUE,
			LIT_FALSE,
			LIT_NULL,
			LITERAL,
			DOCUMENT_END,
		};

		lexer(const std::string& s)
			: lex_str_(s)
		{
			it_ = lex_str_.begin();
		}
		
		uint16_t decode_hex_nibble(char c)
		{
			if(c >= '0' && c <= '9') {
				return c - '0';
			} else if(c >= 'a' && c <= 'f') {
				return c - 'a' + 10;
			} else if(c >= 'A' && c <= 'F') {
				return c - 'A' + 10;
			}
			throw new parse_error(formatter() << "Invalid character in decode: " << c);
		}
		
		boost::tuple<json_token, node> get_next_token()
		{
			bool running = true;
			bool in_string = false;
			std::string::iterator start_it = it_;
			std::string new_string;
			while(running) {
				if(it_ == lex_str_.end()) {
					if(in_string) {
						throw new parse_error(formatter() << "End of data inside string");
					}
					return boost::make_tuple(DOCUMENT_END, node());
				}
				if(in_string) {
					if(*it_ == '"') {
						++it_;
						node string_node(new_string);
						if(new_string == "true") {
							return boost::make_tuple(LIT_TRUE, node());
						} else if(new_string == "false") {
							return boost::make_tuple(LIT_FALSE, node());
						} else if(new_string == "null") {
							return boost::make_tuple(LIT_NULL, node());
						}
						return boost::make_tuple(STRING_LITERAL, string_node);
					} else if(*it_ == '\\') {
						++it_;
						if(it_ == lex_str_.end()) {
							throw new parse_error(formatter() << "End of data in quoted token");
						}
						if(*it_ == '"') {
							new_string += '"';
						} else if(*it_ == '\\') {
							new_string += '\\';
						} else if(*it_ == '/') {
							new_string += '/';
						} else if(*it_ == 'b') {
							new_string += '\b';
						} else if(*it_ == 'f') {
							new_string += '\f';
						} else if(*it_ == 'n') {
							new_string += '\n';
						} else if(*it_ == 'r') {
							new_string += '\r';
						} else if(*it_ == 't') {
							new_string += '\t';
						} else if(*it_ == 'u') {
							if((lex_str_.end() - it_) < 4) {
								throw new parse_error(formatter() << "Unexpected 4 hexadecimal characters after \\u token");
								uint16_t value = 0;
								for(int n = 0; n != 4; ++n) {
									value = (value << 4) | decode_hex_nibble(*it_++);
								}
								// Quick and dirty conversion from \uXXXX -> UTF-8
								if(value >= 0 && value <= 127U) {
									new_string += char(value);
								} else if(value >= 128U && value <= 2047U) {
									new_string += char(0xc0 | (value >> 6));
									new_string += char(0x80 | (value & 0x3f));
								} else if(value >= 2048U) {
									new_string += char(0xe0 | (value >> 12));
									new_string += char(0x80 | ((value >> 6) & 0x3f));
									new_string += char(0x80 | (value & 0x3f));
								}
							}
						} else {
							throw new parse_error(formatter() << "Unrecognised quoted token: " << *it_);
						}
					} else {
						new_string += *it_;
					}
				} else {
					if(*it_ == '{' || *it_ == '}' || *it_ == '[' || *it_ == ']' || *it_ == ',' || *it_ == ':' || *it_ == '"' || isspace(*it_)) {
						if(new_string.empty() == false) {
							if(new_string == "true") {
								return boost::make_tuple(LIT_TRUE, node());
							} else if(new_string == "false") {
								return boost::make_tuple(LIT_FALSE, node());
							} else if(new_string == "null") {
								return boost::make_tuple(LIT_NULL, node());
							} else {
								boost::make_tuple(LITERAL, node(new_string));
							}
						}
					}
					if(*it_ == '{') {
						++it_;
						return boost::make_tuple(LEFT_BRACE, node());
					} else if(*it_ == '}') {
						++it_;
						return boost::make_tuple(RIGHT_BRACE, node());
					} else if(*it_ == '[') {
						++it_;
						return boost::make_tuple(LEFT_BRACKET, node());
					} else if(*it_ == ']') {
						++it_;
						return boost::make_tuple(RIGHT_BRACKET, node());
					} else if(*it_ == '.') {
						++it_;
						return boost::make_tuple(PERIOD, node());
					} else if(*it_ == ',') {
						++it_;
						return boost::make_tuple(COMMA, node());
					} else if(*it_ == ':') {
						++it_;
						return boost::make_tuple(COLON, node());
					} else if(*it_ == '"') {
						in_string = true;
						++it_;
					} else if(isdigit(*it_) || *it_ == '-') {
						bool is_float = false;
						std::string num;
						if(*it_ == '-') {
							num += *it_;
							++it_;
						}
						while(isdigit(*it_)) {
							num += *it_;
							++it_;
						}
						if(*it_ == '.') {
							while(isdigit(*it_)) {
								num += *it_;
								++it_;
							}
							is_float = true;
						} 
						if(*it_ == 'e' || *it_ == 'E') {
							is_float = true;
							if(*it_ == '+') {
								num += *it_;
								++it_;
							} else if(*it_ == '-') {
								num += *it_;
								++it_;
							}
							while(isdigit(*it_)) {
								num += *it_;
								++it_;
							}
						}
						if(is_float) {
							return boost::make_tuple(FLOAT, node(boost::lexical_cast<float>(num)));
						} else {
							return boost::make_tuple(INTEGER, node(boost::lexical_cast<int>(num)));
						}
					} else if(isspace(*it_)) {
						++it_;
					} else {
						new_string += *it_;
						++it_;
					}
				}
			}
			return boost::make_tuple(DOCUMENT_END, node());
		}

	private:
		std::string lex_str_;
		std::string::iterator it_;
	};

	node parse(const std::string& s)
	{
		lexer lex(s);
		lexer::json_token tok;
		node token_value;
		do {
			boost::tie(tok, token_value) = lex.get_next_token();
		} while(tok != lexer::DOCUMENT_END);
		return node(); // XXX
	}

	node parse_from_file(const std::string& fname)
	{
		return parse(sys::read_file(fname));
	}

	void write(const std::string fnanme, const node& n)
	{
	}
}

UNIT_TEST(json_parse_test)
{
	node n = json::parse_from_file("data/test/sameple.json");
}
