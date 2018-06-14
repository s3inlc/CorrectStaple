# Correct Staple

Correct Staple is thought to take password phrases or passwords with multiple words and trying to split them into the word parts.
This is done with a provided dictionary of valid words which can be words. 
Every input might result in multiple guesses, still this results in only 40% more guesses in average.

The idea to have the words split is to make it possible to toggle the case on the first characters of the words.
Correct staple can provide its guesses already with all case possibilities.

## Building

Correct Staple can be built using the provided CMake file or by just compiling the main.cpp file. 
It's recommended to use compiler optimization flag `-O3`.

## Usage

```
correct-staple -i <input-file> -d <dictionary> [-ch] [-n <N>] [-m <min>] [-x <outfile>]
  -i <input-file> File where lines to split are read from
  -d <dictionary> Dictionary to read valid words from
  -n <N>          Set the minimal length for words
  -m <min>        At least one element in combination string must be longer than this value
  -x <outfile>    Save lines where no valid combinations is found to this file
  -c              Instead of printing the splitted words, apply case toggle for all words
  -h              Print Help
```


