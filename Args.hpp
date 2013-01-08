//
// Args.hpp
//

#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#ifndef ARGS_H
#define ARGS_H

class Args {
public:

	class Exception : public std::runtime_error {
	public:
		Exception(const char *message) : std::runtime_error(message) { }
		Exception(const std::string& message) : std::runtime_error(message.c_str()) { }
	};

	class Error {
	public:

		class UnrecognizedArg;
		class NoValueForKey;

		enum Type { UNRECOGNIZED_ARG, NO_VALUE_FOR_KEY };
		Error(Type type) : _type(type) { }
		virtual ~Error() { }
		Type type() const { return _type; }
		virtual const std::string& description() const {
			return _description;
		}

	protected:
		std::string _description;
	private:
		Type _type;
	};

	class Error::UnrecognizedArg : public Error {
	public:
		UnrecognizedArg(const char *arg) : Error(UNRECOGNIZED_ARG), _arg(arg) {
			std::stringstream ss;
			ss << "Unrecognized argument: \"" << arg << "\".";
			_description = ss.str();
		}
		const std::string& arg() const { return _arg; }
	private:
		std::string _arg;
	};

	class Error::NoValueForKey : public Error {
	public:
		NoValueForKey(const char *key) : Error(NO_VALUE_FOR_KEY), _key(key) {
			std::stringstream ss;
			ss << "No corresponding value for keyword argument \"" << key << "\".";
			_description = ss.str();
		}
		const std::string& key() const { return _key; }
	private:
		std::string _key;
	};

	// Default constructor.
	Args() { }
	virtual ~Args() {
		// Release the errors.
		for (int i = 0; i < _errors.size(); ++ i) {
			delete _errors[i];
		}
	}
	const std::string& execName() const { return _exec_name; }
	void addKeywordArg(const char *keyword_arg, const char *abbr = NULL) {
		std::string keyword_arg_str = keyword_arg;
		std::string abbr_str;
		if (abbr != NULL) abbr_str = abbr;
		addKeywordArg(keyword_arg_str, abbr_str);
	}
	void addKeywordArg(const std::string& keyword_arg, const std::string& abbr = "") {
		// Throw exception on duplicates and on keywords that contain colons.
		if (keyword_arg.find(':') != std::string::npos) {
			std::stringstream ss;
			ss << "Keyword argument \"" << keyword_arg << "\" contains a colon.";
			throw Exception(ss.str());
		}
		_keyword_args.push_back(keyword_arg);
		_keyword_arg_abbrs.push_back(abbr);
		_keyword_args_defined.push_back(false);
		_keyword_arg_values.push_back("");
	}
	void addUnaryArg(const char *unary_arg, const char *abbr = NULL) {
		std::string unary_arg_str = unary_arg;
		std::string abbr_str;
		if (abbr != NULL) abbr_str = abbr;
		addUnaryArg(unary_arg_str, abbr_str);
	}
	void addUnaryArg(const std::string& unary_arg, const std::string& abbr = "") {
		// Throw exception on duplicate unary args or those which contain a colon.
		if (unary_arg.find(':') != std::string::npos) {
			std::stringstream ss;
			ss << "Unary argument \"" << unary_arg << "\" contains a colon. It would be interpreted as a a key-value pair.";
			throw Exception(ss.str());
		}
		_unary_args.push_back(unary_arg);
		_unary_arg_abbrs.push_back(abbr);
		_unary_args_defined.push_back(false);
	}
	void processArgs(int argc, const char *argv[]) {
		if (argc <= 0) return;
		_exec_name = argv[0];
		std::string arg;
		int ai = 1;
		while (true) {
			argloop: if (ai == argc) break;
			arg = argv[ai];
			// See if argument matches any unary args.
			for (int i = 0; i < _unary_args.size(); ++ i) {
				if (_unary_args[i] == arg || _unary_arg_abbrs[i] == arg) {
					_unary_args_defined[i] = true;
					++ ai;
					goto argloop;
				}
			}
			// See if it matches any keyword args.
			for (int i = 0; i < _keyword_args.size(); ++ i) {
				if (_keyword_args[i] == arg || _keyword_arg_abbrs[i] == arg) {
					++ ai;
					if (ai == argc) {
						Error::NoValueForKey *error = new Error::NoValueForKey(arg.c_str());
						_errors.push_back(error);
						return;
					}
					_keyword_args_defined[i] = true;
					_keyword_arg_values[i] = argv[ai];
					++ ai;
					goto argloop;
				}
			}
			// Add an "unrecognized argument" error.
			Error::UnrecognizedArg *error = new Error::UnrecognizedArg(arg.c_str());
			_errors.push_back(error);
			++ ai;
		}
	}
	bool hasKeywordArg(const char *keyword_arg) const {
		for (int i = 0; i < _keyword_args.size(); ++ i) {
			if (_keyword_args[i] == keyword_arg) return true;
		}
		return false;
	}
	bool hasKeywordArg(const std::string& keyword_arg) const {
		return hasKeywordArg(keyword_arg.c_str());
	}
	bool keywordArgDefined(const char *keyword_arg) const {
		for (int i = 0; i < _keyword_args.size(); ++ i) {
			if (_keyword_args[i] == keyword_arg) return _keyword_args_defined[i];
		}
		{
			std::stringstream ss;
			ss << "No such keyword argument: \"" << keyword_arg << "\".";
			throw Exception(ss.str());
		}
	}
	bool keywordArgDefined(const std::string& keyword_arg) const {
		return keywordArgDefined(keyword_arg.c_str());
	}
	const std::string& valueForKeywordArg(const char *keyword_arg) const {
		for (int i = 0; i < _keyword_args.size(); ++ i) {
			if (_keyword_args[i] == keyword_arg) return _keyword_arg_values[i];
		}
		{
			std::stringstream ss;
			ss << "Cannot retrieve value for \"" << keyword_arg << "\": no such keyword argument.";
			throw Exception(ss.str());
		}
	}
	const std::string& valueForKeywordArg(const std::string& keyword_arg) const {
		return valueForKeywordArg(keyword_arg.c_str());
	}
	bool hasUnaryArg(const char *unary_arg) const {
		for (int i = 0; i < _unary_args.size(); ++ i) {
			if (_unary_args[i] == unary_arg) return true;
		}
		return false;
	}
	bool unaryArgDefined(const char *unary_arg) const {
		for (int i = 0; i < _unary_args.size(); ++ i) {
			if (_unary_args[i] == unary_arg) return _unary_args_defined[i];
		}
		{
			std::stringstream ss;
			ss << "No such unary arg: \"" << unary_arg << "\".";
			throw Exception(ss.str());
		}
	}
	const std::vector<Error*>& errors() const {
		return _errors;
	}
private:
	// The name of the executable as it was invoked (typically the first command-line argument).
	std::string _exec_name;
	// Unary arg vectors.
	std::vector<std::string> _unary_args;
	std::vector<std::string> _unary_arg_abbrs;
	std::vector<bool>        _unary_args_defined;
	// Keyword arg vectors.
	std::vector<std::string> _keyword_args;
	std::vector<std::string> _keyword_arg_abbrs;
	std::vector<bool>        _keyword_args_defined;
	std::vector<std::string> _keyword_arg_values;
	// Errors.
	std::vector<Error*>      _errors;
};

#endif
