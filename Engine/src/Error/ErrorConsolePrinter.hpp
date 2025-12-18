#ifndef SRC_ERROR_ERRORCONSOLELOGGER_HPP
#define SRC_ERROR_ERRORCONSOLELOGGER_HPP

#include "Error/IErrorPrinter.hpp"

namespace nb
{
    namespace Error
    {
        class ErrorConsolePrinter : public IErrorPrinter 
        {
        public:
            ErrorConsolePrinter() noexcept = default;
            ~ErrorConsolePrinter() noexcept override {};
            
            void print(const ErrorMessage& msg) const noexcept override;

        private:
        
        };
    };
};

#endif