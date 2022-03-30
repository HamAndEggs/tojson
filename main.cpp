#include "CommandLineOptions.h"
#include "Tools.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>

    const std::string mHelp = R"(Usage: tojson [OPTION]
Turn the input file (or as part of a pipe), into a json formatted string.
If input string is not specified then will expect data on the input stream.
If output file is not specified then will write to std::cout.
)";

class PipeToJson : public CommandLineOptions
{
    std::string mInputFileName;
    size_t mDepth = 0;
    size_t mNumLines = std::numeric_limits<size_t>::max();
    size_t mTabSpaces = 2;
    int indent = 0;

    bool GetPretty()const{return IsSet('p');}
    bool GetAsOneDocument()const{return IsSet('o');}
    bool GetSkipEmptyLines()const{return IsSet('s');}

    std::string GetNewline(bool inString = false)
    {
        if(GetPretty() && inString == false )
        {
            return "\n" + std::string(indent,' ');
        }
        else if( GetAsOneDocument() == false )
        {
            return "\n";
        }
        return "";
    }

    std::string ConvertLineToJson(const std::string& rawLine,const std::vector<std::string>& columns)
    {
        std::stringstream jsonLine;
        jsonLine << "{";

        const std::vector<std::string> data = SplitString(rawLine," ");
        if( GetSkipEmptyLines() && data.size() == 0 )
        {
            return "";
        }

        if( data.size() == 0 )
        {
            TOJSON_THROW("Input line from the stream did not contain any data");
        }

        const size_t lastColumn = columns.size()-1;
        // Grab everything upto but not including the last column.
        for( size_t n = 0 ; n < lastColumn ; n++ )
        {
            if( n < data.size() )
            {
                jsonLine << "\"" << columns[n] << "\":\"" << data[n] << "\",";
            }
        }
        // Now do the last column, if there is data. All remaining data is added to the last column.
        std::string pretext = "\"" + columns[lastColumn] + "\":\"";
        for( size_t n = lastColumn ; n < data.size() ; n++ )
        {
            jsonLine << pretext << data[n];
            pretext = " ";
        }
        jsonLine << "\"}";

        return jsonLine.str();
    }

    std::string FormatJsonLine(const std::string& JSONLine)
    {
        if( GetPretty() )
        {
            // After each opening bracket, add a newline and increase the indent.
            // After each value / key value add a new line.
            // After each closing bracket, decrease indent. Add newline, but after a comma if there is one.
            std::string formatted;
            bool inString = false;

            // Don't do the last char so we can easily check the next char. Otherwise could go pop. Last should always be } or ]
            for( size_t n = 0 ; n < JSONLine.size()-1 ; n++ )
            {
                const char c = JSONLine[n];
                const char next = JSONLine[n];
                assert( c != '\n' );
                formatted += c;
                if( c == '{' || c == '[' )
                {
                    indent += mTabSpaces;
                    formatted += GetNewline(inString);
                }
                else if( c == '}' || c == ']' )
                {
                    indent -= mTabSpaces;
                    if( next == ',' )
                    {
                        formatted += ',';
                        n++;
                    }
                    formatted += GetNewline(inString);
                }
                else if( c == ',')
                {
                    formatted += GetNewline(inString);
                }
                else if( c == '\"' )
                {
                    inString = !inString;
                }
            }
            indent -= mTabSpaces;
            formatted += GetNewline(inString);
            formatted += JSONLine.back();
            return formatted;
        }

        return JSONLine;
    }

public:
    PipeToJson() : CommandLineOptions(mHelp)
    {
        AddArgument('i',"in","Sets the input file to used instead of standard input pipe",required_argument,
            [this](const std::string& pOptionalArgument)
            {
                mInputFileName = pOptionalArgument;
            }
        );

        AddArgument('d',"depth","How many of the columns will used as object groups, if zero, which is the default, then all data will be in arrays.",required_argument,
            [this](const std::string& pOptionalArgument)
            {
                mDepth = std::stoul(pOptionalArgument);
            }
        );

        
        AddArgument('n',"num-lines","How many rows of the input data that will be converted to json. Default is to output all of them.",required_argument,
            [this](const std::string& pOptionalArgument)
            {
                mNumLines = std::stoul(pOptionalArgument);
            }
        );

        AddArgument('p',"pretty-printing","If set, will print with indentation and carrage returns.",no_argument);
        AddArgument('o',"one-document","If set, all output will be placed within one document. By default, each line is it's own document.",no_argument);
        AddArgument('s',"skip blank lines","If set, any empty line will be skipped. If not set an exception will be thrown on empty lines. Default is to fail on blank lines.",no_argument);
    }

    int ConvertToString()
    {
        // See if we have specified a file, if not then use the input stream.
        std::ifstream file;
        if( mInputFileName.size() > 0 )
        {
            file.open(mInputFileName);
            if( file.is_open() )
            {
                std::cin.rdbuf(file.rdbuf());
            }
        }

        // input stream set, lets do it.

        // First always assume first line is the column headers for the json members. (later add this as an option)
        std::string line;
        std::getline(std::cin,line);

        if( line.size() == 0 )
        {   // TODO:: throw exception.
            return EXIT_FAILURE;
        }

        const std::vector<std::string> columns = SplitString(line," ");

        if( columns.size() == 0 )
        {   // TODO:: throw exception.
            return EXIT_FAILURE;
        }

        const char* newline = (GetPretty()?"\n":"");
        const char* comma = (GetAsOneDocument()?",":"");
        const char* optionalComma = "";

        if( GetAsOneDocument() )
        {
            indent = mTabSpaces;
            std::cout << "{";
        }

        while( std::cin.eof() == false && mNumLines > 0 )
        {
            std::getline(std::cin,line);

            if( line.size() == 0 )
            {   
                // No more from stream, this happens when processing the pipe and not the file.
                break;
            }

            const std::string jsonLine = ConvertLineToJson(line,columns);
            if( jsonLine.size() > 0 )
            {
                // Now output to std::out
                // This is where we'll add formatting if asked for and either create one json document perline or all the output as one json document.
                std::cout << optionalComma << GetNewline() << FormatJsonLine(jsonLine);
                optionalComma = comma;// Done like this as we don't know when the data will end.
                mNumLines--;
            }
        }

        if( GetAsOneDocument() )
        {
            std::cout << newline << "}" << newline;
        }

        return EXIT_SUCCESS;
    }

};

int main(int argc, char *argv[])
{
    std::string inputFileName;

    PipeToJson Converter;
    if( Converter.Process(argc,argv) )
    {
        return Converter.ConvertToString();
    }

    return EXIT_FAILURE;
}
