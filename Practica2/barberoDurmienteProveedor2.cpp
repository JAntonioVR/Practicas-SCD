/* Problema para practicar:
 * Consiste en el barbero durmiente pero en este caso hay una hebra proveedora de tijeras
 * que proporciona al barbero M tijeras nuevas y espera a un proximo aviso por parte del barbero.
 * El barbero usa unas tijeras distintas en cada pelado y una vez las usa una vez las desecha,
 * por lo que tiene que llamar al proveedor cada M cortes de pelo.
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

const int nClientes = 3,
          M         = 5;

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

/**********************************************************************/
//----------------------------------------------------------------------
// Funciones de espera:
//----------------------------------------------------------------------
/**********************************************************************/


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

/**********************************************************************/
//----------------------------------------------------------------------
// Declaración e implementación del monitor:
//----------------------------------------------------------------------
/**********************************************************************/

class Barberia : public HoareMonitor{
    private:
        CondVar
            salaEspera,             // Cola de condición de la sala de espera
            sillon,                 // Cola de condición del sillón de la barbería
            barbero;                // Cola de condición para cuando el barbero duerme
    public:
        void siguienteCliente();    // Función que duerme al barbero o pasa un cliente
        void finCliente();          // Función que despide a un cliente y libera el sillon
        void cortarPelo(int i);     // Funcion que ejecuta un cliente para cortarse el pelo
        Barberia();                 // Constructor de la clase
};

//-----------------------------------------------------------------------
// Función que ejecuta el barbero, si no hay nadie en la sala de espera
// el barbero se duerme, si hay alguien lo deja pasar para pelarlo

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

//-----------------------------------------------------------------------
// Función que ejecuta el barbero una vez acaba el pelado de un cliente.
// El barbero despide al cliente y libera el sillón.

void Barberia :: finCliente(){
    output.lock();
    cout << "El barbero despide a un cliente" << endl;
    output.unlock();


    assert(!sillon.empty());
    sillon.signal();
}

//----------------------------------------------------------------------
// Función que ejecutan los clientes. Si no hay nadie en la sala de
// espera y el sillón está vacío, el cliente despierta al barbero, en
// otro caso se espera en la sala de espera. Cuando llega su turno,
// se sienta en el sillon de pelar.

void Barberia :: cortarPelo(int i){
    if(salaEspera.empty() && sillon.empty() && !barbero.empty()){
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

//----------------------------------------------------------------------
// Constructor e inicializador de la clase

Barberia ::Barberia (){
    salaEspera = newCondVar();
    sillon     = newCondVar();
    barbero    = newCondVar();
}

class Proveedor : public HoareMonitor{
    private:
        int 
            restantes;
        CondVar
            proveedor,
            barbero;
    public:
        void reponer();
        void esperaReponer();
        void recogerTijeras();
        Proveedor();
};

Proveedor::Proveedor(){
    restantes = M;
    proveedor = newCondVar();
    barbero   = newCondVar();
}

void Proveedor :: reponer(){
    output.lock();
    cout << "\t\tEl proveedor repone " << M << " tijeras." << endl;
    output.unlock();
    restantes = M;
    if(!barbero.empty())
        barbero.signal();
}

void Proveedor :: esperaReponer(){
    if(restantes > 0)
        proveedor.wait();
    assert(restantes == 0);
}

void Proveedor :: recogerTijeras(){
    if(restantes == 0){
        output.lock();
        cout << "El barbero necesita tijeras" << endl;
        output.unlock();
        proveedor.signal();
        barbero.wait();
    }
    assert(restantes > 0);

    restantes--;

}


/**********************************************************************/
//----------------------------------------------------------------------
// Funciones que ejecutan las hebras:
//----------------------------------------------------------------------
/**********************************************************************/

void funcionHebraProveedor( MRef<Proveedor> proveedor ){
    while (true){
        proveedor->esperaReponer();
        proveedor->reponer();
    }
}


void funcionHebraBarbero( MRef<Proveedor> proveedor, MRef<Barberia> barberia ){
    while (true){
        proveedor->recogerTijeras();
        barberia->siguienteCliente();
        cortarPeloACliente();
        barberia->finCliente();
    }
}

void funcionHebraCliente( MRef<Barberia> barberia, int i ){
    while (true)
    {
        barberia->cortarPelo(i);
        esperarFueraBarberia(i);
    }
}

/**********************************************************************/
//----------------------------------------------------------------------
// Funcion main:
//----------------------------------------------------------------------
/**********************************************************************/

int main(){
    MRef<Proveedor> proveedor = Create<Proveedor>();
    MRef<Barberia> barberia = Create<Barberia>();
    thread hebraProveedor, hebraBarbero, hebrasClientes[nClientes];

    hebraProveedor = thread( funcionHebraProveedor, proveedor );
    hebraBarbero = thread( funcionHebraBarbero, proveedor, barberia );
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i] = thread( funcionHebraCliente, barberia, i );
    
    hebraProveedor.join();
    hebraBarbero.join();
    for (unsigned i = 0; i < nClientes; i++)
        hebrasClientes[i].join();

}