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
  // TODO: write help function
  cout << "TODO" << endl;
}

string join(const vector<string> &vec, const char *delim) {
  stringstream res;
  copy(vec.begin(), vec.end(), ostream_iterator<string>(res, delim));
  return res.str();
}

vector<vector<string>> splitWord(string word, vector<string> &result, const int *n, unordered_map<string, bool> *dictionary, int *currentShortest) {
  if (*currentShortest > 0 && result.size() > *currentShortest) {
    return vector<vector<string>>(0);
  }

  string prepare;
  while (isdigit(word[0])) {
    prepare += word[0];
    word = word.substr(1, word.length() - 1);
  }
  if (prepare.length() > 0) {
    result.push_back(prepare);
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
  for (auto &match : matches) {
    vector<string> newResult = result;
    string newWord = word.substr(match.length());
    newResult.push_back(match);
    if (newWord.length() < *n) {
      if (newWord.length() > 0) {
        newResult.push_back(newWord);
      }
      allResults.push_back(newResult);
      if (allResults.size() < *currentShortest || *currentShortest < 0) {
        *currentShortest = static_cast<int>(newResult.size());
      }
    } else {
      vector<vector<string>> res = splitWord(newWord, newResult, n, dictionary, currentShortest);
      allResults.insert(allResults.end(), res.begin(), res.end());
    }
  }

  if (matches.empty()) {
    if (word.length() > 0) {
      result.push_back(word);
    }
    allResults.push_back(result);
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
      case 'd': // input file
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

  // TODO: read dictionary
  unordered_map<string, bool> dictionary;
  ifstream dictionaryFile(dictionaryFilename, ios::in);
  if (!dictionaryFile.is_open()) {
    cerr << "Failed to open dictionary file!" << endl;
    return -1;
  }
  string line;
  while (!dictionaryFile.eof() && dictionaryFile.is_open()) {
    getline(dictionaryFile, line);
    if (line.length() < min) {
      continue;
    }
    dictionary[line] = true;
  }
  dictionaryFile.close();

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
  // loop over all words
  while (!inputFile.eof() && inputFile.is_open()) {
    getline(inputFile, word);
    if (word.length() == 0) {
      continue;
    }
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);

    vector<string> res;
    int currentShortest = -1;
    vector<vector<string>> allResults = splitWord(word, res, &n, &dictionary, &currentShortest);
    auto shortest = static_cast<int>(word.length());
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
        continue;
      }
      if (allResults[i].size() < shortest) {
        shortest = static_cast<int>(allResults[i].size());
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
        cout << join(result, " ") << endl;
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

  inputFile.close();
  if (noMatchFilename.length() > 0) {
    noMatchFile.close();
  }

  return 0;
}