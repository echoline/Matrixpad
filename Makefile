all: Matrixpad

standalone: test.cpp Matrix.cpp
	g++ -o standalone test.cpp Matrix.cpp -DSTANDALONE

cluster: test.cpp Matrix.cpp
	g++ -o cluster test.cpp Matrix.cpp -DCLUSTER -fopenmp

Decoder: Decoder.o Matrix.o
	g++ -o Decoder Decoder.o Matrix.o

Test: Test.o Matrix.o
	g++ -g -o Test Test.o Matrix.o -lm

Test.o: Test.cpp Matrix.h
	g++ -c -g Test.cpp

Matrix.o: Matrix.cpp Matrix.h
	g++ -c -g Matrix.cpp

Decoder.o: Decoder.cpp Matrix.h
	g++ -c -g Decoder.cpp

Matrixpad: Matrixpad.o Matrix.o
	g++ -g -o Matrixpad Matrixpad.o Matrix.o `pkg-config --libs gtk+-2.0` -Wl,--export-dynamic

Matrixpad.o: Matrixpad.cpp Matrixpad.ui.h
	g++ -c -g Matrixpad.cpp `pkg-config --cflags gtk+-2.0`

Matrixpad.xml: Matrixpad.glade
	gtk-builder-convert Matrixpad.glade Matrixpad.xml

Matrixpad.ui.h: makedoth Matrixpad.ui
	./makedoth Matrixpad.ui Matrixpad.ui.h

Matrixpad.xml.h: makedoth Matrixpad.xml
	./makedoth Matrixpad.xml Matrixpad.xml.h

makedoth: makedoth.c
	gcc -g -o makedoth makedoth.c 

clean:
	rm -f *.o Decoder Test Matrixpad Matrixpad.ui.h makedoth cluster standalone MakeMatrix
