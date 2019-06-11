ARCH=$(PROC)

CFLAGS+=-Wall -O3 -funroll-loops -ggdb -D__MOD_G723__

LIB=libg723.a

OBJS=\
 g723.o coder.o decod.o lpc.o exc_lbc.o\
 lsp.o tab_lbc.o util_lbc.o basop.o vad.o\
 cod_cng.o dec_cng.o util_cng.o tame.o

all: $(LIB)

$(LIB): $(OBJS)
	ar crv $(LIB) $(OBJS)

clean:
	rm -f $(LIB) *.o
