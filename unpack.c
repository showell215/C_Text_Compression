/**
   @file unpack.c
   @author Steven Howell (schowel2)
   This file contains the main functionality for the unpack program.
   Will read input/output/wordlist files from the command line parameters,
   and unpack a compressed file using supporting functions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "wordlist.h"
#include "bits.h"
/** Initial capacity for file string */
#define INIT_FILE_CAP 200
/** Error code for invalid number of CLPs */
#define INVALID_ARGC 201
/** Error code for invalid filename */
#define INVALID_FILENAME 202
/** 2nd CLP */
#define CLP_2 2
/** 3rd CLP */
#define CLP_3 3
/** 4th CLP */
#define CLP_4 4

/**
   Main method. Starts the program and calls other functions.
   @param argc the number of arguments
   @param argv the command line arguments
   @return exit status of the program
*/
int main( int argc, char *argv[] )
{
   char *wordFile = "words.txt";
   char *in;
   char *out;
   if (argc == CLP_3) {
      //argv[1] is input
      in = argv[1];
      //argv[2] is output
      out = argv[CLP_2];
   } else if (argc == CLP_4) {
      //argv[1] is input
      in = argv[1];
      //argv[2] is output
      out = argv[CLP_2];
      //argv[3] is word file
      //strcpy(wordFile, argv[3]);
      wordFile = argv[CLP_3];
   } else {
      //error
      fprintf(stderr, "usage: unpack <compressed.raw> <output.txt> [word_file.txt]\n");
      exit(INVALID_ARGC);
   }
   //generate word list
   WordList *wordList = readWordList( wordFile );
   FILE *input = fopen(in, "r");
   if (!input) {
      fprintf(stderr, "Can't open file: %s\n"
              "usage: unpack <compressed.raw> <output.txt> [word_file.txt]\n", in);
      exit(INVALID_FILENAME);
   }
   FILE *output = fopen(out, "w");
   if (!output) {
      fprintf(stderr, "Can't open file: %s\n"
              "usage: unpack <compressed.raw> <output.txt> [word_file.txt]\n", out);
      exit(INVALID_FILENAME);
   }

#ifdef DEBUG
  // Report the entire contents of the word list, once it's built.
   printf( "----- word list -----\n" );
   for ( int i = 0; i < wordList->len; i++ ) {
      printf( "%d == %s\n", i, wordList->words[ i ] );
   }
   printf( "--------------------\n" );
#endif
   //determine the number of bytes to read from the input file
   fseek(input, 0L, SEEK_END);
   size_t sz = ftell(input);
   fseek(input, 0L, SEEK_SET);
   
   //for the input file, read each code and write out its corresponding
   //characters to the output file.
   PendingBits pending = { 0, 0 };
   int code;
   for (int i = 0; i < sz; i++) {
      //if bitcount is 0, two calls to fgetc will be made
      //must compensate the count for reading
      if (pending.bitCount == 0) {
         i++;
      }
      code = readCode(&pending, input);
      fprintf(output, "%s", wordList->words[code]);
   }
   // Write out any remaining bits in the last, partial byte.
   //flushBits( &pending, output );

   fclose(input);
   fclose(output);
   freeWordList(wordList);
   return EXIT_SUCCESS;
}
