#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

const int nClientes = 6;  //(Clientes 0,1,2 tipo 0 y clientes 3,4,5 tipo 1)
                          // Un cliente n es tipo 0 si n/3==0, tipo 1 si n/3==1

mutex output;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio(){
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// funcion que espera un tiempo aleatorio simulando pelar a un cliente
void cortarPeloACliente(){
    // Calcular duracion del pelado
    chrono::milliseconds duracionPelado( aleatorio<20,200>() );
    // Mensaje por pantalla
    output.lock();
    cout << "El barbero está pelando a un cliente (" << duracionPelado.count() << 
            " milisegundos)" << endl;
    output.unlock();

    // espera bloqueada un tiempo igual a ''duracionPelado' milisegundos
    this_thread::sleep_for( duracionPelado );

    // informa de que ha terminado el pelado
    output.lock();
    cout << "El barbero ha acabado de pelar a un cliente" << endl;
    output.unlock();
}

//------------------------------------------------------------------------------
// funcion que espera un tiempo aleatorio simulando espera fuera de la barberia

void esperarFueraBarberia(int i){
    // Calcular duracion de la espera
    chrono::milliseconds duracionEspera( aleatorio<200,400>() );
    // Mensaje por pantalla
    output.lock();
    cout << "\tEl cliente " << i << " está esperando fuera de la barberia (" 
         << duracionEspera.count() 
         << " milisegundos)" << endl;
    output.unlock();

    // espera bloqueada un tiempo igual a ''duracionEspera' milisegundos
    this_thread::sleep_for( duracionEspera );

    // informa de que ha terminado la espera
    output.lock();
    cout << "\tEl cliente " << i << " finaliza su espera y va otra vez a pelarse" << endl;
    output.unlock();
}

//----------------------------------------------------------------------
// Declaración e implementación del monitor:

class Barberia : public HoareMonitor{
    private:
        CondVar
            salaEspera[2],
            sillon,
            barbero;
        int
            tipoSiguiente;
    public:
        void siguienteCliente();
        void finCliente();
        void cortarPelo(int i);
        Barberia();
};

void Barberia :: siguienteCliente(){
    if(salaEspera[tipoSiguiente].empty()){
        output.lock();
        cout << "El barbero se va a dormir" << endl;
        output.unlock();

        barbero.wait();
    }
    else{
        output.lock();
        cout << "El barbero va a llamar a alguien de la sala de espera" << endl;
        output.unlock();

        salaEspera[tipoSiguiente].signal();
    }
        
}

void Barberia :: finCliente(){
    output.lock();
    cout << "El barbero despide a un cliente" << endl;
    output.unlock();

    sillon.signal();
    tipoSiguiente = (tipoSiguiente==0 ? 1 : 0);
}

void Barberia :: cortarPelo(int i){
    int tipo = i/3;
    if(salaEspera[tipo].empty() && sillon.empty() && tipoSiguiente==(tipo)){
        output.lock();
        cout << "\tEl cliente " << i << " despierta al barbero" << endl;
        output.unlock();
        barbero.signal();
    }
    else{
        output.lock();
        cout << "\tHay gente en la sala de espera, el barbero esta ocupado o no toca el tipo " 
             << tipo <<". El cliente " << i << " espera." << endl;
        output.unlock();

        salaEspera[tipo].wait();
    }
        
    cout << "\tEl cliente " << i << " va a ser pelado" << endl;
    sillon.wait();
}

Barberia ::Barberia (){
    salaEspera[0] = newCondVar();
    salaEspera[1] = newCondVar();
    sillon  = newCondVar();
    barbero    = newCondVar();
    tipoSiguiente = 0;
}

//----------------------------------------------------------------------
// Funciones que ejecutan las hebras

void funcionHebraBarbero( MRef<Barberia> monitor ){
    while (true){
        monitor->siguienteCliente();
        cortarPeloACliente();
        monitor->finCliente();
    }
}

void funcionHebraCliente( MRef<Barberia> monitor, int i ){
    while (true)
    {
        monitor->cortarPelo(i);
        esperarFueraBarberia(i);
    }
}

//----------------------------------------------------------------------
// Funcion main

int main(){
    MRef<Barberia> barberia = Create<Barberia>();
    thread hebraBarbero, hebrasClientes[nClientes];
    hebraBarbero = thread( funcionHebraBarbero, barberia );
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i] = thread( funcionHebraCliente, barberia, i );
    
    hebraBarbero.join();
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i].join();

}