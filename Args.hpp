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
		class RedefinitionOfKey;
		class RedefinitionOfUnaryArg;

		enum Type { UNRECOGNIZED_ARG, NO_VALUE_FOR_KEY, REDEFINITION_OF_KEY, REDEFINITION_OF_UNARY_ARG };
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

	class Error::RedefinitionOfKey : public Error {
	public:
		RedefinitionOfKey(const char *key) : Error(REDEFINITION_OF_KEY), _key(key), _count(2) {
			std::stringstream ss;
			ss << "Keyword argument \"" << key << "\" has been redefined.";
			_description = ss.str();
		}
		void addCount() {
			++ _count;
			std::stringstream ss;
			ss << "Keyword argument \"" << _key << "\" has been defined " << _count << " times.";
			_description = ss.str();
		}
		unsigned count() { return _count; }
		const std::string& key() const { return _key; }
	private:
		std::string _key;
		unsigned _count;
	};

	class Error::RedefinitionOfUnaryArg : public Error {
	public:
		RedefinitionOfUnaryArg(const char *unary_arg) : Error(REDEFINITION_OF_UNARY_ARG), _unary_arg(unary_arg), _count(2) {
			std::stringstream ss;
			ss << "Unary argument \"" << _unary_arg << "\" has been redefined.";
			_description = ss.str();
		}
		void addCount() {
			++ _count;
			std::stringstream ss;
			ss << "Unary argument \"" << _unary_arg << "\" has been defined " << _count << " times.";
			_description = ss.str();
		}
		unsigned count() { return _count; }
		const std::string& unary_arg() const { return _unary_arg; }
	private:
		std::string _unary_arg;
		unsigned _count;
	};

	// Default constructor.
	Args() : _seps("="), _redefinition_is_error(true) { }
	virtual ~Args() {
		// Release the errors.
		for (int i = 0; i < _errors.size(); ++ i) {
			delete _errors[i];
		}
	}
	const std::string& execName() const { return _exec_name; }
	void setSepString(const char *sep_string) {
		// Make sure that separator string does not contain any of the characters in the keyword and unary args or abbreviations.
		for (int ci = 0; ci < _seps.length(); ++ ci) {
			char c = _seps[ci];
			for (int i = 0; i < _unary_args.size(); ++ i) {
				if (_unary_args[i].find(c) != std::string::npos) {
					std::stringstream ss;
					ss << "Separator characters cannot include \"" << c << "\" which is in unary argument \"" << _unary_args[i] << "\".";
					throw Exception(ss.str());
				}
				if (_unary_arg_abbrs[i].find(c) != std::string::npos) {
					std::stringstream ss;
					ss << "Separator characters cannot include \"" << c << "\" which is in the abbreviation (\"" << _unary_arg_abbrs[i] << "\") for the unary argument \"" << _unary_args[i] << "\".";
					throw Exception(ss.str());
				}
			}
			for (int i = 0; i < _keyword_args.size(); ++ i) {
				if (_keyword_args[i].find(c) != std::string::npos) {
					std::stringstream ss;
					ss << "Separator characters cannot include \"" << c << "\" which is in keyword argument \"" << _keyword_args[i] << "\".";
					throw Exception(ss.str());
				}
				if (_keyword_arg_abbrs[i].find(c) != std::string::npos) {
					std::stringstream ss;
					ss << "Separator characters cannot include \"" << c << "\" which is in the abbreviation (\"" << _keyword_arg_abbrs[i] << "\") for the keyword argument \"" << _keyword_args[i] << "\".";
					throw Exception(ss.str());
				}
			}
		}
		_seps = sep_string;
	}
	const std::string& sepString() const { return _seps; }
	bool redefinitionIsError() const { return _redefinition_is_error; }
	void setRedefinitionIsError(bool redefinitionIsError) {
		_redefinition_is_error = redefinitionIsError;
	}
	void addKeywordArg(const char *keyword_arg, const char *abbr = NULL) {
		// Throw an exception on duplicate arguments.
		for (int i = 0; i < _keyword_args.size(); ++ i) {
			if (_keyword_args[i] == keyword_arg) {
				std::stringstream ss;
				ss << "Duplicate keyword argument: \"" << keyword_arg << "\".";
				throw Exception(ss.str());
			}
			if (_keyword_arg_abbrs[i] == keyword_arg) {
				std::stringstream ss;
				ss << "Keyword argument \"" << keyword_arg << "\" matches the abbreviation of another keyword argument: \"" << _keyword_args[i] << "\".";
				throw Exception(ss.str());
			}
			if (_keyword_args[i] == abbr) {
				std::stringstream ss;
				ss << "Keyword argument abbreviation \"" << abbr << "\" matches the full name of another keyword argument.";
				throw Exception(ss.str());
			}
			if (_keyword_arg_abbrs[i] == abbr) {
				std::stringstream ss;
				ss << "Keyword argument abbreviation \"" << abbr << "\" matches the abbreviation of another keyword argument (\"" << _keyword_args[i] << "\").";
				throw Exception(ss.str());
			}
		}
		for (int i = 0; i < _unary_args.size(); ++ i) {
			if (_unary_args[i] == keyword_arg) {
				std::stringstream ss;
				ss << "Keyword argument \"" << keyword_arg << "\" matches a unary argument.";
				throw Exception(ss.str());
			}
			if (_unary_arg_abbrs[i] == keyword_arg) {
				std::stringstream ss;
				ss << "Keyword argument \"" << keyword_arg << "\" matches abbreviation of unary argument: \"" << _keyword_args[i] << "\".";
				throw Exception(ss.str());
			}
			if (_unary_args[i] == abbr) {
				std::stringstream ss;
				ss << "Keyword argument abbreviation \"" << abbr << "\" matches the full name of a unary argument.";
				throw Exception(ss.str());
			}
			if (_unary_arg_abbrs[i] == abbr) {
				std::stringstream ss;
				ss << "Keyword argument abbreviation \"" << abbr << "\" matches the abbreviation of unary argument\"" << _keyword_args[i] << "\".";
				throw Exception(ss.str());
			}
		}
		std::string keyword_arg_str = keyword_arg;
		// Throw exception on keywords that contain separator characters.
		for (int ci = 0; ci < _seps.length(); ++ ci) {
			char c = _seps[ci];
			if (keyword_arg_str.find(c) != std::string::npos) {
				std::stringstream ss;
				ss << "Keyword argument \"" << keyword_arg << "\" contains the separator character \"" << c << "\".";
				throw Exception(ss.str());
			}
		}
		std::string abbr_str;
		if (abbr != NULL) abbr_str = abbr;
		_keyword_args.push_back(keyword_arg_str);
		_keyword_arg_abbrs.push_back(abbr_str);
		_keyword_args_defined.push_back(false);
		_keyword_arg_values.push_back("");
	}
	void addUnaryArg(const char *unary_arg, const char *abbr = NULL) {
		// Throw an exception on duplicate arguments.
		for (int i = 0; i < _unary_args.size(); ++ i) {
			if (_unary_args[i] == unary_arg) {
				std::stringstream ss;
				ss << "Duplicate unary argument: \"" << unary_arg << "\".";
				throw Exception(ss.str());
			}
			if (_unary_arg_abbrs[i] == unary_arg) {
				std::stringstream ss;
				ss << "Unary argument \"" << unary_arg << "\" matches the abbreviation of another unary argument: \"" << _unary_args[i] << "\".";
				throw Exception(ss.str());
			}
			if (_unary_args[i] == abbr) {
				std::stringstream ss;
				ss << "Unary argument abbreviation \"" << abbr << "\" matches the full name of another unary argument.";
				throw Exception(ss.str());
			}
			if (_unary_arg_abbrs[i] == abbr) {
				std::stringstream ss;
				ss << "Unary argument abbreviation \"" << abbr << "\" matches the abbreviation of another unary argument (\"" << _unary_args[i] << "\").";
				throw Exception(ss.str());
			}
		}
		for (int i = 0; i < _keyword_args.size(); ++ i) {
			if (_keyword_args[i] == unary_arg) {
				std::stringstream ss;
				ss << "Unary argument \"" << unary_arg << "\" matches a keyword argument.";
				throw Exception(ss.str());
			}
			if (_keyword_arg_abbrs[i] == unary_arg) {
				std::stringstream ss;
				ss << "Unary argument \"" << unary_arg << "\" matches abbreviation of keyword argument: \"" << _keyword_args[i] << "\".";
				throw Exception(ss.str());
			}
			if (_keyword_args[i] == abbr) {
				std::stringstream ss;
				ss << "Unary argument abbreviation \"" << abbr << "\" matches the full name of a keyword argument.";
				throw Exception(ss.str());
			}
			if (_keyword_arg_abbrs[i] == abbr) {
				std::stringstream ss;
				ss << "Unary argument abbreviation \"" << abbr << "\" matches the abbreviation of keyword argument\"" << _keyword_args[i] << "\".";
				throw Exception(ss.str());
			}
		}
		std::string unary_arg_str = unary_arg;
		// Throw exception on unary args that contain separator characters.
		for (int ci = 0; ci < _seps.length(); ++ ci) {
			char c = _seps[ci];
			if (unary_arg_str.find(c) != std::string::npos) {
				std::stringstream ss;
				ss << "Unary argument \"" << unary_arg << "\" contains the separator character \"" << c << "\".";
				throw Exception(ss.str());
			}
		}
		std::string abbr_str;
		if (abbr != NULL) abbr_str = abbr;
		_unary_args.push_back(unary_arg_str);
		_unary_arg_abbrs.push_back(abbr_str);
		_unary_args_defined.push_back(false);

	}
	void processArgs(int argc, const char *argv[]) {
		if (argc <= 0) return;
		_exec_name = argv[0];
		std::string arg;
		int ai = 0;
		while (true) {
			argloop: ++ ai;
			if (ai == argc) break;
			arg = argv[ai];
			// See if argument contains any of the key-value separator characters.
			for (int ci = 0; ci < _seps.length(); ++ ci) {
				char c = _seps[ci];
				if (arg.find(c) != std::string::npos) {
					// Extract keyword and value from argument.
					std::string keyword(arg.substr(0,arg.find(c)));
					std::string value(arg.substr(arg.find(c)+1,std::string::npos));
					// See if key matches.
					for (int i = 0; i < _keyword_args.size(); ++ i) {
						if (_keyword_args[i] == keyword || _keyword_arg_abbrs[i] == keyword) {
							if (_keyword_args_defined[i]) {
								if (_redefinition_is_error) {
									// See if this keyword argument has already been redefined.
									int ei;
									for (ei = 0; ei < _errors.size(); ++ ei) {
										if (_errors[ei]->type() == Error::REDEFINITION_OF_KEY) {
											Error::RedefinitionOfKey *key_error = dynamic_cast<Error::RedefinitionOfKey*>(_errors[ei]);
											// Increment the count of that error.
											if (key_error->key() == _keyword_args[i]) {
												key_error->addCount();
												break;
											}
										}
									}
									// Otherwise add a new error.
									if (ei == _errors.size()) {
										Error::RedefinitionOfKey *error = new Error::RedefinitionOfKey(_keyword_args[i].c_str());
										_errors.push_back(error);
									}
								}
							} else _keyword_args_defined[i] = true;
							_keyword_arg_values[i] = value;
							goto argloop;
						}
					}
					{
						Error::UnrecognizedArg *error = new Error::UnrecognizedArg(arg.c_str());
						_errors.push_back(error);
						goto argloop;
					}
				}
			}
			// See if argument matches any unary args.
			for (int i = 0; i < _unary_args.size(); ++ i) {
				if (_unary_args[i] == arg || _unary_arg_abbrs[i] == arg) {
					if (_unary_args_defined[i]) {
						if (_redefinition_is_error) {
							// See if this unary argument has already been redefined.
							int ei;
							for (ei = 0; ei < _errors.size(); ++ ei) {
								if (_errors[ei]->type() == Error::REDEFINITION_OF_UNARY_ARG) {
									Error::RedefinitionOfUnaryArg *unary_arg_error = dynamic_cast<Error::RedefinitionOfUnaryArg*>(_errors[ei]);
									// Increment the count of that redefinition error.
									if (unary_arg_error->unary_arg() == _unary_args[i]) {
										unary_arg_error->addCount();
										break;
									}
								}
							}
							// Otherwise add a new error.
							if (ei == _errors.size()) {
								Error::RedefinitionOfUnaryArg *error = new Error::RedefinitionOfUnaryArg(_unary_args[i].c_str());
								_errors.push_back(error);
							}
						}
					} else _unary_args_defined[i] = true;
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
					if (_keyword_args_defined[i]) {
						if (_redefinition_is_error) {
							// See if this keyword argument has already been redefined (an error has already been created).
							int ei;
							for (ei = 0; ei < _errors.size(); ++ ei) {
								if (_errors[ei]->type() == Error::REDEFINITION_OF_KEY) {
									Error::RedefinitionOfKey *key_error = dynamic_cast<Error::RedefinitionOfKey*>(_errors[i]);
									// Increment the count of that redefinition error.
									if (key_error->key() == _keyword_args[i]) {
										key_error->addCount();
										break;
									}
								}
							}
							// Otherwise add a new error.
							if (ei == _errors.size()) {
								Error::RedefinitionOfKey *error = new Error::RedefinitionOfKey(_keyword_args[i].c_str());
								_errors.push_back(error);
							}
						}
					} else _keyword_args_defined[i] = true;
					_keyword_arg_values[i] = argv[ai];
					goto argloop;
				}
			}
			// Add an "unrecognized argument" error.
			Error::UnrecognizedArg *error = new Error::UnrecognizedArg(arg.c_str());
			_errors.push_back(error);
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
	void safelyAddKeywordArg(const char *keyword, const char *value) {

	}
	// A string of characters which may be used to separate keys from values in a key-value argument.
	std::string _seps;
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
	bool _redefinition_is_error;
};

#endif
