#include <algorithm>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class Query
{
public:
    bool IsMinusWord(const string& word) const {
        return minus_words_.count(word) > 0;
    }
    
    bool HasWord(const string& word) const {
        return query_words_.count(word) > 0;
    }
    
    void AddWord(const string& word)
    {
        if (word.empty())
            return;
        if (word[0] == '-')
            minus_words_.insert(word.substr(1));
        else
        {
            ++query_words_[word];
            ++qwords_count_;
        }  
    }
    
    const map<string, int>& QueryWords() const {return query_words_;}
    
    unsigned int QueryWordsCount() const {return qwords_count_;}
    
    const set<string>& MinusWords() const {return minus_words_;}
    
private:
    map<string, int> query_words_;
    set<string> minus_words_;
    unsigned int qwords_count_ = 0;
};

// information about word in document
struct WordDocInfo
{
    int count = 0;
    double tf = 0.0;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        for (const auto& word : words)
        {
            auto& doc_word_info = inverse_documents_[word][document_id];
            ++doc_word_info.count;
            doc_word_info.tf = double(doc_word_info.count) / double(words.size());
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const auto query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
    
private:
    map<string, map<int, WordDocInfo>> inverse_documents_;
    int document_count_ = 0;
    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            query.AddWord(word);
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> relevance;
        
        for (const auto [qword, qword_count]: query.QueryWords())
        {
            if(!inverse_documents_.count(qword))
                continue;
            
            const auto& docs_info = inverse_documents_.at(qword);
            double idf = log(double(document_count_) / double(docs_info.size()));
            
            for (const auto [doc_id, doc_info] : docs_info)
            {
                relevance[doc_id] += doc_info.tf * idf;
            }
        }
        
        for (const auto& qword: query.MinusWords())
        {
            if(!inverse_documents_.count(qword))
                continue;
            const auto& docs_info = inverse_documents_.at(qword);
            for (const auto [doc_id, doc_info] : docs_info)
            {
                relevance.erase(doc_id);
            }
        }
        
        vector<Document> matched_documents;
        
        for (const auto [id, rel] : relevance)
        {
            matched_documents.push_back({id, rel});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const auto search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
