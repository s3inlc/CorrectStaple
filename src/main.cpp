#include <iostream>
#include <getopt.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <locale>
#include <sstream>

using namespace std;

void printHelp() {
  cout << "Correct Staple v1.0 - Help" << endl;
  cout << "correct-staple -i <input-file> -d <dictionary> [-ch] [-n <N>] [-m <min>] [-x <outfile>]" << endl;
  cout << "  -i <input-file> File where lines to split are read from" << endl;
  cout << "  -d <dictionary> Dictionary to read valid words from" << endl;
  cout << "  -n <N>          Set the minimal length for words" << endl;
  cout << "  -m <min>        At least one element in combination string must be longer than this value" << endl;
  cout << "  -x <outfile>    Save lines where no valid combinations is found to this file" << endl;
  cout << "  -c              Instead of printing the splitted words, apply case toggle for all words" << endl;
  cout << "  -h              Print Help" << endl;
}

string join(const vector<string> &vec, const char *delim) {
  stringstream res;
  copy(vec.begin(), vec.end(), ostream_iterator<string>(res, delim));
  return res.str().substr(0, res.str().length() - 1); // TODO: find a better way for this
}

bool isShort(vector<string> &vec, const int *min) {
  if (vec.size() < 3) {
    return false;
  }
  bool isShort = true;
  for (auto &p: vec) {
    if (p.length() > *min) {
      isShort = false;
      break;
    }
  }
  return isShort;
}

void cleanup(vector<vector<string>> &results, int *min) {
  for (unsigned int i = 0; i < results.size(); i++) {
    if (isShort(results.at(i), min)) {
      results.erase(results.begin() + i);
      i--;
      continue;
    }
  }
}

vector<vector<string>> splitWord(string word, vector<string> &result, const int *n, unordered_map<string, bool> *dictionary, int *currentShortest, int *min) {
  if (*currentShortest > 0 && result.size() >= *currentShortest) {
    return vector<vector<string>>();
  }

  string prepare;
  while (isdigit(word[0])) {
    prepare += word[0];
    word = word.substr(1, word.length() - 1);
  }

  vector<string> matches;
  string part;
  for (unsigned long i = word.length(); i >= *n; i--) {
    part = word.substr(0, i);
    if (dictionary->find(part) != dictionary->end()) {
      matches.push_back(part);
    }
    if (matches.size() > 2) { // TODO: make it possible to set this value
      break;
    }
  }

  vector<vector<string>> allResults;
  vector<string> newResult;
  vector<vector<string>> res;
  string newWord;
  for (auto &match : matches) {
    newResult = result;
    if (prepare.length() > 0) {
      newResult.push_back(prepare);
    }
    newWord = word.substr(match.length());
    newResult.push_back(match);
    if (newWord.length() < *n) {
      if (newWord.length() > 0) {
        newResult.push_back(newWord);
      }
      if (!isShort(newResult, min)) {
        allResults.push_back(newResult);
        if (newResult.size() < *currentShortest || *currentShortest < 0) {
          *currentShortest = (int) newResult.size();
        }
      }
    } else {
      res = splitWord(newWord, newResult, n, dictionary, currentShortest, min);
      cleanup(res, min);
      allResults.insert(allResults.end(), res.begin(), res.end());
    }
  }

  if (matches.empty()) {
    newResult = result;
    if (prepare.length() > 0) {
      newResult.push_back(prepare);
    }
    if (word.length() > 0) {
      newResult.push_back(word);
    }
    allResults.push_back(newResult);
  }

  return allResults;
}

