/*Problema para practicar:
 * Sobre el problema de los fumadores, se crea una hebra proveedora que 
 * cada vez que el estanquero le avisa, rellena un buffer con total_productos
 * items y después avisa al estanquero para que este venda los productos del
 * buffer.
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

const int tam_vec=5,  
          nFumadores=3;

int vec[tam_vec];

Semaphore   fumadores[3] = { 0, 0, 0 },
            mostrador = 1,
            estanquero = 0,
            proveedora = 1;

mutex output;

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
      for (unsigned i = 0; i < tam_vec; i++)
      vec[i] = aleatorio<0, nFumadores-1>();
      output.lock();
      cout << "***La proveedora ha recargado 10 ingredientes: ***" << endl;
      for (unsigned i = 0; i < tam_vec; i++)
         cout << vec[i]<< ", ";
      cout << endl;
      output.unlock();
      
      sem_signal(estanquero);
   }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while (true){
      sem_wait(estanquero);
      for (unsigned i = 0; i < tam_vec; i++)
      {
         int ingrediente = vec[i];
         sem_wait(mostrador);
         output.lock();
         cout << "Estanquero produce ingrediente " << ingrediente << endl;
         output.unlock();
         sem_signal(fumadores[ingrediente]);
      }
      sem_signal(proveedora);
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    output.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    output.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    output.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    output.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true ){
      sem_wait(fumadores[num_fumador]);
      output.lock();
      cout << "\tEl fumador " << num_fumador << " retira su ingrediente" << endl;
      output.unlock();
      sem_signal(mostrador);
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   thread proveedora ( funcion_hebra_proveedora ),
          estanquero ( funcion_hebra_estanquero ),
          fumador0   ( funcion_hebra_fumador, 0 ),
          fumador1   ( funcion_hebra_fumador, 1 ),
          fumador2   ( funcion_hebra_fumador, 2 );

   proveedora.join();
   estanquero.join();
   fumador0.join();
   fumador1.join();
   fumador2.join();
}
