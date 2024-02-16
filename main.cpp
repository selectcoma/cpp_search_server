#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
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

    Document() : id(0), relevance(0.0) {}
    Document(int id, double relevance) : id(id), relevance(relevance) {}
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
        const double n = words.size();
        for (const string& word: words) {
            documents_[word][document_id] += 1/n;
        }  ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        Query query = ParseQuery(raw_query);
        vector<Document> matched_documents = FindAllDocuments(query);
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
    // word - doc_id - TF
    map<string, map<int, double>> documents_;
    set<string> stop_words_;
    int document_count_ = 0;
    
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
 
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
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
    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }
 
    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }
    
    
    vector<Document> FindAllDocuments(const Query& query) const {
        vector<Document> matching_documents;
        map<int, double> id_to_relevance;
        
        for (const string& plus_word : query.plus_words) {
            if (documents_.find(plus_word) != documents_.end()) {
                int df = documents_.at(plus_word).size();
                double idf = log(static_cast<double>(document_count_) / df); 
                for (const auto&  [id, tf] : documents_.at(plus_word)) {
                    double tf_idf = tf * idf;
                    if (!id_to_relevance.count(id)) {
                        id_to_relevance[id] = tf_idf;
                    }  else {
                        id_to_relevance[id] += tf_idf; 
                    }
                }
            }
        }
        
        for (const string& minus_word : query.minus_words) {
            if (documents_.find(minus_word) != documents_.end()) {
                for (const auto& [id, tf] : documents_.at(minus_word)) {
                   id_to_relevance.erase(id);
                }
            }
        }
        
        for (const auto& [id, relevance]: id_to_relevance) {
            Document doc(id, relevance);
            matching_documents.push_back(doc);
        }
        
        return matching_documents;
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