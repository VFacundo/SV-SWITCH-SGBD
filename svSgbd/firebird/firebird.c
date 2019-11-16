//gcc firebird.c -Wall -g -c -I /usr/include/firebird/ -lfb

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include "libfb/libfb.h"

void funcionFirebird(char user[], char pwd[], char db[], char rol[], int idsockc,char consulta[],char *response)
{
	int i,j;
    memset(response,0,20000);
    extern int FB_SHOW_MESSAGES;         // 1 activa/ 0 desactiva los mensajes de libreria FB
	FB_SHOW_MESSAGES = 1;            // desactivo mensajes de libreria libfb
	fb_db_info dbinfo;
	strcpy(dbinfo.user,user);
	strcpy(dbinfo.passw,pwd);
	strcpy(dbinfo.dbname,db);
	strcpy(dbinfo.role,rol);
	if (fb_do_connect(&dbinfo)) {
		query myquery;
		fb_init(&myquery);
		if ( fb_do_query(&dbinfo,1,consulta,onDoGenericQuery,&myquery) ) {
			strcat(response,"Query Ejecutado OK!\n");
			rquery *q = myquery.top; // primer tupla del query
			char **cols;
			while (q) {
				cols = (char **) q->col;  // q->col arreglo de apuntadores a char *
				for(i=0; i < myquery.rows_fetched; i++){
					for(j=0; j < myquery.cols; j++){
						strcat(response, fb_get_col(&myquery, q, j));
						strcat(response," || ");
					}
					strcat(response,".\n");
					//write(idsockc,response,20000); //Escribir en el socket!!
					//memset(response,0,20000);
					q = q->next;  // siguiente tupla del query
				}
			}
			write(idsockc,response,20000); //Escribir en el socket!!
			memset(response,0,20000);
			fb_free(&myquery);
		} else {
			printf("Error en ejecucion\n\
			    Error FB [%d] mensaje [%s] sql code [%ld]\n",myquery.fb_error,myquery.errmsg,myquery.SQLCODE);
		}
		fb_do_disconnect(&dbinfo);
	}
}
