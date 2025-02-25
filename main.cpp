#include <set>
#include <unordered_set>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <locale>
#include <codecvt>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>

std::string DocReader(std::string &fileName) {
    std::string text;
    std::ifstream outputDoc;
    outputDoc.open(fileName.c_str());
    if(outputDoc.is_open()){
        std::string s;
        while (std::getline(outputDoc,s)) {
            text.append(s);
        }
    }else{
        std::cout << "Invalid output file\n";
    }
    outputDoc.close();

    return text;
}

void CallMyStem(std::list<std::string> &words) {    
    std::ofstream inputFile("../input.txt");
    if (!inputFile.is_open()) {
        throw std::runtime_error("Не удалось открыть файл input.txt");
    }

    for (const auto& word : words) {
        inputFile << word << ' ';
    }
    inputFile.close();

    system("mystem -nl ../input.txt ../output.txt");

    words.clear();

    std::ifstream outputFile("../output.txt");
    if (!outputFile.is_open()) {
        throw std::runtime_error("Не удалось открыть файл output.txt");
    }

    std::string line;
    while (std::getline(outputFile, line)) {
        words.push_back(line);
    }
    outputFile.close();
}

void GetStopWords(std::unordered_set<std::string> &stopWords) {
    std::ifstream outputFile("../russian_stopwords.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Ошибка: файл russian_stopwords.txt не найден или не может быть открыт." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(outputFile, line)) {
        stopWords.insert(line);
    }

    outputFile.close();
}

std::string ToLowercase(std::string &word) {
    std::locale loc("ru_RU.UTF-8");
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wideStr = converter.from_bytes(word);

    for (wchar_t& c : wideStr) {
        c = std::tolower(c, loc);
    }

    return converter.to_bytes(wideStr);
}

std::unordered_map<std::string,std::multiset<std::string>> preProcessor(std::string &text, std::unordered_set<std::string> &stopWords, std::map<std::string, std::set<std::string>> &vocabulary) {
    std::list<std::string> docNorm;
    std::unordered_map<std::string,std::multiset<std::string>> resultDoc;

    boost::tokenizer<> tok(text);

    for (auto beg = tok.begin(); beg != tok.end(); ++beg) { //boost::tokenizer<>::iterator
        std::string word = *beg;
        word = ToLowercase(word);
        if (stopWords.find(word) == stopWords.end() && !word.empty()) {
            docNorm.emplace_back(word);
        }
    }

    CallMyStem(docNorm);

    for( auto&& word : docNorm){
        word.erase(std::find(word.begin(), word.end(),'|'), word.end());
        word.erase(std::find(word.begin(), word.end(),'?'), word.end());
    }
    
    for (auto it = docNorm.begin(); it != docNorm.end(); ++it) {
        if(std::next(it) != docNorm.end()) {
            resultDoc[*it].emplace(*std::next(it));

            if (!vocabulary[*it].count(*std::next(it))) {
                vocabulary[*it].insert(*std::next(it));
            }
            
        }
        if(std::prev(it) != docNorm.end()) {
            resultDoc[*it].emplace(*std::prev(it));

            if (!vocabulary[*it].count(*std::prev(it))) {
                vocabulary[*it].insert(*std::prev(it));
            }
        }

        if(std::next(std::next(it)) != docNorm.end()) {
            resultDoc[*it].emplace(*std::next(std::next(it)));

            if (!vocabulary[*it].count(*std::next(std::next(it)))) {
                vocabulary[*it].insert(*std::next(std::next(it)));
            }
        } 

        if(std::prev(std::prev(it)) != docNorm.end()) {
            resultDoc[*it].emplace(*std::prev(std::prev(it)));

            if (!vocabulary[*it].count(*std::prev(std::prev(it)))) {
                vocabulary[*it].insert(*std::prev(std::prev(it)));
            }
        }
    }

    return resultDoc;
}

std::vector<std::vector<int16_t>> getVector(const std::unordered_map<std::string, std::multiset<std::string>>& text,const std::map<std::string, std::set<std::string>>& vocabulary
) {
    std::vector<std::vector<int16_t>> vec(vocabulary.size(), std::vector<int16_t>(vocabulary.size(),0));
    int counter = 0;
    for (const auto& [term, pairWithTerm] : vocabulary) {
        int inside_counter = 0;
        for (const auto& word : pairWithTerm) {
            if (text.count(term)) {
                vec[counter][inside_counter] =std::count(text.at(term).begin(),text.at(term).end(),word);
            }
            ++inside_counter;
        }
        ++counter;
    }
    return vec;
}

double cosinSimilarity(const std::vector<std::vector<int16_t>>& vec1, const std::vector<std::vector<int16_t>>& vec2) {
    if (vec1.size() != vec2.size() || vec1[0].size() != vec2[0].size()) {
        throw std::invalid_argument("Vectors must have the same size");
    }

    double dotProduct = 0.0;
    double normV1 = 0.0;
    double normV2 = 0.0;

    for (size_t i = 0; i < vec1.size(); ++i) {
        for (size_t j = 0; j < vec1[i].size(); ++j) {
            dotProduct += vec1[i][j] * vec2[i][j];
            normV1 += std::pow(vec1[i][j], 2);
            normV2 += std::pow(vec2[i][j], 2);
        }
    }

    if (normV1 == 0 || normV2 == 0) {
        return 0.0;
    }

    return dotProduct / (std::sqrt(normV1) * std::sqrt(normV2));
}

void printVector(std::vector<int16_t> vec) {
    for (auto &&i : vec) {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}

void printText(std::unordered_map<std::string,std::multiset<std::string>> &text) {
    for (auto &&[term,set] : text) {
        std::cout << term << ": ";
        for (auto &&word : set) {
            std::cout << word << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

int main(){
    std::string path1 = "../testText.txt" ;
    std::string path2 = "../testText2.txt" ;

    std::string doc1 = DocReader(path1);
    std::string doc2 = DocReader(path2);


    std::unordered_set<std::string> stopWords;
    std::map<std::string, std::set<std::string>> vocabulary;
    GetStopWords(stopWords);

    std::unordered_map<std::string,std::multiset<std::string>> text1 = preProcessor(doc1,stopWords,vocabulary);
    std::unordered_map<std::string,std::multiset<std::string>> text2 = preProcessor(doc2,stopWords,vocabulary);


    printText(text1);
    printText(text2);

    std::vector<std::vector<int16_t>> vec1 = getVector(text1,vocabulary);
    std::vector<std::vector<int16_t>> vec2 = getVector(text2,vocabulary);

    std::cout << "Схожесть 1 и 1: " << cosinSimilarity(vec1, vec1) << '\n';
    std::cout << "Схожесть 1 и 2: " << cosinSimilarity(vec1, vec2) << '\n';

    return 0;
}