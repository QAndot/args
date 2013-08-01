class Args(object):
	def __init__(self):
		self._seps = '='
		self._redefIsError = True
		self._execName = None
		self._keywordArgs = []
		self._unaryArgs = []

	# Return the name of the executable.
	def execName():
		return self._execName

	# Sets the string containing the separator characters for keyword arguments.
	def setSeps(seps):
		self._seps = seps

	# Returns the string containing characters which may be used to separate keyword arguments.
	def seps():
		return self._seps

	# Sets whether or not redefinition of a command-line argument is an error.
	def redefIsError(redefIsError):
		self._redefIsError = isError

	# Returns whether or not redefinition of a command-line argument is an error.
	def redefIsError():
		return self._redefIsError

	def addKeywordArg(argName, argAbbr):
		for key in self._keywordArgs:

