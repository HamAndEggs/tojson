#ifndef Tools_h__
#define Tools_h__

#include <memory.h>
#include <assert.h>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Adds line and source file. There is a c++20 way now that is better. I need to look at that.
 */
#define TOJSON_THROW(THE_MESSAGE__)	{throw std::runtime_error("At: " + std::to_string(__LINE__) + " In " + std::string(__FILE__) + " : " + std::string(THE_MESSAGE__));}

/**
 * @brief Does an ascii case insensitive test within the full string or a limited start of the string.
 * 
 * @param pA 
 * @param pB 
 * @param pLength If == 0 then length of second string is used. If first is shorter, will always return false.
 * @return true 
 * @return false 
 */
inline bool CompareNoCase(const char* pA,const char* pB,size_t pLength = 0)
{
    assert( pA != nullptr || pB != nullptr );// Note only goes pop if both are null.
// If either or both NULL, then say no. A bit like a divide by zero as null strings are not strings.
    if( pA == nullptr || pB == nullptr )
        return false;

// If same memory then yes they match, doh!
    if( pA == pB )
        return true;

    if( pLength == 0 )
        pLength = strlen(pB);

    while( (*pA != 0 || *pB != 0) && pLength > 0 )
    {
        // Get here are one of the strings has hit a null then not the same.
        // The while loop condition would not allow us to get here if both are null.
        if( *pA == 0 || *pB == 0 )
        {// Check my assertion above that should not get here if both are null. Note only goes pop if both are null.
            assert( pA != NULL || pB != NULL );
            return false;
        }

        if( tolower(*pA) != tolower(*pB) )
            return false;

        pA++;
        pB++;
        pLength--;
    };

    // Get here, they are the same.
    return true;
}

inline bool CompareNoCase(const std::string& pA,const std::string& pB,size_t pLength = 0)
{
    return CompareNoCase(pA.c_str(),pB.c_str(),pLength);
}

inline std::vector<std::string> SplitString(const std::string& pString, const char* pSeperator)
{
    std::vector<std::string> res;
    for (size_t p = 0, q = 0; p != pString.npos; p = q)
	{
		const std::string part(pString.substr(p + (p != 0), (q = pString.find(pSeperator, p + 1)) - p - (p != 0)));
		if( part.size() > 0 )
		{
	        res.push_back(part);
		}
	}
    return res;
}


#endif //#ifndef Tools_h__