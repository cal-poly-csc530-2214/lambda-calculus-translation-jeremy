## README

I wrote a lexer, parser, and compiler from scratch in C++ that took lambda calculus expressions as input and output JavaScript.
These expressions were of the form of the grammar file specified in the assignment.

The lexer used a set of matching rules (a table of function pointers, with a separate function for each token type) that parsed the input string into a list of tokens.

Since the input grammar is fairly simple, and doesn't include operator precedence for multiplication/addition, I coded most of the parser in a single function with different branches for different grammar constructs. I handled operators (+,\*,ifleq0,println) and lambda expressions in one branch since they all start with an open paren. I recursively called my parsing function when necessary to further parse these elements. I did this when the grammar had recursive references in the rule, such as in (+ LC LC). I then handled variable references (identifiers) and integer constants in separate branches since they were simple rules consisting of single tokens.

The output of my parse function was an AST consisting of nodes all inheriting from the abstract class ASTNode. The individual subclasses implemented a compile method that output their implementation in JavaScript, including recursive calls to compile their children (as is the case for the addition node (+ LC LC)).

I then tested the output in my web browser for a few different examples (such as the one below), and verified the output was correct. In these examples, I made sure to cover all the grammar constructs so all parts were tested. I didn't have time to get unit tests set up in C++.

```
((/ x => (ifleq0 (+ x -10) x (println -1))) 20)
```
outputs
```
(x => ((((x => {console.log(x); return x;})(-1)) <= 0) ? (x) : ((x)+(-10))))(20)
```
