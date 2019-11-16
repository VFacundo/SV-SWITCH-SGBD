/**
 * @file libfb.h
 * Archivo de cabecera de libreria libfb que facilita el uso de la api
 * del servidor de base de datos Firebird
 *
 * Copyright 2012 grchere 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * @author Guillermo Cherencio
**/
/**
  *@mainpage 
  * <p>Bienvenidos a la documentacion de libfb.
  * El objetivo de esta libreria es hacer su programacion cliente-servidor con
  * Firebird Database mas sencilla.
  * Esta libreria es dependiente del cliente Firebird fbclient, firebirdX.X.dev (requiere de ibase.h).</p>
  * <p>La siguiente guia (para Linux Debian) le permitira armar un ambiente de trabajo posible para esta libreria.</p>
  *<ul> 
  * <li>1. Instalar/Verificar que cuenta con el software basico para trabajar<br>
  *       1. Tiene que tener instalado el paquete gcc, gcc-doc, build-essential, make<br>
  * @code
  * $ aptitude install gcc
  * $ aptitude install gcc-doc
  * $ aptitude install build-essential
  * $ aptitude install make
  * @endcode
  * </li>
  * <li>2. Instalar Firebird y FlameRobin<br>
  *  Seguir la guia <a href="http://www.grch.com.ar/docs/bd/tutorial/firebird/instalacion.firebird.2.5.pdf">Instalar Firebird 2.5</a><br>
  * NOTA: Si Ud. va a utilizar un servidor Firebird fuera de su maquina, simplemente puede instalar la libreria fbclient<br>
  * para poder conectarse a un servidor Firebird (esta libreria se instala automaticamente cuando instala el servidor)<br>
  * tambien asegurese de tener instalado el paquete firebirdX.X-dev (reemplace las X por la version firebird,<br>
  * este paquete instala ibase.h necesario para compilar libfb)<br>
  * @code
  * $ aptitude install fbclient
  * $ aptitude install firebirdX.X-dev
  * @endcode
  * </li>
  * <li>3. Utilice FlameRobin o isql-fb para crear una base de datos de prueba, luego cree algunos objetos dentro de la misma
  * para ser utilizados a traves de programas C que utilicen esta libreria y permita la conexion con esta base de datos
  * </li>
  * </li>
  * <li>4. Cree uno o mas programas C que utilicen esta libreria y permitan la conexion y utilizacion de una base de datos,
  * compile sus programas indicando esta libreria y la libreria cliente firebird:<br>
  * @code
  * $ gcc -Wall -o miprog miprog.c -lfb -lfbclient
  * @endcode
  * este simple ejemplo, asume que libfb.a y libfbcient.a se encuentran en el directorio actual o bien dentro de los directorios
  * de busqueda de su compilador ANSI C (sino debera utilizar los switches: -I -L -i -l).
  * </li>
  *</ul> 
**/
#include <stdio.h>
#include <stdlib.h>
#include <ibase.h>
#include <iberror.h>

// para evitar una doble carga de este header en las distintas compilaciones del sistema
#ifndef MODLIBFB1234
   #define MODLIBFB1234


#ifndef ISC_INT64_FORMAT
   /** Define a format string for printf.  Printing of 64-bit integers
      is not standard between platforms */
   #if (defined(_MSC_VER) && defined(WIN32)) || (defined(__BORLANDC__) && defined(__WIN32__))
      #define	ISC_INT64_FORMAT	"I64"
   #else
      #define	ISC_INT64_FORMAT	"ll"
   #endif
#endif

/**  
 * @brief Estructura que representa datos sql de longitud variable
 * 
 * @see get_data
 */
typedef struct vary {
    short          vary_length;
    char           vary_string [1];
} VARY;

/**********************************************************************
fb_do_query() event types
**********************************************************************/
/** Definicion que implica query ejecutado Ok
 * @see fb_do_query()
 */
#define FB_EXECUTE_QUERY_OK          1
/** Definicion que implica query ejecutado con error
 * @see fb_do_query()
 */
#define FB_EXECUTE_QUERY_ERROR    2
/** Definicion que implica instancia en la cual la funcion de callback utilizada
 * en fb_do_query debe indicar los valores de los parametros de entrada del query,
 * esta definicion se pasa como primer argumento de la funcion de callback que utilice
 * el usuario
 * @see fb_do_query()
 */
