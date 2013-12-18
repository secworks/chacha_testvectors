#=======================================================================
#
# Makefile
# --------
# Very simple makefile for the chacha test vectors generator.
#
#
# (c) 2013 Joachim Strombergson
#
#=======================================================================

(CC) = clang
(FLAGS) = -Wall -O2

all: generator


generator: gen_chacha_testvectors.c
	$(CC) $(FLAGS) -o gen_chacha_testvectors gen_chacha_testvectors.c


generate: gen_chacha_testvectors
	./gen_chacha_testvectors > chacha_testvectors.txt


clean:
	rm -f gen_chacha_testvectors
	rm -f chacha_testvectors.txt

#=======================================================================
# EOF Makefile
#=======================================================================

