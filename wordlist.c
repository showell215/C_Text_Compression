/**
   @file wordlist.c
   @author Steven Howell (schowel2)
   
   This file contains functions for reading the wordlist file. The wordlist
   will be a lexicographically sorted list of common words, and provide codes
   for storing those words.
*/
#include "wordlist.h"

//...
/**
   Given a character, this function returns true if it's one of the 98 valid
   characters that can appear in a word of the wordlist or in the text file
   being compressed. This function is may be used internally while reading
   the word file, but it's also exposed in the header file so other components
   can use it.
   @param ch the character we are checking
   @return true if valid; false otherwise
*/
bool validChar(char ch)
{
   return (ch == 0x09 || ch == 0x0A || ch == 0x0D || (ch >= 0x20 && ch <= 0x7E));
}
/**
   String comparison function to be used in qsort
   @param s1 a pointer to void (to be cast as const char *)
   @param s2 a pointer to void (to be cast as const char *)
   @return less than 0 if s1 before s2, greater than 0 if s2 before s1, 0 otherwise
*/
int strComp(const void *s1, const void *s2) {
   //s1 > s2 -> 1
   //s1 < s2 -> -1
   //else 0
   const char *str1 = (const char *)s1;
   const char *str2 = (const char *)s2;
   return strcmp(str1, str2);
}
/**
   This function is responsible for building the word list. It reads words from
   the word file given as fname. After reading all the words from the word file, it
   adds single-character words for each of the 98 valid characters. Finally, it
   sorts the word list lexicographically so that the index of each word is its code.
   @param fname the filename containing the wordlist
   @return a pointer to the wordlist
*/
WordList *readWordList(char const *fname)
{
   FILE *fp = fopen(fname, "r");
   if (!fp) {
      fprintf(stderr, "Can't open word file: %s\n", fname);
      exit(FILE_ERR_WORD_LIST);
   }
   WordList *wordList = (WordList *)malloc(sizeof(WordList));
   //each line contains int with length, then the word
   wordList->capacity = WORDLIST_INIT_SIZE;
   //Initialize words array with enough space for 128 words
   wordList->words = (Word *)malloc(wordList->capacity * sizeof(Word));
   //read file
   int matches;
   int length;
   Word word;
   //What's the best way to both check for EOF and check for a valid input?
   while ((matches = fscanf(fp, "%d", &length) != EOF)/* && matches == 1*/) {
      //check for valid len
      if ((length < WORD_MIN || length > WORD_MAX) || wordList->len >= MAX_WORDS) {
         fprintf(stderr, "Invalid word file\n");
         exit(ERR_WORDFILE_FMT);
      }
      //check for resize
      if (wordList->len >= wordList->capacity) {
         wordList->capacity *= 2; //double capacity
         wordList->words = (Word *)realloc(wordList->words, wordList->capacity * sizeof(Word));
      }
      //read in next "word"
      fgetc(fp); //toss out space
      char currentChar;
      for (int i = 0; i < length; i++) {
         currentChar = fgetc(fp);
         //check validity
         if (!validChar(currentChar)) {
            fprintf(stderr, "Invalid word file\n");
            exit(ERR_WORDFILE_FMT);
         }
         word[i] = currentChar;
      }
      word[length] = '\0';

      strcpy(wordList->words[wordList->len++], word);
      fgetc(fp);//toss out newline
   }
   
   //close the file
   fclose(fp);
   
   //add in the 98 chars
   //tab char
   Word tab;
   tab[0] = 0x09;
   tab[1] = '\0';
   strcpy(wordList->words[wordList->len++], tab);
   //newline
   Word newline;
   newline[0] = 0x0A;
   newline[1] = '\0';
   strcpy(wordList->words[wordList->len++], newline);
   //carriage return
   Word cr;
   cr[0] = 0x0D;
   cr[1] = '\0';
   strcpy(wordList->words[wordList->len++], cr);
   //0x20 - 0x7E
   for (char i = 0x20; i <= 0x7E; i++) {
      word[0] = i;
      word[1] = '\0';
      strcpy(wordList->words[wordList->len++], word);
   }
   //size_t n = wordList->len;
   //sort the elements in wordlist
   qsort(wordList->words[0],             // Start of the list
         wordList->len,                  // number of items
         sizeof(Word),                   // size of each item
         strComp);

   return wordList;
}

/**
   Given a word list, this function returns the best code for representing the sequence
   of characters at the start of the given string. (It returns the index of the
   longest word in the wordList that matches a prefix of the string.)
   @param wordList a pointer to the wordList
   @param str the string for which to find a code
   @return the best code to represent this string
*/
int bestCode(WordList *wordList, char const *str)
{
   //store actual length of string to check
   int lenStr;
   if (strlen(str) < WORD_MAX) {
      lenStr = strlen(str);
   } else {
      lenStr = WORD_MAX;
   }
   for (int i = 0; i < lenStr; i++) {
      //prefix to search for this iteration
      char strPrefix[lenStr - i + 1];
      strncpy(strPrefix, str, lenStr - i);
      strPrefix[lenStr - i] = '\0';
      //bsearch for this prefix
      Word *p = (Word *)bsearch(strPrefix, wordList->words[0], wordList->len,
                                sizeof(Word), strComp);
      if (p) { //match, ptr from bsearch is not null
         //code index will be pointer location minus start of words array
         int code = p - &(wordList->words[0]);
         return code;
      }
   }
   //otherwise return an invalid code
   return -1;
}
/**
   This function frees the memory for the given wordList, including the dynamically
   allocated list of words inside and the wordList structure itself.
   @param wordList the wordlist to free
*/
void freeWordList(WordList *wordList)
{
   free(wordList->words);
   free(wordList);
}
