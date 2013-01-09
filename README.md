args
====

C++ command-line argument handling for people with better things to do.

Examples
--------
```c++
// test.cpp

#include <iostream>

#include "Args.hpp"

int main(int argc, const char *argv[]) {
	Args args;

	// Define some arguments your program will accept.
	args.addKeywordArg("-key1", "-k1");
	args.addKeywordArg("-key2", "-k2");
	args.addUnaryArg("--unary1", "--u1");

	// Process the command-line arguments.
	args.processArgs(argc, argv);

	// Print argument values.
	std::cout << "Keyword \"-key1\" defined: ";
	if (args.keywordArgDefined("-key1")) {
		std::cout << "true\n"; 
		std::cout << "Value: \"" << args.valueForKeywordArg("-key1") << "\"\n";
	} else std::cout << "false\n";
	std::cout << "Keyword \"-key2\" defined: ";
	if (args.keywordArgDefined("-key2")) {
		std::cout << "true\n"; 
		std::cout << "Value: \"" << args.valueForKeywordArg("-key2") << "\"\n";
	} else std::cout << "false\n";
	std::cout << "Unary argument \"--unary1\" defined: " << (args.unaryArgDefined("--unary1")?"true":"false") << "\n";
	std::cout << "Has unary arg \"--unary1\" : " << (args.hasUnaryArg("--unary1")?"true":"false") << "\n";
	std::cout << "Has unary arg \"--unary2\" : " << (args.hasUnaryArg("--unary2")?"true":"false") << "\n";

	if (args.errors().size() > 0) {
		std::cout << "\n\nArgument errors\n---------------\n";
		for (int e = 0; e < args.errors().size(); ++ e) {
			std::cout << args.errors()[e]->description() << "\n";
		}
	}

	return 0;
}
```
