/* Examen del Grupo 1 de Practicas Curso 2019/2020
 * Sobre el problema de los fumadores, se crea una hebra proveedora y se
 * crean n estanqueros (el enunciado pedía dos), de forma que la proveedora
 * aleatoriamente suministra a uno de los estanqueros un pack de 5 suministros.  
 * Cada estanquero pone a la venta en el único mostrador uno de los ingredientes
 * y cuando se le agotan los suministros avisa a la proveedora que puede
 * suministrarle o no.
 * 
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

const int nSuministros=5,  
          nFumadores=3,
          nEstanqueros=2;

Semaphore   fumadores[nFumadores] = { 0, 0, 0 },
            mostrador = 1,
            estanqueros[nEstanqueros] = {0,0},
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
    int estanquero;
   while (true)
   {
      sem_wait(proveedora);
      estanquero = aleatorio<0, nEstanqueros-1>();
      output.lock();
      cout << "***La proveedora ha recargado al estanquero " << estanquero << "***" << endl;
      output.unlock();
      sem_signal(estanqueros[estanquero]);
   }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( int n_estanquero )
{
   while (true){
      sem_wait(estanqueros[n_estanquero]);
      for (unsigned i = 0; i < nSuministros; i++)
      {
         int ingrediente = aleatorio<0, nFumadores-1>();
         sem_wait(mostrador);
         output.lock();
         cout << "Estanquero " << n_estanquero << " produce ingrediente " << ingrediente << endl;
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
          estanquero0 ( funcion_hebra_estanquero, 0 ),
          estanquero1 ( funcion_hebra_estanquero, 1 ),
          fumador0   ( funcion_hebra_fumador, 0 ),
          fumador1   ( funcion_hebra_fumador, 1 ),
          fumador2   ( funcion_hebra_fumador, 2 );

   proveedora.join();
   estanquero0.join();
   estanquero1.join();
   fumador0.join();
   fumador1.join();
   fumador2.join();
}
