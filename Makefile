CXX = g++
CXXFLAGS = -Wall -std=c++0x

# We have two targets.  By default, we'll try to make both.
all: pack unpack

pack: pack.o bits.o wordlist.o

pack.o: bits.h wordlist.h

unpack: unpack.o bits.o wordlist.o

unpack.o: bits.h wordlist.h

bits.o: bits.h

wordlist.o: wordlist.h

clean:
	rm -f *.o
	rm -f pack unpack
