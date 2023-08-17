#pragma once

#include <deque>

#include "search_server.h"


class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : search_server_(search_server) {
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const {
        return no_result_requests;
    }

private:
    void AddRequest(const std::vector<Document>& documents);

private:
    struct QueryResult {
        bool has_result = false;
    };
    
    std::deque<QueryResult> requests_;
    static constexpr int min_in_day_ = 1440;

    const SearchServer& search_server_;
    int no_result_requests = 0;
}; 

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    auto documents = search_server_.FindTopDocuments(raw_query, document_predicate);
    AddRequest(documents);
    return documents;
}