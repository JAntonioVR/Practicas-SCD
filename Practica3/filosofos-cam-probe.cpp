/* Problema para practicar
 * Sobre el problema de los filosofos, implementar el proceso del camarero
 * de forma que antes de recibir ningún mensaje de ningún filosofo utiliza
 * la función probe para saber qué filósofo le enviará la próxima solicitud
 */ 
// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

#define ETIQ_SENTARSE 0
#define ETIQ_LEVANTARSE 1

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos + 1,
   id_camarero   = num_procesos-1;


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

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % (num_procesos-1), //id. tenedor izq.
      id_ten_der = (id+num_procesos-2) % (num_procesos-1), //id. tenedor der.
      id_camarero = num_procesos-1,
      valor = 0;

  while ( true )
  {
    cout << "Filosofo " << id << " solicita sentarse" << endl;
    MPI_Ssend(&valor, 1, MPI_INT, id_camarero, ETIQ_SENTARSE, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id << " se sienta y solicita ten. izq." <<id_ten_izq <<endl;
    // ... solicitar tenedor izquierdo
    MPI_Ssend(&valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    // ... solicitar tenedor derecho
    MPI_Ssend(&valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo
    MPI_Ssend(&valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho
    MPI_Ssend(&valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    cout << " Filosofo " << id << " se levanta de la mesa" << endl;
    MPI_Ssend(&valor, 1, MPI_INT, id_camarero, ETIQ_LEVANTARSE, MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
     MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     id_filosofo = estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     MPI_Recv(&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

void funcion_camarero(){
    int valor, 
        s = 0;
    MPI_Status estado;

    while (true)
    {
        if(s<num_filosofos-1){
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        }
        else
            MPI_Probe(MPI_ANY_SOURCE, ETIQ_LEVANTARSE, MPI_COMM_WORLD, &estado);

        MPI_Recv ( &valor, 1, MPI_INT, estado.MPI_SOURCE , estado.MPI_TAG, MPI_COMM_WORLD,&estado );
        
        switch(estado.MPI_TAG){
            case ETIQ_SENTARSE:
                s++;
                break;
            case ETIQ_LEVANTARSE:
                s--;
                break;
        }
    }
}


// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio == id_camarero)     // si es el último
         funcion_camarero();             //   es el camarero
      else if ( id_propio % 2 == 0 )      // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor 
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
