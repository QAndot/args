args
====

C++ command-line argument handling for people with better things to do.

Background
----------
Processing command-line arguments from within a program can be messy. Doing it right takes a lot of development time, and doing it wrong can result in a frustrating, limited user interface. This package (really just a C++ header file) is designed to make handling arguments easy, so you can move on to better things.

Chances are, you don't particularly care whether the end user types `-key=val` or `-key val`, the program should get the point. *args* makes no distinction, interpreting each of these patterns as an assignment. *args* also allows you to control which characters may be used for assignments. For instance, the colon may be an appropriate choice for your project.

You probably also want your arguments to have abbreviations, so that the user doesn't have to type *--some-incredibly-long-argument-name* each time they run your program, but can type a clear, uniquely identifying abbreviation instead. For that reason, *args* allows you to specify an abbreviation for every argument.

*args* keeps a record of the errors it encounters when processing your program's command-line arguments. These errors can be presented to the user in a human-readable form, allowing them to more quickly identify the error in their invocation. For an example of this, see below.

Examples
--------
```c++
// test.cpp

#include <iostream>

#include "Args.hpp"

int main(int argc, const char *argv[]) {
	Args args;

	// Define some arguments your program will accept.
	// Give the full name of each argument, and end with an optional abbreviation.
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
	std::cout << "Has unary arg \"--foo\" : " << (args.hasUnaryArg("--foo")?"true":"false") << "\n";

	// Print argument errors, if there are any.
	if (args.errors().size() > 0) {
		std::cout << "\n\nArgument errors\n---------------\n";
		for (int e = 0; e < args.errors().size(); ++ e) {
			std::cout << args.errors()[e]->description() << "\n";
		}
		return 1;
	}

	return 0;
}
```

When run like this,
```bash
./test -k1=value1 -key2 value2 --unary1
```
the program produces this output:

	Keyword "-key1" defined: true
	Value: "value1"
	Keyword "-key2" defined: true
	Value: "value2"
	Unary argument "--unary1" defined: true
	Has unary arg "--unary1" : true
	Has unary arg "--foo" : false

When run like this,
```bash
./test --foo --unary1 --unary1 -key1
```
the program produces this output:

	Keyword "-key1" defined: false
	Keyword "-key2" defined: false
	Unary argument "--unary1" defined: true
	Has unary arg "--unary1" : true
	Has unary arg "--foo" : false


	Argument errors
	---------------
	Unrecognized argument: "--foo".
	Unary argument "--unary1" has been redefined.
	No corresponding value for keyword argument "-key1".

By default, *args* treats redefinition of keyword arguments and restatement of unary arguments as errors. You can disable this behavior by adding this line
```c++
args.setRedefinitionIsError(false);
```
before
```c++
args.processArgs(argc, argv);
```

To allow users to separate keywords from values with a colon _or_ an equals sign, add
```c++
args.setSeps(":=");
```
