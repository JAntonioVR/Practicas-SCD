// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2-mu.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario (Versión LIFO)
// (versión con varios productores y varios consumidores)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

#define ETIQ_PRODUCTOR  0
#define ETIQ_CONSUMIDOR 1

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
    np                    =   4, // Numero de productores
    nc                    =   5, // Numero de consumidores
    id_buffer             =  np,
    num_items             =  20,
    tam_vector            =  10,
    prod                  = num_items/np, // Numero de items producidos por cada productor
    cons                  = num_items/nc, // Numero de items consumidos por cada consumidor
    num_procesos_esperado = np+nc+1;


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
// pr2oducir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir(int num_productor)
{
    static int contador = num_productor * prod ;
    sleep_for( milliseconds( aleatorio<10,100>()) );
    contador++ ;
    cout << "Productor " << num_productor << " ha producido valor " << contador << endl << flush;
    return contador;
}
// ---------------------------------------------------------------------

void funcion_productor(int num_productor)
{
    for ( unsigned int i = 0; i < prod ; i++ )  // prod items producidos
    {
        // producir valor
        int valor_prod = producir(num_productor);
        // enviar valor
        cout << "Productor " << num_productor << " va a enviar valor " << valor_prod << endl << flush;
        MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, ETIQ_PRODUCTOR, MPI_COMM_WORLD );
    }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons, int num_consumidor )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor " << num_consumidor << " ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor(int num_consumidor)
{
    int         peticion,
                valor_rec = 1 ;
    MPI_Status  estado ;

    for( unsigned int i = 0 ; i < cons; i++ )  // cons items consumidos
    {
        MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, ETIQ_CONSUMIDOR, MPI_COMM_WORLD);
        MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, ETIQ_CONSUMIDOR, MPI_COMM_WORLD,&estado );
        cout << "Consumidor " << num_consumidor << " ha recibido valor " << valor_rec << endl << flush ;
        consumir( valor_rec, num_consumidor );
    }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
    int         buffer[tam_vector],      // buffer con celdas ocupadas y vacías
                valor,                   // valor recibido o enviado
                primera_libre       = 0, // índice de primera celda libre
                num_celdas_ocupadas = 0, // número de celdas ocupadas
                etiq_emisor_aceptabe   ; // etiqueta de emisor aceptable
    MPI_Status  estado ;                 // metadatos del mensaje recibido

    for( unsigned int i=0 ; i < num_items*2 ; i++ )
    {
        // 1. determinar si puede enviar solo prod., solo cons, o todos

        if ( num_celdas_ocupadas == 0 )               // si buffer vacío
            etiq_emisor_aceptabe = ETIQ_PRODUCTOR ;   // $~~~$ solo prod.
        else if ( num_celdas_ocupadas == tam_vector ) // si buffer lleno
            etiq_emisor_aceptabe = ETIQ_CONSUMIDOR ;  // $~~~$ solo cons.
        else                                          // si no vacío ni lleno
            etiq_emisor_aceptabe = MPI_ANY_TAG ;      // $~~~$ cualquiera

        // 2. recibir un mensaje del emisor o emisores aceptables

        MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_emisor_aceptabe, MPI_COMM_WORLD, &estado );

        // 3. procesar el mensaje recibido

        switch( estado.MPI_TAG ) // leer emisor del mensaje en metadatos
        {
            case ETIQ_PRODUCTOR: // si ha sido el productor: insertar en buffer
                buffer[primera_libre] = valor ;
                primera_libre++ ;
                num_celdas_ocupadas++ ;
                cout << "Buffer ha recibido valor " << valor << endl ;
                break;

            case ETIQ_CONSUMIDOR: // si ha sido el consumidor: extraer y enviarle
                primera_libre-- ;
                valor = buffer[primera_libre] ;
                num_celdas_ocupadas-- ;
                cout << "Buffer va a enviar valor " << valor << endl ;
                MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, ETIQ_CONSUMIDOR, MPI_COMM_WORLD);
                break;
        }
    }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    int id_propio, num_procesos_actual;

    // inicializar MPI, leer identif. de proceso y número de procesos
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

    if(num_procesos_actual==num_procesos_esperado){
        // ejecutar la operación apropiada a 'id_propio'
        if ( id_propio < np )
            funcion_productor(id_propio);
        else if ( id_propio == np )
            funcion_buffer();
        else
            funcion_consumidor(id_propio-np-1);
    }
    else{
        if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
        { 
            cout    << "el número de procesos esperados es:    " << num_procesos_esperado << endl
                    << "el número de procesos en ejecución es: " << num_procesos_actual << endl
                    << "(programa abortado)" << endl ;
        }
    }
    // al terminar el proceso, finalizar MPI
    MPI_Finalize( );
    return 0;
}
