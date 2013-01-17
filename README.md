args
====

C++ command-line argument handling for people with better things to do.

Background
----------
Processing command-line arguments from within a program can be messy. Doing it right takes a lot of development time, and doing it wrong often results in a frustrating, limited user interface. This package (really just a C++ header file) is designed to make handling arguments easy, so you can move on to better things.

Chances are, you don't particularly care whether the user types `-key=val` or `-key val` when invoking your program. *args* makes no distinction, interpreting each of these patterns as an assignment. *args* also allows you to control which characters may be used for assignments. For instance, the colon (as in `-key:val`) may be an appropriate choice for your project.

You probably also want your arguments to have abbreviations, so that the user doesn't have to type *--some-incredibly-long-argument-name* each time they run your program. *args* allows you to specify an abbreviation for every argument, and automagically checks for naming conflicts.

*args* keeps a human-readable record of the errors it encounters when processing your program's command-line arguments. This allows your pgroam's users to more quickly identify the error in their invocation and get on to using the program.

Examples
--------
```c++
// test.cpp

#include <iostream>

#include "Args.hpp"

int main(int argc, const char *argv[]) {
	// The command-line argument parser, initialized with default settings.
	Args args;

	// Define some arguments your program will accept.
	args.addKeywordArg("-key1", "-k1");
	args.addKeywordArg("-key2", "-k2");
	args.addUnaryArg("--unary1", "--u1");

	// Process the command-line arguments.
	args.processArgs(argc, argv);

	// Print argument values.
	// See if -key1 is defined.
	std::cout << "Keyword \"-key1\" defined: ";
	if (args.keywordArgDefined("-key1")) {
		std::cout << "true\n"; 
		std::cout << "Value: \"" << args.valueForKeywordArg("-key1") << "\"\n";
	} else std::cout << "false\n";

	// See if -key2 is defined.
	std::cout << "Keyword \"-key2\" defined: ";
	if (args.keywordArgDefined("-key2")) {
		std::cout << "true\n"; 
		std::cout << "Value: \"" << args.valueForKeywordArg("-key2") << "\"\n";
	} else std::cout << "false\n";

	// See if --unary1 is defined.
	std::cout << "Unary argument \"--unary1\" defined: " << (args.unaryArgDefined("--unary1")?"true":"false") << "\n";
	std::cout << "Has unary arg \"--unary1\" : " << (args.hasUnaryArg("--unary1")?"true":"false") << "\n";
	std::cout << "Has unary arg \"--foo\" : " << (args.hasUnaryArg("--foo")?"true":"false") << "\n";

	// If there were any errors processing the arguments, write a description of each to STDERR, and exit with code 1.
	if (args.errors().size() > 0) {
		std::cerr << "\n\nArgument errors\n---------------\n";
		for (int e = 0; e < args.errors().size(); ++ e) {
			std::cerr << args.errors()[e]->description() << "\n";
		}
		return 1;
	}

	//
	// Your program logic goes here.
	//

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
./test --foo --unary1 --unary1 --unary1 -key1
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
	Unary argument "--unary1" has been defined 3 times.
	No corresponding value for keyword argument "-key1".

Although "--foo" is specified on the command-line, args.hasUnaryArg("--foo") still returns false because the argument handling object was not configured to respond to "--foo".

Other Options
-------------

By default, *args* treats redefinition of keyword arguments and restatement of unary arguments as errors. You can disable this behavior by adding this line
```c++
args.setRedefinitionIsError(false);
```
before processing arguments.

You can choose which characters function as separators in key/value strings using the `setSeps` method. For example, to allow users to separate keywords from values with a colon <b>or</b> an equals sign, add
```c++
args.setSeps(":=");
```
