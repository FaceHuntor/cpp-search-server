#include "request_queue.h"

using namespace std;

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    auto documents = search_server_.FindTopDocuments(raw_query, status);
    AddRequest(documents);
    return documents;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    auto documents = search_server_.FindTopDocuments(raw_query);
    AddRequest(documents);
    return documents;
}

void RequestQueue::AddRequest(const vector<Document>& documents) {
    if(requests_.size() == min_in_day_){
        no_result_requests -= !requests_.front().has_result;
        requests_.pop_front();
    }
    requests_.push_back({!documents.empty()});
    no_result_requests += !requests_.back().has_result;
}