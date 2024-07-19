#include <algorithm>
#include <iostream>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

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
    cin >> ws;
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

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(const int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double occur_word_in_document = 1. / words.size();
        ++document_count_;
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += occur_word_in_document;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

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
    struct Query {
        set<string>plus_words_;
        set<string>minus_words_;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    int document_count_ = 0;

    Query ParseQuery(const string& query) const
    {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(query)) {
            if (word[0] == '-') {
                string sliced = word.substr(1);
                if (!IsStopWord(sliced)) {
                    query_words.minus_words_.insert(word.substr(1));
                }
            }
            else {
                query_words.plus_words_.insert(word);
            }
        }
        return query_words;
    }

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

    double ComputeIdf(const  string& word) const {
        int occurs_word_in_documents = static_cast<int>(word_to_document_freqs_.at(word).size());
        double idf = log(static_cast<double>(document_count_) / occurs_word_in_documents);
        return idf;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double>document_to_relevance;

        for (const string& word: query_words.plus_words_) {
           if (word_to_document_freqs_.count(word)) {
                for (const auto& [id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance[id] += tf * ComputeIdf(word);
                }
           }
        }

        for (const string& word: query_words.minus_words_) {
            if (word_to_document_freqs_.count(word)) {
                for (const auto& [id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(id);
                }
            }
        }

        matched_documents.reserve(document_to_relevance.size());
        for (const auto& [id, relevance] : document_to_relevance) {
            matched_documents.push_back( {id, relevance} );
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
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();

    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}