/* Sobre el problema de los fumadores, se crea una hebra proveedora que cada vez
 * que el estanquero necesita suministro se lo solicita a la proveedora,
 * esta procesa el pedido (retardo aleatorio) y cuando acaba avisa al estanquero
 * de que su pedido está listo
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

const int nFumadores = 4,
          nSuministros = 5;

Semaphore   fumadores[4] = { 0, 0, 0, 0 },
            mostrador = 1,
            suministro = 0,
            solicitar = 1;


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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while (true){
      sem_wait(suministro);
      for (int i = 0; i < nSuministros; i++)
      {
        int ingrediente = aleatorio<0,nFumadores-1>();
        sem_wait(mostrador);
        cout << "Estanquero produce ingrediente " << ingrediente << endl;
        sem_signal(fumadores[ingrediente]);
      }
      sem_signal(solicitar);
      
      
      
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

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

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

void funcion_hebra_suministrador(){
    while (true)
    {
        sem_wait(solicitar);
        this_thread::sleep_for( chrono::milliseconds(aleatorio<20, 200>()));
        cout << "***El suministrador ha suministrado***"  << endl;
        sem_signal(suministro);
    }
}

//----------------------------------------------------------------------

int main()
{
   thread suministrador ( funcion_hebra_suministrador ),
          estanquero ( funcion_hebra_estanquero ),
          fumadores[nFumadores];
    for (int i = 0; i < nFumadores; i++)
        fumadores[i] =  thread(funcion_hebra_fumador, i);
    

   suministrador.join();
   estanquero.join();
   for (int i = 0; i < nFumadores; i++)
        fumadores[i].join();
}
