#include <postgresql/libpq-fe.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>

void funcionPostgresql(char IPPostgrsql[], char PuertoPostgresql[],char bd[],char user[], char pwd[],int idsockc, char query[], char *response[])
{
  printf("funcionPostgresql\n");
   PGconn *conn;
   PGresult *res;
   int i,j;
   //char response[20000];
   memset(response,0,20000);
   conn = PQsetdbLogin(IPPostgrsql,PuertoPostgresql,NULL,NULL,bd,user,pwd);
   if (PQstatus(conn) != CONNECTION_BAD){
      res = PQexec(conn, query);

      if(PGRES_COMMAND_OK == PQresultStatus(res)){
          strcat(response,"Query Ejecutado OK!\n");//Successful completion of a command returning no data
          printf("Query Ejecutado OK!\n");
      }else if (res != NULL && PGRES_TUPLES_OK == PQresultStatus(res)){
          for (i = 0 ; i <= PQntuples(res)-1;  i++){
                for (j = 0 ; j < PQnfields(res); j++){
                      strcat(response,PQgetvalue(res,i,j));
                      strcat(response," || ");
                    }
                strcat(response,".\n");
             }
             strcat(response,"\0");
             PQclear(res);
          //strcpy(response, response);
        }else{
            strcat(response,"Query Ejecutado ERROR!\n");
            printf("Query Ejecutado ERROR!\n");
        }
    }else{
      strcat(response,"Fallo la Conexion con Postgresql!\n");
    }
    printf("%s\n",response );
    write(idsockc,response,20000);
    PQfinish(conn);
}
/*
Possible Status of PQexec:

PGRES_EMPTY_QUERY
The string sent to the server was empty.

PGRES_COMMAND_OK
Successful completion of a command returning no data.

PGRES_TUPLES_OK
Successful completion of a command returning data (such as a SELECT or SHOW).

PGRES_COPY_OUT
Copy Out (from server) data transfer started.

PGRES_COPY_IN
Copy In (to server) data transfer started.

PGRES_BAD_RESPONSE
The server's response was not understood.

PGRES_NONFATAL_ERROR
A nonfatal error (a notice or warning) occurred.

PGRES_FATAL_ERROR
A fatal error occurred.

PGRES_COPY_BOTH
Copy In/Out (to and from server) data transfer started. This is currently used only for streaming replication.
*/
