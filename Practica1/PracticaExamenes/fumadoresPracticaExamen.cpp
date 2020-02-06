/* Problema para practicar
 * Sobre el problema de los fumadores, en este caso hay 3 estancos, cada
 * uno con su propio estanquero y sus 3 clientes habituales. Es decir, hay
 * 3 estancos, 3 estanqueros, 3 mostradores y 9 clientes, 3 por cada estanco.
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

int nFumadores = 3,
    nEstancos  = 3;

Semaphore   fumadores[nEstancos][nFumadores] = {{0,0,0}, {0,0,0},{0,0,0}},
            mostrador[nEstancos] = {1,1,1};


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

void funcion_hebra_estanquero( int indice )
{
   while (true){
      int ingrediente = aleatorio<0,2>();
      sem_wait(mostrador[indice]);
      cout << "Estanquero "<< indice << " produce ingrediente " << ingrediente << endl;
      sem_signal(fumadores[indice][ingrediente]);
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador, int num_estanco )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << " del estanco " << num_estanco << " :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << " del estanco " << num_estanco << " : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador, int num_estanco )
{
   while( true ){
      sem_wait(fumadores[num_estanco][num_fumador]);
      cout << "\tEl fumador " << num_fumador << " del estanco " << num_estanco << " retira su ingrediente" << endl;
      sem_signal(mostrador[num_estanco]);
      fumar(num_fumador, num_estanco);
   }
}

//----------------------------------------------------------------------

int main()
{
   thread estanqueros[nEstancos], hookeros[nEstancos][nFumadores];
   for(int i = 0; i<nEstancos; i++){
      estanqueros[i] = thread(funcion_hebra_estanquero, i);
      for(int j = 0; j < nFumadores; j++)
         hookeros[i][j] = thread(funcion_hebra_fumador, j, i);
   }

   for(int i = 0; i<nEstancos; i++){
      estanqueros[i].join();
      for(int j = 0; j < nFumadores; j++)
         hookeros[i][j].join();
   }
}
