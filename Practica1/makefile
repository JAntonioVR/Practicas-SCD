.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

pc: prodcons_exe
	./$<

fu: fumadores_exe
	./$<

pcd: prodconsDinamico_exe
	./$<

pcFIFO: prodconsFIFO_exe
	./$<

pcLIFO: prodconsLIFO_exe
	./$<

pcvFIFO: prodconsVariosFIFO_exe
	./$<

pcvLIFO: prodconsVariosLIFO_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -pthread -o $@ $< Semaphore.cpp 

clean:
	rm -f *_exe