#define FB_SET_QUERY_INPUT             3  // set input parameter
/** Definicion que implica instancia en la cual la funcion de callback utilizada
 * en fb_do_query debe indicar los valores de los parametros de salida del query,
 * esta definicion se pasa como primer argumento de la funcion de callback que utilice
 * el usuario
 * @see fb_do_query()
 */
#define FB_SET_QUERY_OUTPUT          4  // set input parameter
/** Definicion que implica instancia en la cual la funcion de callback utilizada
 * en fb_do_query toma conocimiento de que el query en cuestion dio error al inicio
 * del query y el mismo no puede avanzar en las etapas subsiguientes del proceso del query.
 * Esta definicion se pasa como primer argumento de la funcion de callback que utilice
 * el usuario
 * @see fb_do_query()
 */
#define FB_START_QUERY_ERROR         5  // error trying start query, we can't continue
/** Definicion que implica instancia en la cual la funcion de callback utilizada
 * en fb_do_query toma conocimiento de que el query en cuestion dio error al intentar asignar
 * memoria en forma dinamica dentro de fb_do_query()  y el mismo no puede avanzar en las etapas subsiguientes del proceso del query.
 * Esta definicion se pasa como primer argumento de la funcion de callback que utilice
 * el usuario
 * @see fb_do_query()
 */
#define FB_MEMORY_QUERY_ERROR     6  // error allocating memory for query, we can't continue
/** Definicion que implica instancia en la cual la funcion de callback utilizada
 * en fb_do_query toma conocimiento de que el query en cuestion ha avanzado en su 
 * ejecucion al punto tal en que esta listo para recuperar filas del query.
 * Esta definicion se pasa como primer argumento de la funcion de callback que utilice
 * el usuario
 * @see fb_do_query()
 */
#define FB_FETCH_RECORDS                7  // cursor ready to fetch records
/**********************************************************************
call back function fb_do_query() return types
**********************************************************************/
/** Definicion que indica un valor de retorno de la funcion de callback del usuario
 * utilizada en la funcion fb_do_query que le indica a fb_do_query() que continue
 * con la normal ejecucion del query en cuestion.
 * Esta definicion es un valor de retorno de la funcion de callback del usuario
 * @see fb_do_query()
 */
#define FB_CONTINUE                1  // commit, execute a parametrized query
/** Definicion que indica un valor de retorno de la funcion de callback del usuario
 * utilizada en la funcion fb_do_query que le indica a fb_do_query() que cancele, aborte
 * la normal ejecucion de este query en cuestion, pero continuar con el resto de los queries
 * pendientes que pudiera haber.
 * Esta definicion es un valor de retorno de la funcion de callback del usuario
 * @see fb_do_query()
 */
#define FB_ABORT                   2  // rollback this query, but continue trying other pendings queries
/** Definicion que indica un valor de retorno de la funcion de callback del usuario
 * utilizada en la funcion fb_do_query que le indica a fb_do_query() que cancele, aborte
 * la normal ejecucion de todos los queries en cuestion.
 * Esta definicion es un valor de retorno de la funcion de callback del usuario
 * @see fb_do_query()
 */
#define FB_ABORT_ALL               3  // rollback this query and cancel all pendings queries

/**  
 * @brief Estructura que representa una conexion a una base de datos firebird determinada
 * 
 * Esta estructura representa una conexion a una base de datos firebird determinada. Almacena
 * los datos de la conexion: usuario, contrasenia, nombre/ubicacion fisica de la base de datos,
 * rol del usuario, etc.
 * Esta estructura se asocia con otras para el seguimiento de queries y otras actividades de esta
 * libreria de forma tal que estos datos esten siempre disponibles.
 * Una vez que se realiza la conexion esta estructura actualiza sus datos. Los datos requeridos 
 * que deben estar cargados en esta estructura previo a la conexion de una base de datos son: 
 * dbname, user, passw, role
 *  
 * @warning observe los limites en cuanto al nombre/ubicacion de la base de datos (512 bytes),
 * lo mismo en cuanto al nombre de usuario firebird, contrasenia, rol, etc.
 * @see fb_do_connect()
 */
