#ifndef NBSTL_SRC_CHARUTILS_HPP
#define NBSTL_SRC_CHARUTILS_HPP

namespace nbstl 
{
    inline bool isNumeric(unsigned char symbol) noexcept
    {
        return symbol >= '0' && symbol <= '9';
    }

    inline bool isUpper(unsigned char symbol) noexcept
    {
        return symbol >= 'A' && symbol <= 'Z';
    }

    inline bool isLower(unsigned char symbol) noexcept
    {
        return symbol >= 'a' && symbol <= 'z';
    }

    inline bool isSpace(unsigned char symbol) noexcept
    {
        return (symbol >= '\t' && symbol <= '\r') || symbol == ' ';
    }

    inline bool isAlpha(unsigned char symbol) noexcept
    {
        return  isUpper(symbol) || isLower(symbol);
    }

    inline bool isAlphaNumeric(unsigned char symbol) noexcept
    {
        return isNumeric(symbol) || isAlpha(symbol);
    }

}; 

#endif 
