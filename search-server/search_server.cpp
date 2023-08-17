#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>


using namespace std;

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status,
                               const vector<int>& ratings) {
    if(document_id < 0){
        throw invalid_argument("Negative id");
    }
    if(documents_.count(document_id)){
        throw invalid_argument("There is document with this id");
    }
    const vector<string> words = SplitIntoWordsNoStop(document);
    for(const auto& word: words)
    {
        if (ContainsSpecSymbols(word)){
            throw invalid_argument("Words contains special characters");
        }
    }

    const double inv_word_count = 1.0 / words.size();
    for (const auto& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    doc_id_by_index_.emplace_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    vector<string> matched_words;
    for (const auto& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const auto& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::EndsWithMinus(const string& word){
    return word.back() == '-';
}

bool SearchServer::ContainsSpecSymbols(const string& word){
    return count_if(word.begin(), word.end(), [](char c){return c < 32 && c >= 0;});
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const auto& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings){
    if (ratings.empty()) {
        return 0;
    }
    return accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
    if(text.empty()){
        throw invalid_argument("Empty word");
    }
    if(ContainsSpecSymbols(text)){
        throw invalid_argument("Words contains special characters");
    }
    if(EndsWithMinus(text)){
        throw invalid_argument("Word ends with '-' character");
    }

    QueryWord result;
    if (text[0] == '-') {
        if(text[1] == '-'){
            throw invalid_argument("Word starts with double '-' character");
        }
        result.is_minus = true;
        text = text.substr(1);
    }
    else{
        result.is_minus = false;
    }
    result.data = move(text);
    result.is_stop = IsStopWord(text);
    return result;
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query result;
    for (const auto& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            } else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

