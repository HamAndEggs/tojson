#ifndef CommandLineOptions_h__
#define CommandLineOptions_h__

#include <getopt.h>

#include <string>
#include <map>
#include <functional>

class CommandLineOptions
{
public:
	CommandLineOptions(const std::string& pUsageHelp);

	void AddArgument(char pArg,const std::string& pLongArg,const std::string& pHelp,int pArgumentOption = no_argument,std::function<void(const std::string& pOptionalArgument)> pCallback = nullptr);
	bool Process(int argc, char *argv[]);
	bool IsSet(char pShortOption)const;
	bool IsSet(const std::string& pLongOption)const;
	void PrintHelp()const;

private:
	struct Argument
	{
		Argument(const std::string& pLongArgument,const std::string& pHelp,const int pArgumentOption,std::function<void(const std::string& pOptionalArgument)> pCallback):
			mLongArgument(pLongArgument),
			mHelp(pHelp),
			mArgumentOption(pArgumentOption),
			mCallback(pCallback),
			mIsSet(false)
		{

		}

		const std::string mLongArgument;
		const std::string mHelp;
		const int mArgumentOption;
		std::function<void(const std::string& pOptionalArgument)> mCallback;
		bool mIsSet;	//!< This will be true if the option was part of the commandline. Handy for when you just want to know true or false.
	};
	
	const std::string mUsageHelp;
	std::map<char,Argument> mArguments;
	

};

#endif // CommandLineOptions_h__
