.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors
hmonsrcs:= HoareMonitor.h HoareMonitor.cpp Semaphore.h Semaphore.cpp

x1: monitor_em_exe
	./$<

x2: barrera1_sc_exe
	./$<

x3: barrera2_sc_exe
	./$<

x4: barrera2_su_exe
	./$<

x5: prodcons1_sc_exe_LIFO
	./$<

x6: prodcons2_sc_exe_LIFO
	./$<

x7: prodcons2_su_exe_LIFO
	./$<

monitor_em_exe: monitor_em.cpp
	$(compilador) $(opcionesc)  -o $@ $<

barrera1_sc_exe: barrera1_sc.cpp
	$(compilador) $(opcionesc)  -o $@ $<

barrera2_sc_exe: barrera2_sc.cpp
	$(compilador) $(opcionesc)  -o $@ $<

barrera2_su_exe: barrera2_su.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

prodcons1_sc_exe_LIFO: prodcons1_sc_LIFO.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<

prodcons2_sc_exe_LIFO: prodcons2_sc_LIFO.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<

prodcons2_su_exe: prodcons2_su.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp

clean:
	rm -f *_exe
