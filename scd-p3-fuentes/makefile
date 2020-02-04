.SUFFIXES:
.PHONY:    pc,pc2,pcm,fp,f,fi,fc,clean

compilador := mpicxx -std=c++11
flagsc     := -g -Wall

pc: prodcons_exe
	mpirun -np  3 ./$<

pc2: prodcons2_exe
	mpirun -np  3 ./$<

pcm: prodcons2-mu_exe
	mpirun -np  10 ./$<

pcmlifo: prodcons2-mu-lifo_exe
	mpirun -np  10 ./$<

pcmodo: prodcons2-mu-modos_exe
	mpirun -np  10 ./$<

pcp: prodcons2-mu-paridad_exe
	mpirun -np  9 ./$<

fp: filosofos-plantilla_exe
	mpirun -np 10 ./$<

fi: filosofos-interb_exe
	mpirun -np 10 ./$<

f: filosofos_exe
	mpirun -np 10 ./$<

fc: filosofos-cam_exe
	mpirun -np 11 ./$<

fcs: filosofos-cam-sondeo_exe
	mpirun -np 11 ./$<

fco: filosofos-cam-orden_exe
	mpirun -np 11 ./$<

fcp: filosofos-cam-probe_exe
	mpirun -np 11 ./$<

c4: camarero4_exe
	mpirun -np 11 ./$<

%_exe: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
