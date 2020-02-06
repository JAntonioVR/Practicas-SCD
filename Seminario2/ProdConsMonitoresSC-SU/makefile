.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors
hmonsrcs:= HoareMonitor.h HoareMonitor.cpp Semaphore.h Semaphore.cpp

all: x1 x2 x3 x4 x5 x6

x1: prodcons1_sc_LIFO_exe
	./$<

x2: prodcons1_sc_FIFO_exe
	./$<

x3: prodcons2_sc_LIFO_exe
	./$<

x4: prodcons2_sc_FIFO_exe
	./$<

x5: prodcons2_su_LIFO_exe
	./$<

x6: prodcons2_su_FIFO_exe
	./$<

prodcons1_sc_LIFO_exe: prodcons1_sc_LIFO.cpp
	$(compilador) $(opcionesc)  -o $@ $<

prodcons1_sc_FIFO_exe: prodcons1_sc_FIFO.cpp
	$(compilador) $(opcionesc)  -o $@ $<

prodcons2_sc_LIFO_exe: prodcons2_sc_LIFO.cpp
	$(compilador) $(opcionesc)  -o $@ $<

prodcons2_sc_FIFO_exe: prodcons2_sc_FIFO.cpp
	$(compilador) $(opcionesc)  -o $@ $<

prodcons2_su_LIFO_exe: prodcons2_su_LIFO.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp

prodcons2_su_FIFO_exe: prodcons2_su_FIFO.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp

clean:
	rm -f *_exe
