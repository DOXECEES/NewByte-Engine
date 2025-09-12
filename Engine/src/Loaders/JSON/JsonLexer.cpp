// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "JsonLexer.hpp"

namespace nb
{
    namespace Loaders
    {
        JsonLexer::JsonLexer(const std::filesystem::path &path)
        {
            setPath(path);
        }

        JsonLexer::JsonLexer(const std::string &json) noexcept
            : data(std::move(json))
            , size(data.size())
        {}

        JsonLexer::~JsonLexer() noexcept
        {
            if(file.is_open())
                file.close();
        }

        void JsonLexer::setPath(const std::filesystem::path &path)
        {
            if(file.is_open())
            {
                throw std::runtime_error("file  is already open");
                return;
            }

            file.open(path, std::ios::in);
            if (!file.is_open())
            {
                throw std::runtime_error("cannot open file");
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            data = buffer.str();
            size = data.size();        
        }

        void JsonLexer::setJson(const std::string &json)
        {
            data = std::move(json);
            size = data.size();
        }

        JsonLexer::Token JsonLexer::nextToken()
        {
            if (!isInRange())
            {
                throw std::runtime_error("lexer out of range");
                return createToken(TokenType::NONE, "");
            }

            switch (data[pos])
            {
            case '{':
                return createToken(TokenType::OBJECT_BEGIN);
            case '}':
                return createToken(TokenType::OBJECT_END);
            case '"':
                return createToken(TokenType::STRING_SEPARATOR, "\"");
            case ':':
                return createToken(TokenType::NAME_SEPARATOR, ":");
            case ',':
                return createToken(TokenType::PAIR_SEPARATOR, ",");
            case '[':
                return createToken(TokenType::ARRAY_BEGIN, "[");
            case ']':
                return createToken(TokenType::ARRAY_END, "]");
            case '\t':
            case '\n':
            case '\r':
            case ' ':
                return createToken(TokenType::WHITESPACE);
            default:
                return parseOther();
            }
        }

        JsonLexer::Token JsonLexer::parseOther() noexcept
        {
            if ((std::isdigit(data[pos]) || data[pos] == '.' || data[pos] == '-') && data[pos - 1] != '"')
            {
                return parseNumber();
            }
            else if ((data[pos] == 'f' || data[pos] == 't' || data[pos] == 'n') && data[pos - 1] != '"')
            {
                return parseLogicExpr();
            }
            else
            {
                return parseString();
            }
        }

        JsonLexer::Token JsonLexer::parseLogicExpr() noexcept
        {
            if (data.substr(pos, 4) == "true")
            {
                pos += 4;
                return {TokenType::JTRUE, "1"};
            }
            else if (data.substr(pos, 5) == "false")
            {
                pos += 5;
                return {TokenType::JFALSE, "0"};
            }
            else if (data.substr(pos, 4) == "null")
            {
                pos += 4;
                return {TokenType::NILL, ""};
            }
            else
            {
                pos++;
                return {TokenType::NONE, ""};
            }
        }

        JsonLexer::Token JsonLexer::parseNumber() noexcept
        {
            size_t startPos = pos;
            pos++;
            while (isInRange() && std::isdigit(data[pos]) || data[pos] == '.')
            {
                pos++;
            }

            return {TokenType::NUMBER, data.substr(startPos, pos - startPos)};
        }

        JsonLexer::Token JsonLexer::parseString() noexcept
        {
            size_t startPos = pos;
            pos++;
            while (isInRange() && data[pos] != '"')
            {
                if (data[pos] == '\\')
                {
                    pos++;
                }
                pos++;
            }
            return {TokenType::STRING, data.substr(startPos, pos - startPos)};
        }
    };
};