#pragma once

#include <memory>
#include <vector>

#include "object.h"
#include "tokenizer.h"

void RemoveNull(std::shared_ptr<Object>& ptr);

class Reader {
    std::vector<Token> tockens_;
    size_t current_tocken_;
    std::shared_ptr<Object> current_obj_;

public:
    Reader(Tokenizer* tokenizer) {
        while (!tokenizer->IsEnd()) {
            tockens_.push_back(tokenizer->GetToken());
            tokenizer->Next();
        }
        current_tocken_ = 0;
    }

    std::shared_ptr<Object> Read() {
        CheckInput();
        if (tockens_.size() == 1) {
            return MakeObj(tockens_[0]);
        } else if (tockens_.size() == 2 && tockens_[0].index() == 1 && tockens_[1].index() == 1 &&
                   std::get<BracketToken>(tockens_[0]) == BracketToken::OPEN &&
                   std::get<BracketToken>(tockens_[1]) == BracketToken::CLOSE) {
            return nullptr;
        } else {
            current_tocken_++;
            std::shared_ptr<Object> ptr = ReadList();
            RemoveNull(ptr);
            if (ptr) {
                return ptr;
            } else {
                return std::make_shared<Cell>();
            }
        }
    }

private:
    // current token should be after '('. reads to ')', recursive called in case of '('
    std::shared_ptr<Cell> ReadList() {
        std::shared_ptr<Cell> begin = std::make_shared<Cell>();
        std::shared_ptr<Object> current = begin;

        while (current_tocken_ < tockens_.size() &&
               !(tockens_[current_tocken_].index() == 1 &&
                 std::get<BracketToken>(tockens_[current_tocken_]) == BracketToken::CLOSE)) {

            switch (tockens_[current_tocken_].index()) {
                case 0:
                case 2: {
                    // symbol or number
                    As<Cell>(current)->SetFirst(MakeObj(tockens_[current_tocken_]));
                    if (tockens_[current_tocken_ + 1].index() != 4) {
                        // next is not dot
                        As<Cell>(current)->SetSecond(std::make_shared<Cell>());
                        current = std::dynamic_pointer_cast<Cell>(As<Cell>(current)->GetSecond());
                        current_tocken_++;
                    } else {
                        // next is dot
                        if (tockens_[current_tocken_ + 2].index() == 0 ||
                            // sybmol or number is after dot
                            tockens_[current_tocken_ + 2].index() == 2) {
                            current_tocken_ += 2;
                            As<Cell>(current)->SetSecond(MakeObj(tockens_[current_tocken_]));

                            if (current && !As<Cell>(current)->GetSecond() &&
                                !As<Cell>(current)->GetFirst()) {
                                current = nullptr;
                            }
                            return begin;
                        } else {
                            // bracket is after dot
                            current_tocken_ += 3;
                            As<Cell>(current)->SetSecond(ReadList());

                            return begin;
                        }
                    }
                    break;
                }
                case 1: {
                    // open bracket
                    current_tocken_++;
                    As<Cell>(current)->SetFirst(ReadList());
                    As<Cell>(current)->SetSecond(std::make_shared<Cell>());
                    current = std::dynamic_pointer_cast<Cell>(As<Cell>(current)->GetSecond());
                    break;
                }
                default: {

                    return begin;
                }
            }
        }

        current_tocken_++;
        return begin;
    }

    void CheckInput() {
        CheckOneToken();
        CheckEmpty();
        CheckFirst();
        CheckBrackets();
        CheckDots();
    }
    void CheckDots() {
        for (size_t i = 0; i < tockens_.size(); i++) {
            if (tockens_[i].index() == 4) {
                if (i == tockens_.size() - 1 || tockens_[i + 1].index() == 3 ||
                    tockens_[i + 1].index() == 4 ||
                    (tockens_[i + 1].index() == 1 &&
                     std::get<BracketToken>(tockens_[i + 1]) == BracketToken::CLOSE)) {
                    throw SyntaxError("Bad dot");
                }
                int bracket_counter = (tockens_[i + 1].index() == 1) ? 1 : 0;
                for (size_t j = i + 2; j < tockens_.size(); j++) {
                    switch (tockens_[j].index()) {
                        case 0:
                        case 2: {
                            if (bracket_counter == 0) {
                                throw SyntaxError("Bad dot");
                            }
                            break;
                        }
                        case 1: {
                            if (std::get<BracketToken>(tockens_[j]) == BracketToken::OPEN) {
                                bracket_counter++;
                            } else {
                                bracket_counter--;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    void CheckOneToken() {
        if (tockens_.size() == 1) {
            if (!(tockens_[0].index() == 0 || tockens_[0].index() == 2)) {
                throw SyntaxError("One token string must be sumbol or string token");
            }
        }
    }
    void CheckEmpty() {
        if (tockens_.empty()) {
            throw SyntaxError("Empty string");
        }
    }
    void CheckFirst() {
        if (tockens_.size() > 1) {
            if (!(tockens_[0].index() == 1 &&
                  std::get<BracketToken>(tockens_[0]) == BracketToken::OPEN)) {
                throw SyntaxError("First symbol must be an open bracket");
            }
        }
    }
    void CheckBrackets() {
        int opens = 0;
        for (auto elem : tockens_) {
            if (elem.index() == 1) {
                if (std::get<BracketToken>(elem) == BracketToken::OPEN) {
                    opens++;
                } else {
                    opens--;
                }
                if (opens < 0) {
                    throw SyntaxError("Bad Brackets");
                }
            }
        }
        if (opens != 0) {
            throw SyntaxError("Bad Brackets");
        }
    }
};

inline std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    Reader reader(tokenizer);
    return reader.Read();
}


