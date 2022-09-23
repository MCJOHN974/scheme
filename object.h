#pragma once

#include <memory>
#include "tokenizer.h"

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
    int value_;

public:
    Number(int v = 0) : value_(v) {
    }

    int GetValue() const {
        return value_;
    }
};

class Symbol : public Object {
    std::string name_;

public:
    Symbol(const std::string& s = "") : name_(s) {
    }
    Symbol(std::string&& s = "") : name_(s) {
    }

    const std::string& GetName() const {
        return name_;
    }
};

class Cell : public Object {
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;

public:
    Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second)
        : first_(first), second_(second) {
    }
    Cell() : first_(nullptr), second_(nullptr) {
    }

    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    std::shared_ptr<Object>& GetFirst() {
        return first_;
    }
    std::shared_ptr<Object>& GetSecond() {
        return second_;
    }

    void SetFirst(std::shared_ptr<Object> obj) {
        first_ = obj;
    }
    void SetSecond(std::shared_ptr<Object> obj) {
        second_ = obj;
    }
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return obj == std::dynamic_pointer_cast<T>(obj);
    // return (bool)std::dynamic_pointer_cast<T>(obj);
}

inline std::shared_ptr<Object> MakeObj(Token token) {
    if (token.index() == 0) {
        return std::make_shared<Number>(std::get<ConstantToken>(token).value);
    } else if (token.index() == 2) {
        return std::make_shared<Symbol>(std::get<SymbolToken>(token).name);
    }
    return std::make_shared<Object>();
}
