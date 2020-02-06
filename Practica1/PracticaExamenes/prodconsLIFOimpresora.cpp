/* Sobre el problema del productor-consumidor, se extiende a varios productores
 * y varios consumidores y se crea una hebra impresora que cada 5 items consumidos
 * por cualesquiera de los consumidores imprime un mensaje informativo por pantalla.
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <cstring>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 40 ,   // número de items
	       tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}, // contadores de verificación: consumidos
          buffer[tam_vec];

Semaphore   libres   = tam_vec,
            ocupadas = 0,
            impresora = 0;

int primera_libre = 0;
int primera_ocupada = 0;

int nProductores   = 3,
    nConsumidores  = 5,
    nIterImpresora = 4,
    nConsumidos    = 0;

mutex mtx, mtxImpresora;



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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora( int n_hebra )
{

   for( unsigned i = n_hebra ; i < num_items ; i+=nProductores )
   {
      int dato = producir_dato() ;
      sem_wait(libres);
      mtx.lock();
      buffer[primera_libre] = dato;
      primera_libre++;
      mtx.unlock();  
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int n_hebra )
{
   for( unsigned i = n_hebra ; i < num_items ; i+=nConsumidores )
   {
      int dato ;
      sem_wait(ocupadas);
      mtx.lock();
      primera_libre--;
      dato = buffer[primera_libre];
      mtx.unlock();
      sem_signal(libres);
      consumir_dato( dato );
      mtxImpresora.lock();
      nConsumidos++;
      if(!(nConsumidos % nIterImpresora))
        sem_signal(impresora);
      mtxImpresora.unlock();
    }
}

void funcion_hebra_impresora( )
{
    for (unsigned i = 0; i < num_items/nIterImpresora; i++)
    {
        sem_wait(impresora);
        cout << "***********************************************" << endl
             << "**** Se ha consumido un paquete de " << nIterImpresora << " items ****" << endl
             << "***********************************************" << endl;
    }
    
}



//----------------------------------------------------------------------

int main(int argc, char ** argv)
{

   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;
   

   thread productores[nProductores], consumidores[nConsumidores], impresora;
   impresora = thread(funcion_hebra_impresora);
   for (unsigned i = 0; i < nProductores; i++)
      productores[i] = thread( funcion_hebra_productora, i );
   for (unsigned i = 0; i < nConsumidores; i++)
      consumidores[i] = thread( funcion_hebra_consumidora, i );
   
   impresora.join();
   for (unsigned i = 0; i < nProductores; i++)
      productores[i].join();
   for (unsigned i = 0; i < nConsumidores; i++)
      consumidores[i].join();


   test_contadores();
}
