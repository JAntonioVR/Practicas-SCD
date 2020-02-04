#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

const int nClientes = 6,
          nBarberos = 2;

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
void cortarPeloACliente(int i){
    // Calcular duracion del pelado
    chrono::milliseconds duracionPelado( aleatorio<20,200>() );
    // Mensaje por pantalla
    output.lock();
    cout << "El barbero " << i << " está pelando a un cliente (" << duracionPelado.count() << 
            " milisegundos)" << endl;
    output.unlock();

    // espera bloqueada un tiempo igual a ''duracionPelado' milisegundos
    this_thread::sleep_for( duracionPelado );

    // informa de que ha terminado el pelado
    output.lock();
    cout << "El barbero " << i << " ha acabado de pelar a un cliente" << endl;
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
            salaEspera,
            sillon[nBarberos],
            cama;
    public:
        void siguienteCliente(int i);
        void finCliente(int i);
        void cortarPelo(int i);
        Barberia();
};

void Barberia :: siguienteCliente(int i){
    if(salaEspera.empty()){
        output.lock();
        cout << "El barbero "<< i << " se va a dormir" << endl;
        output.unlock();

        cama.wait();
    }
    else{
        output.lock();
        cout << "El barbero " << i << " va a llamar a alguien de la sala de espera" << endl;
        output.unlock();

        salaEspera.signal();
    }
        
}

void Barberia :: finCliente(int i){
    output.lock();
    cout << "El barbero " << i << " despide a un cliente" << endl;
    output.unlock();

    sillon[i].signal();
}

void Barberia :: cortarPelo(int i){
    int nBarbero;
    bool barberoDisponible = false;
    if(salaEspera.empty()){
        for (nBarbero = 0; nBarbero < nBarberos && !barberoDisponible; nBarbero++)
        {
            if (sillon[nBarbero].empty())
            {
                output.lock();
                cout << "\tEl cliente " << i << " despierta a un barbero " << endl;
                output.unlock();
                barberoDisponible = true;
                cama.signal();
            }   
        } 
    }
    if (!barberoDisponible){ 
        output.lock();
        cout << "\tTodos los barberos están ocupados o hay gente en la sala de espera "
             << "El cliente " << i << " espera." << endl;
        output.unlock();
        salaEspera.wait();
    }
    
    output.lock();
    cout << "\tEl cliente " << i << " va a ser pelado" << endl;
    output.unlock();

    for (nBarbero = 0; nBarbero < nBarberos && !sillon[nBarbero].empty(); nBarbero++){}
    sillon[nBarbero].wait();
}

Barberia ::Barberia (){
    salaEspera = newCondVar();
    for (int i = 0; i < nBarberos; i++)
        sillon[i] = newCondVar();
    cama = newCondVar();
}

//----------------------------------------------------------------------
// Funciones que ejecutan las hebras

void funcionHebraBarbero( MRef<Barberia> monitor, int i ){
    while (true){
        monitor->siguienteCliente(i);
        cortarPeloACliente(i);
        monitor->finCliente(i);
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
    thread hebrasBarberos[nBarberos], hebrasClientes[nClientes];

    for (unsigned i = 0; i < nBarberos; i++)
        hebrasBarberos[i] = thread( funcionHebraBarbero, barberia, i );
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i] = thread( funcionHebraCliente, barberia, i );
    
    for (unsigned i = 0; i < nBarberos; i++)
        hebrasBarberos[i].join();
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i].join();

}