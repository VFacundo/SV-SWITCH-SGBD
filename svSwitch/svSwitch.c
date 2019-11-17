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
#define IpSv "127.0.0.1"
int puertoSv;

typedef struct direcciones{
    char ip[50];
    char port[20];
}direccionesSv;
/*Leo el Archivo de Configuracion y Cargo las direcciones IP/PUERTO de los Server para Conexion Socket*/
direccionesSv* cargarDireccionesSvsgbd(char sgbd[]){
  ini_table_s* config = ini_table_create();
  direccionesSv* conf = malloc(sizeof(direccionesSv));
    if (!ini_table_read_from_file(config, "config/config.ini")) {
        puts("El Archivo de Config no Existe!");
      return 0;
    }else {
        strcpy(&conf->ip,ini_table_get_entry(config, sgbd, "ip"));
        strcpy(&conf->port,ini_table_get_entry(config, sgbd, "puerto"));
    }
    ini_table_destroy(config);
    return conf;
}

/*Recibe el query, y obtiene el nombre del Sgbd para identificar a quien enviar la consulta*/
void obtenerQuery(char query[],char sgbd[],char querySend[]){
  int i,k,j;
  for(i=0;query[i]!='/';i++){
      sgbd[i] = tolower(query[i]);
  }
  k=0;
  sgbd[i] = '\0';
  i++;
  for(j=i;j<strlen(query) || query[j]!= '\0';j++){
    querySend[k] = query[j];
    k++;
  }
  querySend[k] = '\0';
}

/*Inicia un Socket como Cliente contra el Sgbd*/
int iniciarSocketCliente(char ip[],int port, int idsockc){
  int sockfd;
  struct sockaddr_in servaddr, cli;

  sockfd = socket(AF_INET, SOCK_STREAM,0);
  if(sockfd == -1){
    printf("Socket Creation Failed..\n");
    write(idsockc,"No se pudo Conectar con el SGBD",1024);
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
      write(idsockc,"No se pudo Conectar con el SGBD",1024);
      exit(0);
    }else{
      printf("Connected..\n");
    }
  }
  return sockfd;
}

int main(int argc, char const *argv[]) {
  puertoSv = atoi(argv[1]);//Puerto en que va a escuchar el SV Switch
  struct sockaddr_in s_sock,c_sock;
  int idsocks,idsockc;
  socklen_t lensock = sizeof(struct sockaddr_in);
  idsocks = socket(AF_INET, SOCK_STREAM, 0);
  printf("---Servidor Switch Iniciado!---\n");
  printf("IDSocks %d\n",idsocks);

  s_sock.sin_family      = AF_INET;
  s_sock.sin_port        = htons(puertoSv);
  s_sock.sin_addr.s_addr = inet_addr(IpSv);
  memset(s_sock.sin_zero,0,8);

  printf("Bind %d\n", bind(idsocks,(struct sockaddr *) &s_sock,lensock));
  printf("Listen %d\n",listen(idsocks,5));

  //Cargo las Config de los SV de SGBD
  printf("Intento Cargar IP/Puerto de Firebird sv .. \n");
  direccionesSv* conffb = cargarDireccionesSvsgbd("firebird");
  printf("La Direccion(ip/port) de %s es: %s : %s \n","firebird",conffb->ip,conffb->port);
  printf("\n");
  printf("Intento Cargar IP/Puerto de Postgresql sv .. \n");
  direccionesSv* confpost = cargarDireccionesSvsgbd("postgresql");
  printf("La Direccion(ip/port) de %s es: %s : %s \n","postgresql",confpost->ip,confpost->port);
  printf("\n");
  printf("Intento Cargar IP/Puerto de mySQL sv .. \n");
  direccionesSv* confmy = cargarDireccionesSvsgbd("mysql");
  printf("La Direccion(ip/port) de %s es: %s : %s \n","mysql",confmy->ip,confmy->port);
  //int p = atoi(&conffb->port);
  printf("\n");

      while(1){
          printf("Esperando conexion....\n");
          idsockc = accept(idsocks,(struct sockaddr *)&c_sock,&lensock);

           if(idsockc != -1){
               if (!fork()){
                printf("Nueva Conexion Aceptada..\n" );
                  //formato firebird/nombrebd/consultasql
                char sgbd[10]="";
                char querySend[1024]="";
                char query[1024]="";
                read(idsockc,query,1024);
                obtenerQuery(query,sgbd,querySend);
                printf("SGBD :%s , QUERY: %s\n", sgbd,querySend );
                int sockCli;
                //Preparo Socket para ser Cliente del sv sgbd

                if(strcmp(sgbd,"firebird")==0){
                  printf("Firebird\n");
                  sockCli = iniciarSocketCliente(conffb->ip,atoi(&conffb->port),idsockc);
                }else if(strcmp(sgbd,"postgresql")==0){
                  printf("Postgresql\n");
                  sockCli = iniciarSocketCliente(confpost->ip,atoi(&confpost->port),idsockc);
                }else if(strcmp(sgbd,"mysql")==0){
                  printf("Mysql\n");
                  sockCli = iniciarSocketCliente(confmy->ip,atoi(&confmy->port),idsockc);
                }else{
                  printf("Opcion Incorrecta!\n");
                }

                if(sockCli!=-1){
                    write(sockCli,querySend,sizeof(querySend));
                    printf("Esperando leer\n" );
                    strcpy(query,"");
                    char respuesta[1024]="";
                    read(sockCli,respuesta,1024);
                    write(idsockc,respuesta,sizeof(respuesta));
                    close(sockCli);
                    close(idsockc);
                }else{
                  printf("No se pudo establecer la Conexion\n");
                }

                printf("Conexion Finalizada con el cliente! Bye\n");
                close(idsockc);
                exit(0);
        }
      }
    }
  return 0;
}
