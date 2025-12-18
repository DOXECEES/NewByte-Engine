#ifndef NB_SRC_ERROR_IERRORPRINTER_HPP
#define NB_SRC_ERROR_IERRORPRINTER_HPP

#include "Error/ErrorManager.hpp"

namespace nb 
{
    namespace Error
    {
        class IErrorPrinter
        {
        public:
            virtual ~IErrorPrinter() noexcept = default;
            virtual void print(const ErrorMessage& msg) const noexcept = 0;
        };
    };
    
}; 

#endif 
