#include <mysql.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void funcionMysql(char server[],char user[], char password[],char databasename[],int idsockc,char query[], char *response){
	MYSQL *conn ; /* variable de conexión para MySQL */
	MYSQL_RES *res; /* variable que contendra el resultado de la consuta */
	MYSQL_ROW row; /* variable que contendra los campos por cada registro consultado */
	int i;
   // MYSQL *mysql_init(MYSQL *conn);
	conn = mysql_init(NULL); /*inicializacion a nula la conexión */

	/* conectar a la base de datos */
	if (!mysql_real_connect(conn, server, user, password, databasename, 0, NULL, 0))
	{ /* definir los parámetros de la conexión antes establecidos */
        printf("Hubo error: ");
		fprintf(stderr, "%s\n", mysql_error(conn)); /* si hay un error definir cual fue dicho error */
		exit(1);
	}else{
		printf("Conectado exitosamente a base de datos: %s\n\n",databasename);
	}

	/* enviar consulta SQL */
	if (mysql_query(conn,query)){ /* definicion de la consulta y el origen de la conexion */
		printf("Realizando consulta: %s\n",query);
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}else{
		printf("Fallo al realizar la consulta: %s\n",query);
	}
	memset(response,0,20000);
	res = mysql_use_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL){
		for (i = 0 ; i < mysql_num_fields(res); i++){
			if(row[i]==NULL){
				strcat(response, "null");
				strcat(response, "||");
			}else{
				strcat(response, row[i]);
				strcat(response, "||");
			}
		}
    	strcat(response, ".\n");
		write(idsockc,response,20000);
   		memset(response,0,20000);
	}
	mysql_free_result(res);
	mysql_close(conn);
}
