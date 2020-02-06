/* Problema para practicar:
 * Consiste en el barbero durmiente pero en este caso hay una hebra proveedora de tijeras
 * que en cada pelado proporciona al barbero unas tijeras nuevas y espera a un proximo aviso
 * por parte del barbero.
 * Implementación usando dos monitores diferentes.
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

const int nClientes = 3;

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
// Declaración e implementación del barberia:

class Barberia : public HoareMonitor{
    private:
        CondVar
            salaEspera,
            sillon,
            barbero;
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
        cout << "\tHay gente en la sala de espera o el barbero esta ocupado. "
             << "El cliente " << i << " espera." << endl;
        output.unlock();

        salaEspera.wait();
    }
        
    cout << "\tEl cliente " << i << " va a ser pelado" << endl;
    sillon.wait();
}

Barberia ::Barberia (){
    salaEspera = newCondVar();
    sillon     = newCondVar();
    barbero    = newCondVar();
}

//----------------------------------------------------------------------
// barberia de la proveedora
class Proveedora : public HoareMonitor{
    private:
        CondVar barbero, proveedor;
    public:
        void reponer();
        void esperarReponer();
        void recogerTijeras();
        Proveedora();
};

void Proveedora :: reponer(){
    output.lock();
    cout << "\t\tLa proveedora proporciona tijeras nuevas" << endl;
    output.unlock();
    if(!barbero.empty())
        barbero.signal();
}

void Proveedora :: esperarReponer(){
    if(barbero.empty())
       proveedor.wait(); 
}

void Proveedora :: recogerTijeras(){
    proveedor.signal();
    barbero.wait();
}

Proveedora :: Proveedora(){
    barbero = newCondVar();
    proveedor = newCondVar();
}


//----------------------------------------------------------------------
// Funciones que ejecutan las hebras

void funcionHebraProveedor( MRef<Proveedora> proveedora ){
    while (true){
        proveedora->esperarReponer();
        proveedora->reponer();
    }
    
}

void funcionHebraBarbero( MRef<Barberia> barberia, MRef<Proveedora> proveedora ){
    while (true){
        barberia->siguienteCliente();
        cortarPeloACliente();
        barberia->finCliente();
        proveedora->recogerTijeras();
    }
}

void funcionHebraCliente( MRef<Barberia> barberia, int i ){
    while (true)
    {
        barberia->cortarPelo(i);
        esperarFueraBarberia(i);
    }
}

//----------------------------------------------------------------------
// Funcion main

int main(){
    MRef<Barberia> barberia = Create<Barberia>();
    MRef<Proveedora> proveedora = Create<Proveedora>();
    thread hebraProveedora, hebraBarbero, hebrasClientes[nClientes];
    hebraProveedora = thread( funcionHebraProveedor, proveedora);
    hebraBarbero = thread( funcionHebraBarbero, barberia, proveedora );
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i] = thread( funcionHebraCliente, barberia, i );
    
    hebraProveedora.join();
    hebraBarbero.join();
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i].join();

}