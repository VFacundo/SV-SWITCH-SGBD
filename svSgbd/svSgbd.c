#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "ini/ini.h"

//#define IpSv "127.0.0.1"
#define IpSv "0.0.0.0"

int puertoSv;
char sgbd[20];
char userBD[20];
char passUserBD[20];
char pathDefaultBD[50];
char rolBd[20];
char puertoBd[20];
char ipBd[20];

/*Carga el Archivo de Configuracion, y guarda los Datos de la BD en Variables locales*/
int setConfigBd(){
  char pathConfig[1024];
  ini_table_s* config = ini_table_create();
    if (!ini_table_read_from_file(config, "config/config.ini")) {
        puts("El Archivo de Config no Existe!");
      return 0;
    }else {//Cargo las config para los diferentes sgdb
        strcpy(userBD,ini_table_get_entry(config, sgbd, "user"));
        strcpy(passUserBD,ini_table_get_entry(config, sgbd, "pass"));

        if(ini_table_check_entry(config, sgbd, "path")!=NULL){
            strcpy(pathDefaultBD,ini_table_get_entry(config, sgbd, "path"));
        }

        if(ini_table_check_entry(config, sgbd, "rol")!=NULL){
            strcpy(rolBd,ini_table_get_entry(config, sgbd, "rol"));
        }

        if(ini_table_check_entry(config, sgbd, "ip")!=NULL){
            strcpy(ipBd,ini_table_get_entry(config, sgbd, "ip"));
        }

        if(ini_table_check_entry(config, sgbd, "puerto")!=NULL){
            strcpy(puertoBd,ini_table_get_entry(config, sgbd, "puerto"));
        }
    }
    ini_table_destroy(config);
    return 1;
}

/*Recibe el Query, obtiene de el el NOmbre de la Base de Datos y el Query en Sql a ejecutar*/
void obtenerNombreBd(char query[],char databasename[],char querySql[]){
  int i;
  int j;
  int k;
  for(i=0;query[i]!='/';i++){
      databasename[i] = query[i];
  }
  databasename[i] = '\0';
  k=0;
  i++;
    for(j=i;j<strlen(query) && query[j]!= '\0';j++){
      querySql[k] = query[j];
      k++;
    }
    querySql[k] = '\0';
    printf(querySql);
}

int main(int argc, char *argv[]) {

  struct sockaddr_in s_sock,c_sock;
  int idsocks,idsockc;
  socklen_t lensock = sizeof(struct sockaddr_in);
  idsocks = socket(AF_INET, SOCK_STREAM, 0);
  printf("idsocks %d\n",idsocks);
  puertoSv = atoi(argv[2]);//Puerto en que va a escuchar el SV Switch
  s_sock.sin_family      = AF_INET;
  s_sock.sin_port        = htons(puertoSv);
  s_sock.sin_addr.s_addr = inet_addr(IpSv);
  memset(s_sock.sin_zero,0,8);

  printf("bind %d\n", bind(idsocks,(struct sockaddr *) &s_sock,lensock));
  printf("listen %d\n",listen(idsocks,5));
  strcpy(sgbd,argv[1]);
  printf("Intento Cargar la Configuracion de %s .. \n",sgbd);
  if(setConfigBd() == 1){
      printf("La Configuracion es: \n");

      printf("%s , %s ,%s , %s, %s, %s \n",userBD,passUserBD,pathDefaultBD,rolBd,ipBd,puertoBd);
      while(1){
          char query[1024];
          printf("Esperando conexion\n");
          idsockc = accept(idsocks,(struct sockaddr *)&c_sock,&lensock);

           if(idsockc != -1){
               if (!fork()){
                 printf("Nueva Conexion Aceptada..\n" );
                  //formato firebird/nombrebd/consultasql
                  char response[20000];
                  char databasename[1024];
                  char querySql[1024];
                  read(idsockc,query,1024);
                  obtenerNombreBd(query,databasename,querySql);
                  printf("Nombre BD: %s ; SQL: %s\n",databasename,querySql );

                  if(strcmp(sgbd,"firebird")==0){
                    char rutaBd[1024];
                    strcpy(rutaBd,pathDefaultBD);
                    strcat(rutaBd,databasename);
                    strcat(rutaBd,".fdb");
                    funcionFirebird(userBD, passUserBD, rutaBd, rolBd, idsockc,querySql,response);
                  }

                  if(strcmp(sgbd,"postgresql")==0){
                    printf("Consulta Postgresql\n");
                    funcionPostgresql(ipBd,puertoBd,databasename,userBD,passUserBD,idsockc,querySql,response);
                  }

                  if(strcmp(sgbd,"mysql")==0){
                    printf("Consulta MySQL\n");
                    //funcionMysql(ipBd,userBD, passUserBD,databasename,idsockc,querySql,response);
                  }

                  strcpy(querySql,"");
                  printf("Conexion Finalizada con el cliente! Bye\n");
                  close(idsockc);
                  exit(0);
                }
              }
         }
      printf("No se pudo Cargar la Configuracion!\n");
    }
  return 0;
}
