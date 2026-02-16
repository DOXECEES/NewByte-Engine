#include "ErrorConsolePrinter.hpp"

#include <windows.h>
#include <iostream>


namespace nb
{
	namespace Error
	{
		void ErrorConsolePrinter::print(const ErrorMessage& msg) const noexcept
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole == INVALID_HANDLE_VALUE)
            {
                return; 
            }

            WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; 
            if ((msg.type & Type::INFO))
            {
                color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            }
            if ((msg.type & Type::WARNING))
            {
                color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; 
            }
            if ((msg.type & Type::FATAL))
            {
                color = FOREGROUND_RED | FOREGROUND_INTENSITY;
            }
            SetConsoleTextAttribute(hConsole, color);

            auto tt = std::chrono::system_clock::to_time_t(msg.time);
            std::tm local_tm;
            localtime_s(&local_tm, &tt);
            char timeBuf[16];
            std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &local_tm);

            std::string output = "[" + std::string(timeBuf) + "] ";

            if ((msg.type & Type::INFO))
            {
                output += "[INFO] ";
            }
            if ((msg.type & Type::WARNING))
            {
                output += "[WARNING] ";
            }
            if ((msg.type & Type::FATAL))
            {
                output += "[FATAL] ";
            }

            output.append(msg.message); 
            output.append("\n");

            for (const auto& [key, value] : msg.data)
            {
                output += "    " + key + ": " + value + "\n";
            }

            DWORD written = 0;
            WriteConsoleA(hConsole, output.c_str(), static_cast<DWORD>(output.size()), &written, NULL);

            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        
    };
};

