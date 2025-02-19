// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodcons_1.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// del productor/consumidor, con un único productor y un único consumidor.
// Opcion LIFO (stack)
//
// Historial:
// Creado en Julio de 2017
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"  // Para usar HoareMonitor

using namespace std ;
using namespace HM;

constexpr int
   num_items     = 40,     // número de items a producir/consumir
   nproductores  = 4,      // número de hebras productoras
   nconsumidores = 2;      // número de hebras consumidoras

mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items],    // contadores de verificación: producidos
   cont_cons[num_items],    // contadores de verificación: consumidos
   item_prod[nproductores]; // vector que cuenta el numero de items producidos por cada productor

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

int producir_dato(int indice_hebra)
{
   static int contador = 0;
   int dato ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   
   //Cada hebra produce un dato en función de cuántos haya creado anteriormente
   dato = indice_hebra*(num_items/nproductores)+item_prod[indice_hebra];

   mtx.lock();
   cout << "producido: " << dato << " por la hebra " << indice_hebra << endl << flush ;
   mtx.unlock();

   //Actualizar contadores de verificacion y número de items producidos por hebra
   item_prod[indice_hebra]++;
   cont_prod[contador] ++ ;
   contador++;
   
   return dato ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
   for (unsigned i = 0; i < nproductores; i++)
      item_prod[i] = 0;
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SC, un prod. y un cons.

class ProdCons2SU : public HoareMonitor    // El monitor hereda de HoareMonitor
{
   private:
      static const int           // constantes:
            num_celdas_total = 10;   //  núm. de entradas del buffer
      int                        // variables permanentes
            buffer[num_celdas_total],//  buffer de tamaño fijo, con los datos
            primera_libre,           //  indice de celda de la próxima inserción
            primera_ocupada,         //  indice de celda de la proxima retirada
            num_celdas_ocupadas;     //  numero de celdas ocupadas
      mutex
            cerrojo_monitor ;        // cerrojo del monitor
      CondVar                        // colas condicion (tipo CondVar):
            ocupadas,                //  cola donde esperan los consumidores
            libres ;                 //  cola donde esperan los productores
   public:                    // constructor y métodos públicos
      ProdCons2SU(  ) ;           // constructor
      int  leer();                // extraer un valor (sentencia L) (consumidores)
      void escribir( int valor ); // insertar un valor (sentencia E) (productores)
} ;
// -----------------------------------------------------------------------------

ProdCons2SU::ProdCons2SU(  )
{
   primera_libre        = 0 ;
   primera_ocupada      = 0;
   num_celdas_ocupadas  = 0;
   ocupadas             = newCondVar();
   libres               = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdCons2SU::leer(  )
{

   // esperar bloqueado hasta que 0 < num_celdas_ocupadas
   while ( num_celdas_ocupadas == 0 )
      ocupadas.wait( );

   // hacer la operación de lectura, actualizando estado del monitor
   assert( num_celdas_ocupadas > 0  );
   const int valor = buffer[primera_ocupada] ;
   primera_ocupada = (primera_ocupada+1)%num_celdas_total;
   num_celdas_ocupadas--;

   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdCons2SU::escribir( int valor )
{
   // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
   while ( num_celdas_ocupadas == num_celdas_total )
      libres.wait( );

   assert( num_celdas_ocupadas < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre = (primera_libre+1)%num_celdas_total ;
   num_celdas_ocupadas++;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdCons2SU> monitor, int indice_hebra )
{
   // Cada hebra produce num_items/nproductores items
   for( unsigned i = 0 ; i < num_items/nproductores ; i++ ){
      int valor = producir_dato(indice_hebra) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdCons2SU> monitor )
{
   // Cada hebra consume num_items/nproductores items
   for( unsigned i = 0 ; i < num_items/nconsumidores ; i++ ){
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}

// -----------------------------------------------------------------------------

int main()
{
    cout << "-------------------------------------------------------------------------------------" << endl
            << "Problema de los productores-consumidores (Varios prod/cons, Monitor SU, buffer FIFO). " << endl
            << "--------------------------------------------------------------------------------------" << endl
            << flush ;

      MRef<ProdCons2SU> monitor = Create<ProdCons2SU>(); ;

         
      thread  hebras_productoras[nproductores],
               hebras_consumidoras[nconsumidores];

      for (int i = 0; i < nproductores; i++)
         hebras_productoras[i] = thread( funcion_hebra_productora, monitor, i );
      
      for (int i = 0; i < nconsumidores; i++)
         hebras_consumidoras[i] = thread( funcion_hebra_consumidora, monitor);
      
      for (int i = 0; i < nproductores; i++)
         hebras_productoras[i].join();
      
      for (int i = 0; i < nconsumidores; i++)
         hebras_consumidoras[i].join();

      // comprobar que cada item se ha producido y consumido exactamente una vez
      test_contadores() ;
}