/* Problema para practicar:
 * Sobre el problema de los fumadores, se crea una hebra proveedora que 
 * crea y suministra uno a uno los posibles ingredientes y este los pone
 * a la venta antes de volver a avisar al proveedor para que este le de 
 * otro ingrediente.
 * Además, se lleva la cuenta de los cigarrillos que ha fumado cada fumador
 * y cada vez que uno de ellos se termina un cigarro se imprime un mensaje
 * diciendo el número de cigarrillos que lleva dicho fumador y el número de
 * cigarros que llevan entre todos los fumadores.
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


const int nFumadores = 3;
int producto,
    totalCigarros = 0;

int cigarros[nFumadores] = {0, 0, 0};

Semaphore   fumadores[nFumadores] = { 0, 0, 0 },
            mostrador = 1,
            proveedora = 1,
            estanquero = 0;

mutex total;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


void funcion_hebra_proveedora( )
{
    while (true)
    {
        sem_wait(proveedora);
        producto = aleatorio<0, nFumadores-1>();
        cout << "****La proveedora ha fabricado el ingrediente " << producto << endl;
        sem_signal(estanquero);
    }
    
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while (true){
      sem_wait(estanquero);
      int ingrediente = producto;
      sem_signal(proveedora);
      sem_wait(mostrador);
      cout << "Estanquero pone a la venta ingrediente " << ingrediente << endl;
      sem_signal(fumadores[ingrediente]);
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    cigarros[num_fumador]++;
    total.lock();
    totalCigarros++;
    total.unlock();

    cout << "Fumador " << num_fumador << "  : termina de fumar" << endl
         << "Ha fumado " << cigarros[num_fumador] << " cigarros " << endl
         << "En total se han fumado " << totalCigarros << " cigarros " << endl;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true ){
      sem_wait(fumadores[num_fumador]);
      cout << "\tEl fumador " << num_fumador << " retira su ingrediente" << endl;
      sem_signal(mostrador);
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   thread hebraProveedora ( funcion_hebra_proveedora ),
          hebraEstanquero ( funcion_hebra_estanquero ),
          fumador0   ( funcion_hebra_fumador, 0 ),
          fumador1   ( funcion_hebra_fumador, 1 ),
          fumador2   ( funcion_hebra_fumador, 2 );
   
   hebraProveedora.join();
   hebraEstanquero.join();
   fumador0.join();
   fumador1.join();
   fumador2.join();
}
