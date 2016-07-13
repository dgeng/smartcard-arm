CC=gcc
CFLAGS= -g

all: smartcard.so smartcard_c.so testbull testdsper testflex alpar_test writeflex readflex

alpar_test: alpar.o alpar_main.o iso7816.o
	$(CC) -o $@ $^

smartcard_c.so: alpar.o iso7816.o
	$(CC) -shared -o $@ $^

smartcard.so: alpar.o iso7816.o
	$(CC) -shared -o $@ $^

testbull: bullcp8.o
	$(CC) $(CFLAGS) -o $@ $< smartcard_c.so

testdsper: dsperson.o
	$(CC) $(CFLAGS) -o $@ $< smartcard_c.so

testflex: payflex.o
	$(CC) $(CFLAGS) -o $@ $< smartcard_c.so

writeflex: writeflex.o
	$(CC) $(CFLAGS) -o $@ $< smartcard_c.so

readflex: readflex.o
	$(CC) $(CFLAGS) -o $@ $< smartcard_c.so

.c.o:
	$(CC) -c $(CFLAGS) -fpic -o $@ $<

clean:
	rm -f *.o *.doc *wrap.c *.so
