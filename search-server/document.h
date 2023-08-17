#pragma once

#include <iostream>

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {}

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};


std::ostream& operator<<(std::ostream& out, const Document& d);