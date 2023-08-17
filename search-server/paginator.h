#pragma once

#include <ostream>
#include <vector>
#include <algorithm>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator first, Iterator last) : first_(first), last_(last), size_(distance(first_, last_)) {
    }

    Iterator begin() const {
        return first_;
    }

    Iterator end() const {
        return last_;
    }

    size_t size() const {
        return size_;
    }

private:
    Iterator first_, last_;
    size_t size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        const size_t max_pos = distance(begin, end);
        size_t left_pos = 0;
        auto left = begin;

        while(left != end)
        {
            const auto cur_page_size = std::min(max_pos - left_pos, page_size);
            const auto right = next(left, cur_page_size);
            pages_.emplace_back(left, right);
            left = right;
            left_pos += cur_page_size;
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    size_t size() const {
        return pages_.size();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};
