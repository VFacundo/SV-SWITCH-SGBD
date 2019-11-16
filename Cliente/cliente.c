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
    printf("Socket Cliente Creado..\n");
    bzero(&servaddr, sizeof(servaddr));
    //IP-Puerto
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    if(connect(sockfd, (SA*)&servaddr,sizeof(servaddr))!=0){
      printf("Connection Failed..\n");
      exit(0);
    }else{
      printf("Connected..\n");
    }
  }
  return sockfd;
}

int main(int argc, char const *argv[]) {
  printf("---Cliente SV-SWITCH Iniciado!---\n");
  printf("Formato de las Consultas: 'Sgbd'/'dbName'/'querySQL'\n");
  printf("Ingrese Una Consulta:\n");
  printf(">");
  char query[1024];
  //scanf("%s", query);
  fgets (query, 100, stdin);
  printf("Tu Consulta es: %s\n",query );

  direccionSv* confSv =cargarDireccionSv();
  printf("La direccion del Sv es %s : %s\n",confSv->ip,confSv->port);
  return 0;
}
