#include "parser.h"

void RemoveNull(std::shared_ptr<Object>& ptr) {
    if (Is<Cell>(ptr)) {
        if (As<Cell>(ptr)->GetFirst()) {
            RemoveNull(As<Cell>(ptr)->GetFirst());
        }
        if (As<Cell>(ptr)->GetSecond()) {
            RemoveNull(As<Cell>(ptr)->GetSecond());
        }
        if (!As<Cell>(ptr)->GetSecond() && !As<Cell>(ptr)->GetFirst()) {
            ptr = nullptr;
        }
    }
}

