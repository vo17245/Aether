#pragma once
#include "Aether/Core.h"
#include <array>
#include <cstring>
#include <optional>
#include <string>
#include <winnt.h>

namespace Aether {
namespace Editor {
class UIAttributeParser
{
    enum class TokenType
    {
        Identifier,
        Number,
        String,
        Delimiter,
    };
    struct Token
    {
        TokenType type;
        std::string value;
        Token(TokenType _type, const std::string& _value)
            : type(_type), value(_value)
        {}
        static Token Delimiter(const std::string& value)
        {
            return Token(TokenType::Delimiter, value);
        }
        static Token Identifier(const std::string& value)
        {
            return Token(TokenType::Identifier, value);
        }
        static Token Number(const std::string& value)
        {
            return Token(TokenType::Number, value);
        }
        static Token String(const std::string& value)
        {
            return Token(TokenType::String, value);
        }
    };
    static constexpr const std::array<const char*, 5> delimiters = {",", "(", ")", "[", "]"};

public:
    bool IsDelimiter(const std::string& s)
    {
        for (auto d : delimiters)
        {
            if (strcmp(s.c_str(), d) == 0)
            {
                return true;
            }
        }
        return false;
    }
    bool IsNumber(const std::string& s)
    {
        for (auto c : s)
        {
            if (!isdigit(c) && c != '.')
            {
                return false;
            }
        }
        return true;
    }
    bool IsString(const std::string& s)
    {
        return s.front() == '"' && s.back() == '"';
    }
    bool IsIdentifier(const std::string& s)
    {
        return !IsNumber(s) && !IsString(s);
    }
    std::vector<Token> ParseToken(const std::string& code)
    {
        std::vector<Token> tokens;
        std::string token;
        for (auto c : code)
        {
            if (c == ' ') continue;
            if (IsDelimiter(std::string(1, c)))
            {
                if (!token.empty())
                {
                    if (IsNumber(token))
                    {
                        tokens.push_back(Token::Number(token));
                    }
                    else if (IsString(token))
                    {
                        tokens.push_back(Token::String(token));
                    }
                    else if (IsIdentifier(token))
                    {
                        tokens.push_back(Token::Identifier(token));
                    }

                    token.clear();
                }
                tokens.push_back(Token::Delimiter(std::string(1, c)));
            }
            else
            {
                token.push_back(c);
            }
        }
        if (!token.empty())
        {
            if (IsNumber(token))
            {
                tokens.push_back(Token::Number(token));
            }
            else if (IsString(token))
            {
                tokens.push_back(Token::String(token));
            }
            else if (IsIdentifier(token))
            {
                tokens.push_back(Token::Identifier(token));
            }
        }
        return tokens;
    }
    std::optional<Vec4> GetVec4(const std::string& code)
    {
        auto tokens = ParseToken(code);
        if (tokens[0].type != TokenType::Delimiter)
        {
            return std::nullopt;
        }
        if (tokens[0].value != "(")
        {
            return std::nullopt;
        }
        //(
        if (tokens[1].type != TokenType::Number)
        {
            return std::nullopt;
        }
        //(1
        if (tokens[2].type != TokenType::Delimiter)
        {
            return std::nullopt;
        }
        if (tokens[2].value != ",")
        {
            return std::nullopt;
        }
        //(1,
        if (tokens[3].type != TokenType::Number)
        {
            return std::nullopt;
        }
        //(1,1
        if (tokens[4].type != TokenType::Delimiter)
        {
            return std::nullopt;
        }
        if (tokens[4].value != ",")
        {
            return std::nullopt;
        }
        //(1,1,
        if (tokens[5].type != TokenType::Number)
        {
            return std::nullopt;
        }

        //(1,1,1
        if (tokens[6].type != TokenType::Delimiter)
        {
            return std::nullopt;
        }
        if (tokens[6].value != ",")
        {
            return std::nullopt;
        }
        //(1,1,1,
        if (tokens[7].type != TokenType::Number)
        {
            return std::nullopt;
        }
        //(1,1,1,1
        if (tokens[8].type != TokenType::Delimiter)
        {
            return std::nullopt;
        }
        if (tokens[8].value != ")")
        {
            return std::nullopt;
        }
        //(1,1,1,1)
        return Vec4(std::stof(tokens[1].value),
                    std::stof(tokens[3].value),
                    std::stof(tokens[5].value),
                    std::stof(tokens[7].value));
    }
};
}
} // namespace Aether::Editor