typedef struct {
	/** handle de la conexion de la base de datos */
   isc_db_handle db1;
   /** vector de status de conexion que provee firebird */
   ISC_STATUS status_vector[32];
   /** nombre de la base de datos */
   char dbname[512];
   /** usuario (sysdba) utilizado para conectarme a la base de datos */
   char user[32];
   /** contrasenia (masterkey) utilizada para conectarme a la base de datos */
   char passw[32];
   /** rol del usuario (sysdb) utilizado para conectarme a la base de datos */
   char role[32];
   /** version del servidor firebird */
   char fb_version[50];
   /** tamanio de la pagina de la base de datos */
   int fb_page_size;
   /** numero de buffers de la base de datos */
   int fb_num_buffers;
   /** true si la base de datos es read-only (ej una b.d. en un cd-rom) */
   int fb_read_only;
   /** dialecto firebird utilizado */
   int fb_sql_dialect;
} fb_db_info;

/**  
 * @brief Estructura que representa un query y su informacion asociada
 * 
 * Esta estructura representa un query y su informacion asociada. Almacena los datos relacionados
 * de un query del usuario. Muchos de estos datos seran cargados una vez que se realice la ejecucion
 * del query en cuestion. Por ejemplo, esta estructura es pasada como argumento de la funcion de
 * callback del usuario utilizada junto con la funcion fb_do_query()
 *  
 * @see fb_do_cmd()
 * @see fb_analyze_query()
 * @see fb_do_query()
 */
typedef struct {
	/** puntero a conexion con la base de datos */
   fb_db_info *dbinfo;
   /** puntero a manipulador (handle) de la transaccion asociada con este query */
   isc_tr_handle *tr1;
   /** puntero a manipulador (handle) del SQL statement asociada con este query */
   isc_stmt_handle *stmt; 
   XSQLDA *in_sqlda;
   XSQLDA *out_sqlda;
   /** SQL query a ejecutar */
   char *sql;
   /** nombre del cursor  */
   char *cursor; 
   /** Id del query desde el punto de vista del usuario, permite identificarlo en caso de usar
    * la misma funcion de callback para varios queries  */
   int queryId;
   int method;
   /** flag de error true->error, false->no error */
   int fb_error; 
   /** si hay error, en este miembro queda el codigo sql de error */ 
   long SQLCODE; 
   /** ultimo status del ultimo fetch realizado */
   ISC_STATUS FETCHCODE; 
   /** buffer con mensajes de error */
   char *errmsg;
   /** cuenta cuantas veces se lanzo el evento FB_SET_QUERY_INPUT, comenzando por 0 */
   int set_time;
   /** cuantas veces debe ser ejecutado este query parametrizado? */
   int set_maxtimes; 
   /** cuantas filas fueron actualizadas */
   int rows_updated;
   /** cuantas filas fueron insertadas */
   int rows_inserted;
   /** cuantas filas fueron borradas */
   int rows_deleted;
   /** cuantas filas fueron recuperadas,seleccionadas */
   int rows_selected;
   /** cuantas filas fueron obtenidas (fetched) en cada ejecucion del query */
   int rows_fetched; 
} fb_query_info;

/**  
 * @brief Estructura que representa un atributo de tipo blob
 * 
 * Esta estructura representa un atributo de tipo blob, indica su tamano en bytes,
 * numero de segmentos asignados, cantidad maxima de segmentos
 *  
 * @see fb_import_fblbpar()
 * @see fb_export_fblbpar()
 * @see fb_export_bblbpar
 * @see fb_export_sblbpar
 * @see fb_info_blbpar
 * 
 */
typedef struct {
	/** tamano en bytes del blob */
   long size;
   /** numero de segmentos asignados */
   long num_segments;
   /** numero maximo de segmentos asignados */
   long max_segment;
   /** tipo de blob */
   int type;
} fb_blob_info;

/**  
 * @brief Estructura para el recupero de tuplas en queries genericos
 * 
 * Esta estructura representa una tupla recuperada de un query que esta almacenada en
 * memoria bajo la forma de una lista doblemente enlazada
 *  
 * @see onDoGenericQuery()
 * @see fb_free()
 * @see fb_fprintf
 * @see fb_get_rowcol
 * @see fb_get_col
 * @see fb_get_col_len
 * @see fb_row_len
 */
