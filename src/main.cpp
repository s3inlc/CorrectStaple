#include <iostream>
#include <getopt.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

using namespace std;

void printHelp() {
  // TODO: write help function
  cout << "TODO" << endl;
}

vector<vector<string>>
splitWord(string word, vector<string> &result, const int *n, unordered_map *dictionary, int *currentShortest) {
  if (currentShortest > 0 && result.size() > *currentShortest) {
    return vector<vector<string>>(0);
  }

  string prepare = "";
  while (isdigit(word[0])) {
    prepare += word[0];
    word = word.substr(1, word.length() - 1);
  }
  if (prepare.length() > 0) {
    result.push_back(prepare);
  }

  vector<string> matches;
  string part;
  for (unsigned int i = word.length(); i >= *n; i--) {
    part = word.substr(0, i);
    if (dictionary->find(part) != dictionary->end()) {
      matches.push_back(part);
    }
    if (matches.size() > 2) { // TODO: make it possible to set this value
      break;
    }
  }

  vector<vector<string>> allResults;
  for (int i = 0; i < matches.size(); i++) {
    vector<string> newResult = result;
    string newWord = word.substr(matches[i].length());
    newResult.push_back(matches[i]);
    if (newWord.length() < *n) {
      if (newWord.length() > 0) {
        newResult.push_back(newWord);
      }
      allResults.push_back(newResult);
      if (allResults.size() < *currentShortest || *currentShortest < 0) {
        *currentShortest = newResult.size();
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
  string noMatchFilename = "";
  string inputFilename = "";
  string dictionaryFilename = "";
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
  while (!inputFile.eof() && inputFile.is_open()) {
    getline(inputFile, word);
    if (word.length() == 0) {
      continue;
    }
  }

  return 0;
}