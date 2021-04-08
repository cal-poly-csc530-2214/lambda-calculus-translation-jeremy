#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <cassert>

namespace lexer {
	enum class TokenType {
		LPAREN,
		RPAREN,
		TIMES,
		PLUS,
		LAMBDA,
		LAMBDA_ARROW,
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
		void back() {
			position--;
		}
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
			if (i+1 < len && s[i] == '=' && s[i+1] == '>') {
				Token t = {"=>", TokenType::LAMBDA_ARROW};
				ts.add(t);
				i+=2;
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			std::string value = "";
			bool first_letter = true;
			while (isalpha(s[i]) || (!first_letter && isdigit(s[i]))) {
				value += s[i];
				i++;
				first_letter = false;
			}
			if (value.length()) {
				Token t = {value, TokenType::ID};
				ts.add(t);
			}
		},
		[](const char* s, size_t& i, size_t len, TokenStream& ts) -> void {
			std::string value = "";
			bool first_letter = true;
			while ((first_letter && s[i] == '-') || isdigit(s[i])) {
				value += s[i];
				i++;
				first_letter = false;
			}
			if (value.length()) {
				Token t = {value, TokenType::NUM};
				ts.add(t);
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
			int oldI = i;
			for (auto& m : matchers) {
				m(s, i, len, *ts);
			}
			if (i == oldI) {
				std::cerr << "No matching token found at position " << i << std::endl;
				std::cerr << input << std::endl;
				exit(1);
			} 
		}
		std::move(ts);
		return ts;
	}
};

namespace parser {
	class ASTNode {
	public:
		virtual ~ASTNode() {};
		virtual void compile(std::stringstream& ss) = 0;
	};
	class Add : public ASTNode {
	public:
		Add(ASTNode *left, ASTNode *right):
			left(left), right(right) {};
		~Add() {
			delete left;
			delete right;
		};
		void compile(std::stringstream& ss) {
			ss << "(";
			left->compile(ss);
			ss << ")+(";
			right->compile(ss);
			ss << ")";
		};
	private:
		ASTNode *left, *right;	
	};
	class Times : public ASTNode {
	public:
		Times(ASTNode *left, ASTNode *right):
			left(left), right(right) {};
		~Times() {
			delete left;
			delete right;
		};
		void compile(std::stringstream& ss) {
			ss << "(";
			left->compile(ss);
			ss << ")*(";
			right->compile(ss);
			ss << ")";
		};
	private:
		ASTNode *left, *right;	
	};
	class Lambda : public ASTNode {
	public:
		Lambda(std::string paramName, ASTNode *body):
			paramName(paramName), body(body) {};
		~Lambda() {
			delete body;
		};
		void compile(std::stringstream& ss) {
			ss << paramName << " => (";
			body->compile(ss);
			ss << ")";
		};
	private:
		std::string paramName;
		ASTNode *body;	
	};
	class Apply : public ASTNode {
	public:
		Apply(ASTNode* arg, ASTNode *func):
			func(func), arg(arg) {};
		~Apply() {
			delete func;
			delete arg;
		};
		void compile(std::stringstream& ss) {
			ss << "(";
			func->compile(ss);
			ss << ")(";
			arg->compile(ss);
			ss << ")"; 
		};
	private:
		ASTNode *func, *arg;
	};	
	class VarRead : public ASTNode {
	public:
		VarRead(std::string name):
			name(name) {};
		~VarRead() {};
		void compile(std::stringstream& ss) {
			ss << name;
		};
	private:
		std::string name;
	};
	class Constant : public ASTNode {
	public:
		Constant(std::string value):
			value(value) {};
		~Constant() {};
		void compile(std::stringstream& ss) {
			ss << value;
		};
	private:
		std::string value;
	};
	class IfLeq0 : public ASTNode {
	public:
		IfLeq0(ASTNode* falseResult, ASTNode* trueResult, ASTNode* value) :
			value(value), trueResult(trueResult), falseResult(falseResult) {};
		~IfLeq0() {
			delete value;
			delete trueResult;
			delete falseResult;
		};
		void compile(std::stringstream& ss) {
			ss << "((";
			value->compile(ss);
			ss << ") <= 0) ? (";
			trueResult->compile(ss);
			ss << ") : (";
			falseResult->compile(ss);
			ss << ")";
		};
	private:
		ASTNode* value, * trueResult, * falseResult;
	};
	class PrintLn : public ASTNode {
	public:
		PrintLn(ASTNode* message) :
			message(message) {};
		~PrintLn() {
			delete message;
		};
		void compile(std::stringstream& ss) {
			ss << "(x => {console.log(x); return x;})(";
			message->compile(ss);
			ss << ")";
		};
	private:
		ASTNode* message;
	};

	ASTNode* parseBase(lexer::TokenStream& ts) {
		if (ts.peek().type == lexer::TokenType::LPAREN) {
			ts.next();
			ASTNode* node;
			auto& token = ts.next();
			if (token.type == lexer::TokenType::PLUS) {
				node = new Add(parseBase(ts), parseBase(ts));
			}
			else if (token.type == lexer::TokenType::TIMES) {
				node = new Times(parseBase(ts), parseBase(ts));
			
			}
			else if (token.type == lexer::TokenType::LAMBDA) {
				assert(ts.peek().type == lexer::TokenType::ID);
				auto param = ts.peek().value;
				ts.next();
				assert(ts.next().type == lexer::TokenType::LAMBDA_ARROW);
				node = new Lambda(param, parseBase(ts));
			}
			else {
				ts.back();
				if (ts.peek().type == lexer::TokenType::ID) {
					if (ts.peek().value == "ifleq0") {
						ts.next();
						node = new IfLeq0(parseBase(ts), parseBase(ts), parseBase(ts));
						ts.next(); // Skip ')'
						return node;
					}
					else if (ts.peek().value == "println") {
						ts.next();
						node = new PrintLn(parseBase(ts));
						ts.next(); // Skip ')'
						return node;
					}
				}
				node = new Apply(parseBase(ts), parseBase(ts));	
			}
			ts.next(); // Skip ')'
			return node;
		}
		else if (ts.peek().type == lexer::TokenType::ID) {
			return new VarRead(ts.next().value);
		}
		else if (ts.peek().type == lexer::TokenType::NUM) {
			return new Constant(ts.next().value);
		}
		else {
			std::cout << ts.peek().value << std::endl;
			assert(false);
		}
	};

	std::unique_ptr<ASTNode> parse(lexer::TokenStream& ts) {
		auto ast = std::unique_ptr<ASTNode>(parseBase(ts));
		std::move(ast);
		return ast;
	};
};

int main() {
	std::string input = "((/ x => (ifleq0 (+ x -10) x (println -1))) 20)";

	auto ts = lexer::lex(input);
	auto ast = parser::parse(*ts);
	std::stringstream ss;
	ast->compile(ss);

	std::cout << "JavaScript Output: \n";
	std::cout << ss.str() << std::endl;

	return 0;
}
