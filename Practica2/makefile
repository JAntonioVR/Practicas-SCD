.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors
hmonsrcs:= HoareMonitor.h HoareMonitor.cpp Semaphore.h Semaphore.cpp

x1: fumadoresSU_exe
	./$<

x2: barberoDurmiente_exe
	./$<

x3: barberoDurmienteVariosBarberos_exe
	./$<

x4: barberoDurmienteNBarberos1Cama_exe
	./$<

x5: barberoDurmienteSalaEspera_exe
	./$<

x6: fumadoresSUProveedora_exe
	./$<

x7: fumadoresSUProveedoraBuffer_exe
	./$<

x8: barberoDurmienteNBarberosAlternados_exe
	./$<

x9: fumadoresSUExtraño_exe
	./$<

x10: fumadoresSU2monitores_exe
	./$<

x11: NBarberos1Cama1sillon_exe
	./$<

x12: barberoDurmienteProveedor_exe
	./$<

x13: fumadoresSUFumEspecial_exe
	./$<

x14: fumadoresNEstanqueros_exe
	./$<

x15: fumadoresNEstanqueros1mostrador_exe
	./$<

x16: barberoDurmiente2_exe
	./$<

x17: barberoDurmienteProveedor2_exe
	./$<

x18: barberia2_exe
	./$<

fumadoresSU_exe: fumadoresSU.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmiente_exe: barberoDurmiente.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmienteVariosBarberos_exe: barberoDurmienteVariosBarberos.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmienteNBarberos1Cama_exe: barberoDurmienteNBarberos1Cama.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmienteSalaEspera_exe: barberoDurmienteSalaEspera.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

fumadoresSUProveedora_exe: fumadoresSUProveedora.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

fumadoresSUProveedoraBuffer_exe: fumadoresSUProveedoraBuffer.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmienteNBarberosAlternados_exe: barberoDurmienteNBarberosAlternados.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

fumadoresSUExtraño_exe: fumadoresSUExtraño.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

fumadoresSU2monitores_exe: fumadoresSU2monitores.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

NBarberos1Cama1sillon_exe: NBarberos1Cama1sillon.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmienteProveedor_exe: barberoDurmienteProveedor.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

fumadoresSUFumEspecial_exe: fumadoresSUFumEspecial.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

fumadoresNEstanqueros_exe: fumadoresNEstanqueros.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

fumadoresNEstanqueros1mostrador_exe: fumadoresNEstanqueros1mostrador.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmiente2_exe: barberoDurmiente2.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberoDurmienteProveedor2_exe: barberoDurmienteProveedor2.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

barberia2_exe: barberia2.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp Semaphore.cpp

clean:
	rm -f *_exe
