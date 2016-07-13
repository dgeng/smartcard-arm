CC=arm-linux-gcc
CFLAGS= -g

all: smartpoll.o smartcard.so smartcard_c.so testbull testdsper testflex alpar_test writeflex readflex

smartpoll.o: smartpoll.c
	arm-linux-gcc -I/home/proj/ajit/linux/include -Wall -Wstrict-prototypes -O2  -pipe -msoft-float -mshort-load-bytes -march=armv4 -mtune=strongarm110 -fno-strict-aliasing -c -o smartpoll.o smartpoll.c

alpar_test: alpar.o alpar_main.o iso7816.o
	$(CC) -o $@ $^

smartcard_c.so: alpar.o iso7816.o
	$(CC) -shared -o $@ $^

smartcard.so: alpar.o iso7816.o smartcard_wrap.o
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

smartcard_wrap.o: smartcard.i
	swig -perl5 $<
	$(CC) -c $(CFLAGS) -DPERL_POLLUTE -Dbool=char -I/home/opt/usr/lib/perl5/5.00503/armv4l-linux/CORE/ -fpic -o $@ smartcard_wrap.c

clean:
	rm *.o *.doc *wrap.c *.so
