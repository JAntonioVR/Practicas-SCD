/* Problema para practicar:
 * Consiste en el barbero durmiente pero en este caso hay N barberos en lugar de uno
 * solo y cada barbero atiende una tanda de clientes y una vez se duerme, el proximo
 * barbero en despertarse y trabajar es otro diferente.
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

const int nClientes = 3,
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
            sillon,
            barbero;
    public:
        void siguienteCliente(int i);
        void finCliente(int i);
        void cortarPelo(int i);
        Barberia();
};

void Barberia :: siguienteCliente(int i){
    if(salaEspera.empty() || !sillon.empty()){
        output.lock();
        cout << "El barbero "<< i << " se va a dormir" << endl;
        output.unlock();

        barbero.wait();
    }
    else{
        assert(salaEspera.get_nwt()>0 && sillon.empty());
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

    sillon.signal();

}

void Barberia :: cortarPelo(int i){
    int nBarbero;
    if(salaEspera.empty() && sillon.empty()){
        output.lock();
        cout << "El cliente " << i << " despierta a un barbero" << endl;
        output.unlock();
        barbero.signal();
    }
    else{ 
        output.lock();
        cout << "\tEl sillon esta ocupado o hay gente en la sala de espera "
             << "El cliente " << i << " espera." << endl;
        output.unlock();
        salaEspera.wait();
    }
    
    output.lock();
    cout << "\tEl cliente " << i << " va a ser pelado" << endl;
    output.unlock();

    sillon.wait();
}

Barberia ::Barberia (){
    salaEspera = newCondVar();
    sillon = newCondVar();
    barbero = newCondVar();
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