#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <iostream>
#include <sstream>
#include "error.h"

inline bool OnlySpaces(const std::string& s) {
    for (auto c : s) {
        if (!std::isspace(c)) {
            return false;
        }
    }
    return true;
}

struct SymbolToken {
    std::string name;
    SymbolToken(const std::string& s) {
        name = s;
    }
    SymbolToken(std::string&& s) {
        name = s;
    }
    SymbolToken(char c) {
        name.clear();
        name += c;
    }
    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    }
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const {
        return true;
    }
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    }
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    ConstantToken(int x) {
        value = x;
    }

    ConstantToken(const std::string& s) {
        value = 0;
        int mul = 1;
        for (size_t i = 0; i < s.size(); i++) {
            if (std::isdigit(s[s.size() - i - 1])) {
                value += (s[s.size() - i - 1] - '0') * mul;
                mul *= 10;
            } else {
                if (s[s.size() - i - 1] == '-') {
                    value *= -1;
                }
            }
        }
    }
    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    }
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

inline bool IsGoodForSymbolTocken(char c) {
    std::string s = "<=>?!-*";
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') ||
           s.find(c) != std::string::npos;
}

class Tokenizer {
    Token last_token_;
    bool is_end_;
    bool real_end_;
    std::istream* stream_;
    std::string current_;

public:
    Tokenizer(std::istream* in) : last_token_(SymbolToken("")) {
        stream_ = in;
        is_end_ = false;
        real_end_ = false;
        Next();
    }

    bool IsEnd() {
        return real_end_;
    }

    void Next() {
        if (is_end_ && OnlySpaces(current_)) {
            real_end_ = true;
        } else {
            Read();
        }
    }

    Token GetToken() {
        return last_token_;
    }

private:
    void Read() {

        char c = ' ';
        std::string tmp;
        for (auto smb : current_) {
            if (!std::isspace(smb)) {
                tmp += smb;
            }
        }
        current_ = std::move(tmp);

        if (current_.empty()) {
            while (true) {
                auto tmp = ReadNext();
                if (!tmp.first) {
                    real_end_ = true;
                    return;
                } else {
                    c = tmp.second;
                }
                if (!std::isspace(c)) {
                    break;
                }
            }
        } else {
            c = current_[0];
            current_.clear();
        }

        if (c == '(') {
            last_token_ = BracketToken::OPEN;
            return;
        }
        if (c == ')') {
            last_token_ = BracketToken::CLOSE;
            return;
        }
        if (c == '.') {
            last_token_ = DotToken();
            return;
        }
        if (c == '\'') {
            last_token_ = QuoteToken();
            return;
        }

        // so, if we here, tocken is constant or symbol
        std::string cur;
        cur += c;
        if (c == '+' || c == '-') {
            auto tmp = ReadNext();
            if (!tmp.first) {
                last_token_ = SymbolToken(cur);
                is_end_ = true;
                return;
            } else {
                c = tmp.second;
            }

            cur += c;
            if (std::isdigit(c)) {
                ReadConstant(cur);
                last_token_ = ConstantToken(cur);
            } else if (IsGoodForSymbolTocken(c)) {
                ReadSymbol(cur);
                last_token_ = SymbolToken(cur);
            } else {
                current_.clear();
                current_ += cur[1];
                last_token_ = SymbolToken(cur[0]);
            }
        } else if (std::isdigit(c)) {
            ReadConstant(cur);
            last_token_ = ConstantToken(cur);
        } else if (IsGoodForSymbolTocken(c)) {
            ReadSymbol(cur);
            last_token_ = SymbolToken(cur);
        } else if (std::isspace(c)) {
        } else {
            throw SyntaxError("Bad char\n");
        }
    }

    void ReadConstant(std::string& cur) {
        char c = ' ';
        auto tmp = ReadNext();
        if (!tmp.first) {
            is_end_ = true;
            return;
        } else {
            c = tmp.second;
        }
        while (std::isdigit(c)) {
            cur += c;
            auto tmp = ReadNext();
            if (!tmp.first) {
                is_end_ = true;
                return;
            } else {
                c = tmp.second;
            }
        }
        current_.clear();
        current_ += c;
    }
    void ReadSymbol(std::string& cur) {
        char c = ' ';
        auto tmp = ReadNext();
        if (!tmp.first) {
            is_end_ = true;
            return;
        } else {
            c = tmp.second;
        }
        while (IsGoodForSymbolTocken(c)) {
            cur += c;
            auto tmp = ReadNext();
            if (!tmp.first) {
                is_end_ = true;
                return;
            } else {
                c = tmp.second;
            }
        }
        current_.clear();
        current_ += c;
    }

    std::pair<bool, char> ReadNext() {
        char c = stream_->peek();
        if (c != EOF) {
            stream_->get();
            return std::pair<bool, char>(true, c);
        } else {
            return std::pair<bool, char>(false, c);
        }
    }
};


