/* Examen Practica 2:
 * Sobre el problema del barbero durmiente, implementar los cambios necesarios para que
 * el barbero pele a sus clientes de dos en dos. Si un cliente llega a la barberia y la
 * sala de espera está vacía o el barbero está ocupado, se sienta a esperar. Cuando
 * llega un cliente y hay una persona más en la sala de espera, este despierta al barbero
 * y espera. TODOS LOS CLIENTES DEBEN PASAR POR LA SALA DE ESPERA. Por su parte, el barbero
 * si no hay al menos dos personas en la sala de espera se duerme, en caso contrario hace pasar
 * a dos clientes y cuando ambos están sentados en el sillon, los pela simultaneamente y cuando
 * acaba despide a los dos.
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

const int nClientes = 6;

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
    cout << "El barbero está pelando a dos clientes (" << duracionPelado.count() << 
            " milisegundos)" << endl;
    output.unlock();

    // espera bloqueada un tiempo igual a ''duracionPelado' milisegundos
    this_thread::sleep_for( duracionPelado );

    // informa de que ha terminado el pelado
    output.lock();
    cout << "El barbero ha acabado de pelar a dos clientes" << endl;
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

//---------------------------------------------------------------------------
// Función que ejecuta el barbero, si no hay al menos dos clientes esperando
// el barbero se duerme. Una vez despierto (se supone que hay al menos 2
// clientes) el barbero llama a dos clientes y espera a que estos dos se
// sienten en el sillón

void Barberia :: siguienteCliente(){
    if(salaEspera.get_nwt()<2){
        output.lock();
        cout << "No hay suficiente gente a la que pelar. El barbero se va a dormir" << endl;
        output.unlock();

        barbero.wait();
    }

    output.lock();
    cout << "El barbero va a llamar a dos clientes de la sala de espera" << endl;
    output.unlock();

    salaEspera.signal();
    salaEspera.signal();

    if (sillon.get_nwt()!=2)
        barbero.wait();
}

//-----------------------------------------------------------------------
// Función que ejecuta el barbero una vez acaba el pelado de dos clientes.
// El barbero despide a los clientes y libera el sillón.

void Barberia :: finCliente(){
    output.lock();
    cout << "El barbero despide a dos clientes" << endl;
    output.unlock();

    sillon.signal();
    sillon.signal();
}

//----------------------------------------------------------------------
// Función que ejecutan los clientes. Si el barbero está durmiendo y 
// hay alguien esperando el nuevo cliente despierta al barbero, en
// otro caso no lo despierta. En todo caso el cliente se espera en la 
// sala de espera. Cuando llega su turno, si es el segundo avisa al 
// barbero y en cualquier caso se sienta en el sillón de pelar.

void Barberia :: cortarPelo(int i){
    if(!barbero.empty() && salaEspera.get_nwt()>=1){ // 
        output.lock();
        cout << "\tEl cliente " << i << " despierta al barbero y espera" << endl;
        output.unlock();
        barbero.signal();
    }
    else{                  
        output.lock();
        cout << "\tLa sala de espera está vacía o el barbero esta ocupado. "
             << "El cliente " << i << " espera." << endl;
        output.unlock();
    }
    salaEspera.wait();
        
    cout << "\tEl cliente " << i << " va a ser pelado" << endl;

    if(sillon.get_nwt()==1) 
        barbero.signal();
    sillon.wait();
}

//----------------------------------------------------------------------
// Constructor e inicializador de la clase

Barberia ::Barberia (){
    salaEspera = newCondVar();
    sillon     = newCondVar();
    barbero    = newCondVar();
}

/**********************************************************************/
//----------------------------------------------------------------------
// Funciones que ejecutan las hebras:
//----------------------------------------------------------------------
/**********************************************************************/

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

/**********************************************************************/
//----------------------------------------------------------------------
// Funcion main:
//----------------------------------------------------------------------
/**********************************************************************/

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