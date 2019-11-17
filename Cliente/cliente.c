#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "ini/ini.h"

#define SA struct sockaddr

typedef struct direcciones{
    char ip[50];
    char port[20];
}direccionSv;
/*Leo el Archivo de Configuracion y Cargo las direcciones IP/PUERTO de los Server Switch para Conexion Socket*/
direccionSv* cargarDireccionSv(){
  ini_table_s* config = ini_table_create();
  direccionSv* conf = malloc(sizeof(direccionSv));
    if (!ini_table_read_from_file(config, "config/config.ini")) {
        puts("El Archivo de Config no Existe!");
      return 0;
    }else {
        strcpy(&conf->ip,ini_table_get_entry(config, "server", "ip"));
        strcpy(&conf->port,ini_table_get_entry(config, "server", "puerto"));
    }
    ini_table_destroy(config);
    return conf;
}

/*Inicia un Socket como Cliente contra el Sgbd*/
int iniciarSocketCliente(char ip[],int port){
  int sockfd;
  struct sockaddr_in servaddr, cli;

  sockfd = socket(AF_INET, SOCK_STREAM,0);
  if(sockfd == -1){
    printf("Socket Creation Failed..\n");
    exit(0);
  }else{
    //Socket Creado..
    bzero(&servaddr, sizeof(servaddr));
    //IP-Puerto
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    if(connect(sockfd, (SA*)&servaddr,sizeof(servaddr))!=0){
      printf("Connection Failed..\n");
      exit(0);
    }else{
      printf("Connected.. ");
    }
  }
  return sockfd;
}

int main(int argc, char const *argv[]) {
  printf("---Cliente SV-SWITCH Iniciado!---\n");

  while(true){//Bucle Principal
    printf("Formato de las Consultas: 'Sgbd'/'dbName'/'querySQL'\n");
    printf("Ingrese Una Consulta || Ingrese '//' para Salir:\n");
    printf(">");
    char query[1024];
    char respuesta[1024];
    fgets (query, 100, stdin);

    char *p;
    p = strchr(query,'\n');//Busco el salto de linea
    if(p) *p = '\0';//Reemplazo el salto de linea
    if(strcmp(query,"//")==0) exit(0);//salgo del bucle..

    direccionSv* confSv =cargarDireccionSv();
    int sockId = iniciarSocketCliente(confSv->ip,atoi(confSv->port));//Intento conectar con el sv
    if(sockId != -1){
      printf("Conectado con el Server, enviando Consulta...\n");
      write(sockId,query,sizeof(query));
      printf("Respuesta a tu Consulta: \n");
      printf(">");
      read(sockId,respuesta,1024);
      printf("%s\n \n",respuesta);
      close(sockId);
      strcpy(respuesta,"");
      strcpy(query,"");
    }else{
      printf("No fue posible conectar con el Server...\n");
    }
  }

  return 0;
}