typedef struct rquery {
	/** buffer que contiene arreglo de apuntadores a char * de longitud 
	 * query.cols con el contenido de las columnas */
   void *col;                   
   /** puntero a siguiente tupla */
   struct rquery *next;
   /** puntero a tupla anterior */
   struct rquery *prev;
} rquery;

/**  
 * @brief Estructura que representa un query generico que sera cargado en 
 * memoria utilizando lista de tipo rquery asociada al mismo
 * 
 * Esta estructura representa un query generico que sera cargado en 
 * memoria utilizando lista de tipo rquery asociada al mismo
 *  
 * @see onDoGenericQuery()
 * @see fb_free()
 * @see fb_fprintf
 */
typedef struct query {
	/** cantidad de filas */
   int rows;
	/** cantidad de columnas */
   int cols;
   /** buffer que contiene arreglo de apuntadores a char * de longitud 
    * query.cols con los nombres de las columnas */
   void *colname; 
   /** flag de error true->error, false->no error */
   int fb_error; 
   /** si hay error, en este miembro queda el codigo sql de error */ 
   long SQLCODE; 
   /** ultimo status del ultimo fetch realizado */
   ISC_STATUS FETCHCODE; 
   /** buffer con mensajes de error */
   char *errmsg; 
   /** cuantas filas fueron actualizadas */
   int rows_updated;
   /** cuantas filas fueron insertadas */
   int rows_inserted;
   /** cuantas filas fueron borradas */
   int rows_deleted;
   /** cuantas filas fueron recuperadas,seleccionadas */
   int rows_selected;
   /** cuantas filas fueron obtenidas (fetched) en cada ejecucion del query */
   int rows_fetched; 
   /** puntero a la primer tupla en memoria */
   struct rquery *top;
   /** puntero a la ultima tupla en memoria */
   struct rquery *bottom;
} query;
// fin estructuras para el recupero de tuplas en queries genericos


// funciones de libreria, interfase externa
int fb_analyze_query(ISC_STATUS ISC_FAR *status_vector,isc_stmt_handle *stmt,fb_query_info *info);
int fb_do_connect(fb_db_info *dbinfo);
query *fb_do_connect_squery(char *dbname,char *user,char *passwd,char *role,char *sql_stmt);
void fb_pre_connect(fb_db_info *dbinfo,char *dbname,char *user,char *passwd,char *role);
int fb_do_cmd(fb_db_info *dbinfo,char *sql_stmt);
int fb_do_db(fb_db_info *dbinfo,char *sql_stmt);
int fb_do_disconnect(fb_db_info *dbinfo);
int fb_do_exec_query(fb_db_info *dbinfo,char *sql_stmt,int pinput,int poutput,int (*onDoQuery)(int eventType,fb_query_info *qi,void *buffer2),void *buffer);
int fb_do_query(fb_db_info *dbinfo,int queryId,char *sql_stmt,int (*)(int eventType,fb_query_info *qi,void *buffer),void *buffer);
query *fb_do_single_query(fb_db_info *dbinfo,char *sql_stmt);
int fb_dsql_method(char *sql_stmt);
void fb_dsql_outvars(XSQLDA *out_sqlda);
void fb_dsql_outvars2(XSQLDA *out_sqlda);
void fb_dsql_setvars(XSQLDA *in_sqlda);
int fb_error(fb_db_info *dbinfo,fb_query_info *qi); // the 2nd parameter may be NULL
void fb_export_arraypar(fb_query_info *qi,int  param,void *buffer,int size);
void fb_export_bblbpar(fb_query_info *qi,int  param,void *buffer);
void fb_export_fblbpar(fb_query_info *qi,int  param,char *output_file);
void fb_export_sblbpar(fb_query_info *qi,int  param,void *buffer,long position,long size);
int fb_fetch(fb_query_info *qi);
void fb_free(query *q);
void fb_fprintf(FILE *,query *q);
char *fb_get_rowcol(query *q,int row,int col);
// get query data members
rquery *fb_get_query_top(query *q);
rquery *fb_get_query_bottom(query *q);
int fb_get_query_fberror(query *q);
char *fb_get_query_errmsg(query *q);
int fb_get_query_rows(query *q);
int fb_get_query_cols(query *q);
long fb_get_query_sqlcode(query *q);
ISC_STATUS fb_get_query_fetchcode(query *q);

