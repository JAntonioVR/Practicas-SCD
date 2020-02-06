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
            ocupadas = 0;

int primera_libre = 0;
int primera_ocupada = 0;

mutex mtx;

bool LIFO = true;          // por defecto se usa la gestión por LIFO
static int contador = 0 ;

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

void  funcion_hebra_productora(  )
{

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      sem_wait(libres);
      mtx.lock();
      if(LIFO){
         buffer[primera_libre] = dato;
         primera_libre++;
      }
      else{
         buffer[primera_libre] = dato;
         primera_libre = (primera_libre+1) % tam_vec;
      }
      mtx.unlock();  
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      sem_wait(ocupadas);
      mtx.lock();
      if(LIFO){
         primera_libre--;
         dato = buffer[primera_libre];
      }
      else{
         dato = buffer[primera_ocupada];
         primera_ocupada = (primera_ocupada+1)%tam_vec;
      }
      mtx.unlock();
      sem_signal(libres);
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main(int argc, char ** argv)
{

   //Comprobamos argumentos
   if( argc > 2 || (argc==2 && strcmp(argv[1], "LIFO" )!=0 &&  strcmp(argv[1], "FIFO" )!=0 ) ){
      cout << "Error en los argumentos. Uso: " << endl <<
               argv[0] << " LIFO | FIFO" << endl <<
              "\tUse la opción \'LIFO\' para gestionar el buffer con LIFO." << endl <<
              "\tUse la opción \'FIFO\' para gestionar el buffer con FIFO." << endl <<
              "Por defecto, si no especifica ningún argumento se usará LIFO." << endl;
      exit(-1);
   }

   //Asignamos true o false a LIFO según los argumentos
   if(argc==2){
      if( !strcmp(argv[1], "LIFO" ) )
         LIFO = true;
      else 
         LIFO = false;
   }
   
   if(LIFO)
      cout << "--------------------------------------------------------" << endl
           << "Problema de los productores-consumidores (solución LIFO)." << endl
           << "--------------------------------------------------------" << endl
           << flush ;
   else
      cout << "--------------------------------------------------------" << endl
           << "Problema de los productores-consumidores (solución FIFO)." << endl
           << "--------------------------------------------------------" << endl
           << flush ;
   

   thread productor ( funcion_hebra_productora ),
          consumidor( funcion_hebra_consumidora );
   
   productor.join();
   consumidor.join();
   
   test_contadores();
}
