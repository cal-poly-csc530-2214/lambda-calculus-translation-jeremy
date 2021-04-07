#include <iostream>
#include <vector>
#include <memory>
#include <string>

namespace lexer {
	enum class TokenType {
		LPAREN,
		RPAREN,
		TIMES,
		PLUS,
		LAMBDA,
		ID,
		NUM
	};
	struct Token {
		std::string value;
		TokenType type;
	};
	class TokenStream {
	public:
		TokenStream() {};
		Token& next() {
			return tokens[position++];
		};
		Token& peek() {
			return tokens[position];
		};
		Token& peek(size_t lookahead) {
			return tokens[position+lookahead];
		};
		void add(Token t) {
			tokens.push_back(t);
		};
	private:
		std::vector<Token> tokens;
		size_t position = 0;
	};

	typedef void (*Matcher)(const char*, size_t&, size_t, TokenStream&);
	Matcher matchers[] = {
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			if (s[i] == '(') {
				Token t = {"(", TokenType::LPAREN};
				ts.add(t);
				i++;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			if (s[i] == ')') {
				Token t = {")", TokenType::RPAREN};
				ts.add(t);
				i++;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			if (s[i] == '*') {
				Token t = {"*", TokenType::TIMES};
				ts.add(t);
				i++;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			if (s[i] == '+') {
				Token t = {"+", TokenType::PLUS};
				ts.add(t);
				i++;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			if (s[i] == '/') {
				Token t = {"/", TokenType::LAMBDA};
				ts.add(t);
				i++;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			std::string value = "";
			while (s[i] >= 'a' && s[i] <= 'z' && s[i] >= 'A' && s[i] <= 'Z') {
				value += s[i];
				i++;
			}
			if (value.length()) {
				Token t = {value, TokenType::ID};
				ts.add(t);
				i++;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			std::string value = "";
			while (s[i] >= '0' && s[i] <= '9') {
				value += s[i];
				i++;
			}
			if (value.length()) {
				Token t = {value, TokenType::NUM};
				ts.add(t);
				i++;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			while (s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r') {
				i++;
			}
		}
	};

	std::unique_ptr<TokenStream> lex(std::string& input) {
		auto ts = std::make_unique<TokenStream>();
		const char* s = input.c_str();
		size_t i = 0;
		size_t len = input.length();
		while (i < len) {
			for (auto& m : matchers) {
				m(s, i, len, *ts);
			}
			std::cout << i << ": " << input << std::endl;
		}
		std::move(ts);
		return ts;
	}
};

int main() {
	std::string input = "(+ 8 9)";

	auto ts = lexer::lex(input);
	std::cout << ts->peek().value << std::endl;
	//auto ast = parser::parse(*ts);
	//auto output = compiler::compile(*ast);
	
	//std::cout << output << std::endl;

	return 0;
}
