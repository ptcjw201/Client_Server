#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void error(char *msg){
    perror(msg);
    exit(1);
}

//calculates the lendth of subject in path, using stat, buf
// st_size of struct stat buf contains the length of the subject
int gettinglength(char* path){
  struct stat buf;
  int n = stat(path,&buf);
  int count = (int)(buf.st_size);  
  return count;
}

//header-Filetype
//scans the string and c
char *gettingtype(char *path){
    if(strstr(path,".html")!=NULL){
        return "text/html";
    }
    else if(strstr(path, "jpeg")!=NULL){
        return "image/jpeg";
    }
    else if(strstr(path, "gif")!=NULL){
        return "image/gif";
    }
    else if(strstr(path, "pdf")!=NULL){
        return "application/pdf";
    }
    else if(strstr(path, "mp3")!=NULL){
        return "audio/mpeg";
    }
    else{
        return NULL;
    }
}


int main(int argc, char *argv[]){
     int sockfd, newsockfd, portno;
     int n;
     socklen_t clilen;

     char *token;
     char *path;
     struct sockaddr_in serv_addr, cli_addr;
     /*for getting commands*/
     char buffer[256],buff_copy[256];

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     /*Create a new socket
       AF_INET: Address Domain is Internet 
       SOCK_STREAM: Socket Type is STREAM Socket */
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);//atoi converts from String to Integer
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;//for the server the IP address is always the address that the server is running on
     serv_addr.sin_port = htons(portno);//convert from host to network byte order
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //Bind the socket to the server address
              error("ERROR on binding");
     
     listen(sockfd,5);// Listen for socket connections. Backlog queue (connections to wait) is 5
     
     clilen = sizeof(cli_addr);
     /*accept function: 
       1) Block until a new connection is established
       2) the new socket descriptor will be used for subsequent communication with the newly connected client.
     */
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

    /*fill buffer with 0*/
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);//Read is a block function. It will read at most 255 bytes
     if (n < 0) error("ERROR reading from socket");
        printf("Here is the message:\n%s",buffer);


     //copy buffer for parcing
     strcpy(buff_copy,buffer);
     /*the part you need is the path*/
     token = strtok(buff_copy, " ");
     if(token!=NULL){
         token = strtok(NULL," ");
     }
     path = token;

     int sender; /*used in fread, send, equals to the len of data you got*/

     /*part that makes the header
     includes necessary informations such as
     length, type of file*/
     /*to open your file, html, jpeg...*/
     FILE *fp;
     //allocate memory
     //we are going to make a header called "head"
     char *head = malloc(sizeof(char)*256);
     strcpy(head,"HTTP/1.1 200\n");
     strcat(head,"Content-Length: ");
     //by using a char array and sprintf, you can get a string version of int
     char numtostring[256];
     sprintf(numtostring,"%d",gettinglength(path+1));
     strcat(head,numtostring);  
     strcat(head,"\nContent-Type: ");
     char *type = gettingtype(path);
     strcat(head,type);  
     strcat(head,"\n\n");   

     /*print left buffers by reading again*/
     n = read(newsockfd,buffer,255);
     printf("%s\n",buffer);

     /*empty buffer to use memory*/
     bzero(buffer,256);

     /*path is /xxxx.xxx so, use +1 to pass parameter without / */
     fp=fopen(path+1,"rb");
     printf("%s\n",head);
     strcpy(buffer,head);
     sender=fread(buffer+strlen(head),1,256-strlen(head),fp);//combine header and data
     if (sender < 0) {
      error("ERROR read from file");
     }
     sender=send(newsockfd,buffer,strlen(head)+sender,0);
     if (sender < 0) {
      error("ERROR writing to socket");
     }
     bzero(buffer,256);
 
     //return all leftovers that are in the buffer*/
     int t;
     while (1){
      t=fread(buffer,1,256,fp);
      sender=send(newsockfd,buffer,256,0);
      bzero(buffer,256);
      if(t!=256){
        break;
      }
    }
    //done using the fp, so close it*/
    fclose(fp);
    close(sockfd);
    close(newsockfd);
    return 0; 
}
