// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "JsonParser.hpp"

namespace nb
{
    namespace Loaders
    {
        Node JsonParser::parse()
        {
            using jst = JsonLexer::TokenType;

            switch (token.type)
            {
            case jst::OBJECT_BEGIN:
                return parseObj();
            case jst::ARRAY_BEGIN:
                return parseArray();
            case jst::STRING_SEPARATOR:
                return parseValue();
            case jst::NUMBER:
                return parseNumber();
            case jst::JTRUE:
            case jst::JFALSE:
            case jst::NILL:
                return parseLogicExpr();
            default:
                throw std::runtime_error("Invalid token");
                return nullptr;
            }
        }

        Node::Map JsonParser::parseObj()
        {
            using jst = JsonLexer::TokenType;

            Node::Map map;

            while (token.type != jst::OBJECT_END && token.type != jst::NONE)
            {
                std::string key = std::move(parseKey());

                Node val = std::move(parsePair());
                map.try_emplace(std::move(key), std::move(val));

                token = getTokenNoWhiteSpaces();
                if (token.type != jst::PAIR_SEPARATOR && token.type != jst::OBJECT_END)
                {
                    throw std::runtime_error("Expected ',' or '}'");
                    return {};
                }
            }
            return map;
        }

        Node JsonParser::parsePair()
        {
            using jst = JsonLexer::TokenType;
            token = getTokenNoWhiteSpaces();

            if (token.type != jst::NAME_SEPARATOR)
            {
                throw std::runtime_error("expected NAME_SEPARATOR");
                return {};
            }

            token = getTokenNoWhiteSpaces();
            return parse();
        }

        Node JsonParser::parseValue()
        {
            using jst = JsonLexer::TokenType;
            token = getTokenNoWhiteSpaces();

            switch (token.type)
            {
            case jst::STRING:
            {
                std::string str = std::move(token.value);

                token = getTokenNoWhiteSpaces();
                if (token.type != jst::STRING_SEPARATOR)
                {
                    throw std::runtime_error("expected STRING_SEPARATOR");
                    return {};
                }

                return Node(str);
            }
            default:
                throw std::runtime_error("expected STRING");
                return {};
            }
        }

        std::string JsonParser::parseKey()
        {
            using jst = JsonLexer::TokenType;
            token = getTokenNoWhiteSpaces();

            if (token.type != jst::STRING_SEPARATOR)
            {
                throw std::runtime_error("expected STRING_SEPARATOR");
                return {};
            }
            token = getTokenNoWhiteSpaces();

            if (token.type != jst::STRING)
            {
                throw std::runtime_error("expected STRING");
                return {};
            }
            std::string key = std::move(token.value);

            token = getTokenNoWhiteSpaces();
            if (token.type != jst::STRING_SEPARATOR)
            {
                throw std::runtime_error("expected last STRING_SEPARATOR");
                return {};
            }

            return key;
        }

        Node::Array JsonParser::parseArray()
        {

            using jst = JsonLexer::TokenType;
            Node::Array arr;

            jst arrayType;

            token = getTokenNoWhiteSpaces();

            if (token.type != jst::STRING_SEPARATOR && token.type != jst::NUMBER && token.type != jst::OBJECT_BEGIN)
            {
                throw std::runtime_error("Incorrect array type");
                return {};
            }

            arrayType = token.type;

            while (token.type != jst::ARRAY_END)
            {
                if (arrayType != token.type)
                {
                    throw std::runtime_error("Enters incorrect type to array");
                    return {};
                }

                arr.emplace_back(parse());

                token = getTokenNoWhiteSpaces();
                if (token.type == jst::PAIR_SEPARATOR)
                {
                    token = getTokenNoWhiteSpaces();
                }
                else if (token.type != jst::ARRAY_END)
                {
                    throw std::runtime_error("Expected ',' or ']'");
                    return {};
                }
            }

            return arr;
        }

        Node JsonParser::parseLogicExpr()
        {
            switch (token.type)
            {
            case JsonLexer::TokenType::JTRUE:
                return Node(true);
            case JsonLexer::TokenType::JFALSE:
                return Node(false);
            case JsonLexer::TokenType::NILL:
                return Node(nullptr);
            default:
                throw std::runtime_error("Invalid logic expression token");
                break;
            }
        }

        JsonLexer::Token JsonParser::getTokenNoWhiteSpaces()
        {
            JsonLexer::Token newToken = lexer.nextToken();
            while (newToken.type == JsonLexer::TokenType::WHITESPACE)
            {
                newToken = lexer.nextToken();
            }
            return newToken;
        }

        Node JsonParser::parseNumber()
        {
            if (token.value.find('.') == std::string::npos)
                return Node(std::stoi(token.value));
            else
                return Node(std::stof(token.value));
        }
    };
};
