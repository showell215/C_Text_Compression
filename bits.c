/**
   @file bits.C
   @author Steven Howell (schowel2)

   Functions supporting reading and writing arbitrary bit sequences to files.
*/
#include <stdio.h>
#include "bits.h"

/** Write the 9 low-order bits from code to the given file.
    @param code bits to write out, a value between 0 and 2^9 - 1.
    @param pending pointer to storage for unwritten bits left over
    from the previous call to writeCode().  After this call, any bits
    that partially fill the next byte will be left in the pending
    struct, to be written in the next call.
    @param fp file we're writing to, opened for writing.
*/
void writeCode( int code, PendingBits *pending, FILE *fp )
{

   //temporarily store bitCount for efficiency
   int bitCt = pending->bitCount;
   // target the bits that we want to join with pending bits
   unsigned char codeOut = (unsigned char) (code << bitCt);
   //targeted bits from code ORed with pending bits will yield the byte to write
   codeOut = (codeOut | pending->bits);
   //move the remaining high order bits from code into pending
   pending->bits = (unsigned char)(code >> (BITS_PER_BYTE - bitCt));
   //write the code to file
   fputc(codeOut, fp);
   //update bitCount in pending
   //if pending contains a full byte, flush
   if (++bitCt == BITS_PER_BYTE) {
      flushBits(pending, fp);
   }
   pending->bitCount = bitCt % BITS_PER_BYTE;
}
/** If there are any bits buffered in pending, write them out to the
    given file in the low-order bit positions of a byte, leaving zeros
    in the high-order bits.
    @param pending pointer to storage for unwritten bits left over
    from the most recent call to writeCode().
    @param fp file these bits are to be written to, opened for writing.
*/
void flushBits( PendingBits *pending, FILE *fp )
{
   fputc(pending->bits, fp);
   pending->bits = 0;
   pending->bitCount = 0;
}
/** Read and return the next 9-bit code from the given file.
    @param pending pointer to storage for left-over bits read during
    the last call to readCode().
    @param fp file bits are being read from, opened for reading.
    @return value of the 9-bit code read in, or -1 if we reach the
    end-of-file before getting 9 bits.
*/
int readCode( PendingBits *pending, FILE *fp )
{
   int code;
   unsigned char nextByte;
   //special case where 0 pending bits:
   //store next byte in pending before reading another
   if (pending->bitCount == 0) {
      //read nextByte and check for EOF
      nextByte = fgetc(fp);
#ifdef DEBUG
      printf("read byte: %c\n", nextByte);
#endif
     // if (nextByte == EOF) {
        // return -1;
     // }
      //nextByte = fgetc(fp);
      pending->bits = nextByte;

      pending->bitCount = BITS_PER_BYTE;
   }
   //temporarily store bitCount for efficiency
   int bitCt = pending->bitCount;
   nextByte = fgetc(fp);
#ifdef DEBUG
      printf("read byte: %c\n", nextByte);
#endif
  // if (nextByte  == EOF) {
   //   return -1;
  // }
   //code gets any pending bits, plus the remainder filled with LSB of this byte
   unsigned short shortMask = 0x01FF;
   unsigned short nByteShort = (unsigned short) nextByte;
#ifdef DEBUG
   printf("nextByte: %04x\n", nextByte);
   printf("nByteShort: %04x\n", nByteShort);
  // printf("shortMask: %04x\n", mask);
#endif
   unsigned short nByte = ((nByteShort << (bitCt)) & (shortMask));
   code = (unsigned int)pending->bits | (unsigned int)nByte;
   //move the unused higher order bits of read byte into pending and decrement bitCount
   pending->bits = nextByte >> (BITS_PER_BYTE - (bitCt - 1));
   pending->bitCount = --bitCt;
#ifdef DEBUG
      printf("code: %d\n", code);
#endif
   return code;
}
