CMP_CFILES = scanner.c parser.c symtable/*.c utils/*.c icode/*.c tcode/*.c
AVM_CFILES = *.c ../tcode/instructions.c ../tcode/consts.c ../utils/utils.c

all: clean alpha alphavm

alpha:
	flex --outfile=scanner.c scanner.l
	bison --defines --output=parser.c parser.y
	gcc -o alpha $(CMP_CFILES)

alphavm:
	cd avm && gcc -o alphavm $(AVM_CFILES) -lm && mv alphavm ../
	@rm scanner.c parser.c parser.h

clean:
	@rm -f alpha alphavm scanner.c parser.c parser.h
	@rm -f quads.txt binary.abc.txt binary.abc binary_loaded.abc