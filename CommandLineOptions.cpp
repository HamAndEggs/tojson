#include "CommandLineOptions.h"
#include "Tools.h"

#include <assert.h>
#include <memory.h>

#include <stdexcept>
#include <iostream>


CommandLineOptions::CommandLineOptions(const std::string& pUsageHelp):mUsageHelp(pUsageHelp)
{// Always add this, everyone does. BE rude not to. ;)
	AddArgument('h',"help","Display this help and exit");
}

void CommandLineOptions::AddArgument(char pArg,const std::string& pLongArg,const std::string& pHelp,int pArgumentOption,std::function<void(const std::string& pOptionalArgument)> pCallback)
{
	if( mArguments.find(pArg) != mArguments.end() )
	{
        throw std::runtime_error("At: " + std::to_string(__LINE__) + " In " + std::string(__FILE__) + " : " + std::string("class CommandLineOptions: Argument ") + pArg + " has already been registered, can not contine");        
	}
	
	mArguments.emplace(pArg,Argument(pLongArg,pHelp,pArgumentOption,pCallback));
}

bool CommandLineOptions::Process(int argc, char *argv[])
{
	std::vector<struct option> longOptions;
	std::string shortOptions;

	// Build the data for the getopt_long function that will do all the work for us.
	for( auto& opt : mArguments)
	{
		shortOptions += opt.first;
		if( opt.second.mArgumentOption == required_argument )
		{
			shortOptions += ":";
		}
		// Bit of messing about because mixing c code with c++
		struct option newOpt = {opt.second.mLongArgument.c_str(),opt.first,nullptr,opt.first};
		longOptions.emplace_back(newOpt);
	}
	struct option emptyOpt = {NULL, 0, NULL, 0};
	longOptions.emplace_back(emptyOpt);

	int c,oi;
	while( (c = getopt_long(argc,argv,shortOptions.c_str(),longOptions.data(),&oi)) != -1 )
	{
		auto arg = mArguments.find(c);
		if( arg == mArguments.end() )
		{// Unknow option, print help and bail.
			std::cout << "Unknown option \'" << c << "\' found.\n";
			PrintHelp();
			return false;
		}
		else
		{
			arg->second.mIsSet = true;
			std::string optionalArgument;
			if( optarg )
			{// optarg is defined in getopt_code.h 
				optionalArgument = optarg;
			}

			if( arg->second.mCallback != nullptr )
			{
				arg->second.mCallback(optionalArgument);
			}
		}
	};

	// See if help was asked for.
	if( IsSet('h') )
	{
		PrintHelp();
		return false;
	}

	return true;
}

bool CommandLineOptions::IsSet(char pShortOption)const
{
	return mArguments.at(pShortOption).mIsSet;
}

bool CommandLineOptions::IsSet(const std::string& pLongOption)const
{
	for( auto& opt : mArguments)
	{
		if( CompareNoCase(opt.second.mLongArgument,pLongOption) )
		{
			return opt.second.mIsSet;
		}
	}

	return false;
}

void CommandLineOptions::PrintHelp()const
{
	std::cout << mUsageHelp << "\n";

	std::vector<char> shortArgs;
	std::vector<std::string> longArgs;
	std::vector<std::string> descriptions;

	for( auto& opt : mArguments)
	{
		shortArgs.push_back(opt.first);
		descriptions.push_back(opt.second.mHelp);
		if( opt.second.mArgumentOption == required_argument )
		{
			longArgs.push_back("--" + opt.second.mLongArgument + "=arg");
		}
		else if( opt.second.mArgumentOption == optional_argument )
		{
			longArgs.push_back("--" + opt.second.mLongArgument + "[=arg]");
		}
		else
		{
			longArgs.push_back("--" + opt.second.mLongArgument);
		}
	}

	// Now do a load of formatting of the output.
	size_t DescMaxSpace = 0;
	for(auto lg : longArgs)
	{
		size_t l = 5 + lg.size(); // 5 == 2 spaces + -X + 1 for space for short arg.
		if( DescMaxSpace < l )
			DescMaxSpace = l;
	}

	DescMaxSpace += 4; // Add 4 spaces for formatting.
	for(size_t n=0;n<shortArgs.size();n++)
	{
		std::string line = "  -";
		line += shortArgs[n];
		line += " ";
		line += longArgs[n];
		line += " ";
		std::cout << line;

		size_t space = DescMaxSpace - line.size();
		const std::vector<std::string> lines = SplitString(descriptions[n],"\n");
		for(auto line : lines)
		{
			std::cout << std::string(space,' ') << line << '\n';
			space = DescMaxSpace + 2;// For subsequent lines.
		}
	}
}