int main(int argc, char **argv) {
  string noMatchFilename;
  string inputFilename;
  string dictionaryFilename;
  int n = 1, min = 2;
  bool printWithCase = false;
  int c;

  opterr = 0;
  while ((c = getopt(argc, argv, "n:m:x:i:d:hc")) != -1) {
    switch (c) {
      case 'h': // print help
        printHelp();
        return 0;
      case 'c': // apply case toggle to detected words instead of printing splitted parts
        printWithCase = true;
        break;
      case 'x': // safe no-matches to file
        noMatchFilename = optarg;
        break;
      case 'i': // input file
        inputFilename = optarg;
        break;
      case 'd': // dictionary file
        dictionaryFilename = optarg;
        break;
      case 'n': // set minimal length of elements
        n = atoi(optarg);
        if (n <= 0) {
          cerr << "Invalid value for option -n!" << endl;
          printHelp();
          return -1;
        }
        break;
      case 'm': // set minimal length one of the elements at least should have
        min = atoi(optarg);
        if (min <= 0) {
          cerr << "Invalid value for option -m!" << endl;
          printHelp();
          return -1;
        }
        break;
      case '?':
        if (optopt == 'm' || optopt == 'n' || optopt == 'x')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        return -1;
      default:
        abort();
    }
  }

  // TODO: test config

  unordered_map<string, bool> dictionary;
  ifstream dictionaryFile(dictionaryFilename, ios::in);
  if (!dictionaryFile.is_open()) {
    cerr << "Failed to open dictionary file!" << endl;
    return -1;
  }
  string line;
  while (!dictionaryFile.eof() && dictionaryFile.is_open()) {
    getline(dictionaryFile, line);
    if (!line.empty() && line[line.size() - 1] == '\r') {
      line = line.substr(0, line.size() - 1);
    }
    if (line.length() < n) {
      continue;
    }
    dictionary[line] = true;
  }
  dictionaryFile.close();

  cerr << "Dictionary reading complete!" << endl;

  ifstream inputFile(inputFilename.c_str(), ios::in);
  if (!inputFile.is_open()) {
    cerr << "Failed to open input file!" << endl;
    return -1;
  }

  ofstream noMatchFile;
  if (noMatchFilename.length() > 0) {
    noMatchFile.open(noMatchFilename.c_str(), ios::out);
    if (!noMatchFile.is_open()) {
      cerr << "Failed to open no-match output file!" << endl;
      return -1;
    }
  }

  string word;
  stringstream buffer;
  int bufferCount = 0;
  // loop over all words
  while (!inputFile.eof() && inputFile.is_open()) {
    getline(inputFile, word);
    if (!word.empty() && word[word.size() - 1] == '\r') {
      word = word.substr(0, word.size() - 1);
    }
    if (word.length() == 0) {
      continue;
    }
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);

    vector<string> res;
    int currentShortest = -1;
    vector<vector<string>> allResults = splitWord(word, res, &n, &dictionary, &currentShortest, &min);
    auto shortest = (int) word.length();
    for (unsigned int i = 0; i < allResults.size(); i++) {
      bool isShort = true;
      for (auto &p: allResults.at(i)) {
        if (p.length() > min) {
          isShort = false;
          break;
        }
      }
      if (isShort) {
        allResults.erase(allResults.begin() + i);
        i--;
        continue;
      }
      if (allResults[i].size() < shortest) {
        shortest = (int) allResults[i].size();
      }
    }

    bool printed = false;
    locale loc;
    for (auto &result: allResults) {
      if (result.size() > shortest) {
        continue;
      }
      printed = true;
      if (!printWithCase) {
        buffer << join(result, " ") << endl;
        bufferCount++;
        if(bufferCount > 100){
          cout << buffer.str();
          buffer.str("");
          bufferCount = 0;
        }
      } else {
        for (int i = 0; i < pow(2, result.size()); i++) {
          string output;
          int j = i;
          for (auto &element: result) {
            if (j % 2 == 1) {
              element[0] = toupper(element[0], loc);
              output += element;
            } else {
              output += element;
            }
            j >>= 1;
          }
          cout << output << endl;
        }
      }
    }

    if (!printed && noMatchFilename.length() > 0) {
      noMatchFile << word << endl;
    }
  }

  if(bufferCount > 0){
    cout << buffer.str();
  }

  inputFile.close();
  if (noMatchFilename.length() > 0) {
    noMatchFile.close();
  }

  return 0;
}