cc := aarch64-none-linux-gnu-gcc

all: writer

writer: writer.o
	$(cc) writer.o -o writer

writer.o: writer.c 
	$(cc) -c writer.c -o writer.o 

clean:
	rm -f writer.o writer