char *fb_get_col(query *q,rquery *rq,int col);
char *fb_get_col_byname(query *q,rquery *rq,char *colname);
int fb_get_col_len(query *q,rquery *rq,int col);
char *fb_get_col_name(query *q,int col);
int fb_get_col_len_name(query *q);

int fb_get_fb_query_info_fberror(fb_query_info *q);
char *fb_get_fb_query_info_errmsg(fb_query_info *q);
int fb_get_fb_query_info_queryId(fb_query_info *q);
int fb_get_fb_query_info_settime(fb_query_info *q);
char *fb_get_fb_query_info_sql(fb_query_info *q);
long fb_get_fb_query_info_sqlcode(fb_query_info *q);
ISC_STATUS fb_get_fb_query_info_fetchcode(fb_query_info *q);

void fb_set_fb_query_info_fberror(fb_query_info *q,int fberror);
void fb_set_fb_query_info_errmsg(fb_query_info *q,char *errmsg);
void fb_set_fb_query_info_sqlcode(fb_query_info *q,long sqlcode);
void fb_set_fb_query_info_fetchcode(fb_query_info *q,ISC_STATUS fetchcode);

int fb_row_len(query *q,rquery *rq);
char *fb_getcol(fb_query_info *qi,int pos);
void fb_import_arraypar(fb_query_info *qi,int  param,void *buffer,int size);
void fb_import_bblbpar(fb_query_info *qi,int  param,void *buffer,long buffer_size,int segment_size);
void fb_import_fblbpar(fb_query_info *qi,int  param,char *input_file,int segment_size);
void fb_init(query *q) ;
void fb_info_blbpar(fb_query_info *qi,int param,fb_blob_info *info);
void fb_log_message(char *errmsg);
void fb_print_error(ISC_STATUS ISC_FAR *status_vector,const char *msg);
rquery *fb_rquery_next(rquery *r);
rquery *fb_rquery_previous(rquery *r);
void fb_set_strpar(fb_query_info *qi,int param,char *strpar);
void fb_set_intpar(fb_query_info *qi,int param,int intpar);
void fb_set_longpar(fb_query_info *qi,int param,long longpar);
void fb_set_stream_msg(int file_descriptor,char *mode);
void fb_set_query_fberror(query *q,int fberr);
void fb_set_query_errmsg(query *q,char *errmsg);
void fb_set_query_sqlcode(query *q,long sqlcode);
void fb_set_query_fetchcode(query *q,ISC_STATUS fetchcode);

int  fb_sizeof_query();
int  fb_sizeof_rquery();
int  fb_sizeof_fb_blob_info();
int  fb_sizeof_fb_query_info();
int  fb_sizeof_fb_db_info();
void fb_stream_msg_close();
char *fb_get_strpar(fb_query_info *qi,int param);
int  fb_get_intpar(fb_query_info *qi,int param);
long fb_get_longpar(fb_query_info *qi,int param);

char *get_data (XSQLDA *out_sqlda, int pos );
// funcion generica de callback para ser usada en combinacion con fb_do_query()
int onDoGenericQuery(int eventType,fb_query_info *dbinfo,void *buffer);
// fin funciones de libreria, interfase externa


// funciones internas, no obstante, por ahora tambien se declaran como parte de la interfase externa
int fb_load_query(query *q,fb_query_info *qi);
int fb_parse_cmd(char *buffer,int tope,char *cmd,int *value);
int fb_parse_scmd(char *buffer,char *cmd,char *value);
void fb_print_db_info( fb_db_info *dbinfo);
void fb_init_query_info(fb_query_info *qi);
void fb_end_query_info(fb_query_info *qi);
void fb_end_dsql_vars(XSQLDA *sqlvar);
void fb_wait_events(fb_db_info *dbinfo,char *events[],int *flag,int (*)(fb_db_info *dbinfo,char *eventName,int count));
isc_callback fb_event_function(char *result, short length, char *updated);
// fin funciones internas, no obstante, por ahora tambien se declaran como parte de la interfase externa

#endif
