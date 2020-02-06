/* Problema para practicar:
 * Consiste en el barbero durmiente pero en este caso la sala de espera se asume que tiene
 * un numero finito de sillas. Cuando la sala de espera está completa, el proximo cliente
 * que llega espera en la puerta de la barberia. Cuando un cliente va a pelarse, avisa a un
 * cliente que espera en la puerta para que este ocupe su lugar en la sala de espera.
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

const int nClientes = 10,
          nSillas   = 5;

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
            salaEspera,
            sillon,
            barbero,
            puerta;
    public:
        void siguienteCliente();
        void finCliente();
        void cortarPelo(int i);
        Barberia();
};

void Barberia :: siguienteCliente(){
    if(salaEspera.empty()){
        output.lock();
        cout << "El barbero se va a dormir" << endl;
        output.unlock();

        barbero.wait();
    }
    else{
        output.lock();
        cout << "El barbero va a llamar a alguien de la sala de espera" << endl;
        output.unlock();

        salaEspera.signal();
    }
        
}

void Barberia :: finCliente(){
    output.lock();
    cout << "El barbero despide a un cliente" << endl;
    output.unlock();

    sillon.signal();
}

void Barberia :: cortarPelo(int i){
    if(salaEspera.empty() && sillon.empty()){
        output.lock();
        cout << "\tEl cliente " << i << " despierta al barbero" << endl;
        output.unlock();
        barbero.signal();
    }
    else{
        output.lock();
        cout << "" ;
        output.unlock();

        if(salaEspera.get_nwt()<nSillas){
            output.lock();
            cout << "\tHay gente en la sala de espera o el barbero esta ocupado." 
                 << "El cliente " << i << " espera en la sala de espera" << endl;
            output.unlock();
            salaEspera.wait();
        }
        else{
            output.lock();
            cout << "\tHay gente en la sala de espera o el barbero esta ocupado." 
                 << "El cliente " << i << " espera en la puerta" << endl;
            output.unlock();
            puerta.wait();

            assert(salaEspera.get_nwt()<nSillas);
            output.lock();
            cout << "El cliente " << i << " espera en la sala de espera" << endl;
            output.unlock();
            salaEspera.wait();
        }
    }


        
    cout << "\tEl cliente " << i << " va a ser pelado" << endl;
    if(!puerta.empty()){
        output.lock();
        cout << "\tEl cliente " << i << " llama a alguien de la puerta" << endl;
        output.unlock();
        puerta.signal();
    }
    sillon.wait();
}

Barberia ::Barberia (){
    salaEspera = newCondVar();
    sillon     = newCondVar();
    barbero    = newCondVar();
    puerta     = newCondVar();
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