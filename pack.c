/**
   @file pack.c
   @author Steven Howell (schowel2)
   @author CSC 230 Teaching Staff
   
   This file contains the main functionality for the pack program. Reads the input/
   output/wordlist files from Command Line Parameters and calls supporting functions
   to compress a text file.
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
   Reads the entire input file into a buffer.
   @param fp the pointer to the input file.
   @return a pointer to the buffer containing the contents of the file.
*/
char *readFile(FILE *fp)
{
   int capacity = INIT_FILE_CAP;
   char *fileString = (char *)malloc(capacity);
   int len = 0;
   char ch;
   while (fscanf(fp, "%c", &ch) == 1) {
      if (len >= capacity) {
         capacity *= 2; //double capacity
         fileString = (char *)realloc(fileString, capacity);
      }
      fileString[len++] = ch;
   }
   fileString[len] = '\0';
   return fileString;
}
/**
   Main method. Starts the program, reads input/output/wordlist files from command line,
   and calls supporting methods to compress the file.
   @param argc the number of command line parameters
   @param argv the command line parameters
   @return the programs exit status
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
      out= argv[CLP_2];
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
      fprintf(stderr, "usage: pack <input.txt> <compressed.raw> [word_file.txt]\n");
      exit(INVALID_ARGC);
   }
   //generate word list
   WordList *wordList = readWordList( wordFile );
   FILE *input = fopen(in, "r");
   if (!input) {
      fprintf(stderr, "Can't open file: %s\n"
              "usage: pack <input.txt> <compressed.raw> [word_file.txt]\n", in);
      exit(INVALID_FILENAME);
   }
   FILE *output = fopen(out, "w");
   if (!output) {
      fprintf(stderr, "Can't open file: %s\n"
              "usage: pack <input.txt> <compressed.raw> [word_file.txt]\n", out);
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
   // Read the contents of the whole file into one big buffer.  This could be more
   // efficient, but it simplifies the rest of the program.
   char *buffer = readFile( input );
#ifdef DEBUG
      printf( "%s\n", buffer);
#endif
   // Write out codes for everything in the buffer.
   int pos = 0;
   PendingBits pending = { 0, 0 };
   int code;
   while ( buffer[ pos ] ) {
      //check for valid char
      if (!validChar(buffer[pos])) {
         fprintf(stderr, "Invalid character code: %02x\n", (unsigned char)buffer[pos]);
         exit(ERR_WORDFILE_FMT);
      }
      // Get the next code.
      code = bestCode( wordList, buffer + pos );
#ifdef DEBUG
      printf( "%d <- %s\n", code, wordList->words[ code ] );
#endif

      // Write it out and move ahead by the number of characters we just encoded.
      writeCode( code, &pending, output );
      pos += strlen( wordList->words[ code ] );
   }

   // Write out any remaining bits in the last, partial byte.
      if (pending.bitCount != 0) {
         flushBits( &pending, output );
      }
   fclose(input);
   fclose(output);
   freeWordList(wordList);
   free(buffer);
   return EXIT_SUCCESS;
}
