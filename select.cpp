/*
USAGE : ./select XXXX
note: XXXX is the username from which you want to run. username belongs to {Satyam,Kunal,Abhishek,Gautam,Hemanth}
*/
/**
    Handle multiple socket connections with select and fd_set on Linux
*/
#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

using namespace std;

#define TRUE   1
#define FALSE  0

typedef struct{
    string ip;
    int port;
    int sock;
} info;


int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    int baderror;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K
    char buffer2[1025];

    char* fname,*fport,*fmsg;
    int fpno;
    struct hostent* fserver;
    struct sockaddr_in faddr;

    struct timeval tv;

    map<string,info> user_info;
    info a ={.ip = "127.0.0.1", .port = 7777, .sock=0};
    user_info["Satyam"]=a;
    info b ={.ip = "127.0.0.1", .port = 8888, .sock=0};
    user_info["Kunal"]=b;
    info c ={.ip = "127.0.0.1", .port = 9999, .sock=0};
    user_info["Abhishek"]=c;
    info d ={.ip = "127.0.0.1", .port = 1111, .sock=0};
    user_info["Gautam"]=d;
    info e ={.ip = "127.0.0.1", .port = 2222, .sock=0};
    user_info["Hemanth"]=e;

    if(user_info.find(argv[1])==user_info.end())
    {
        cout<<"This user is , "<<argv[1]<<", is not in list"<<endl;
        exit(0);
    }
    int PORT = user_info[argv[1]].port;

    //set of socket descriptors
    fd_set readfds;

    //a message
    string message="Welcome, you connected to ";
    message.append(argv[1]);
    message.append("\n");


    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");


    int runagain;
    while(TRUE)
    {
        runagain=0;
        tv.tv_sec=60;
        tv.tv_usec=0;
        cout<<"-----"<<endl;
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(0, &readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

        if(activity==0)
        {
            cout<<"Timeout Occurred"<<endl<<endl;
            for(i=0;i<max_clients;i++)client_socket[i]=0;
            for(map<string,info>::iterator it = user_info.begin();it!=user_info.end();it++)
            {
                if(it->second.sock)
                {
                    close(it->second.sock);
                    it->second.sock=0;
                }
            }
            continue;
        }

        if ((activity < 0) && (errno!=EINTR))
        {
            perror("select error");
            exit(1);
        }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            //printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            //send new connection greeting message
            if( send(new_socket, message.c_str(), message.length(), 0) != message.length() )
            {
                perror("Greeting message send error");
            }


            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    //printf("Adding to list of sockets as %d\n" , i);

                    break;
                }
            }



            if (valread = read( new_socket , buffer, 1024))
            {
                //set the string terminating NULL byte on the end of the data read
                getpeername(new_socket , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                buffer[valread] = '\0';

                for(i=0;buffer[i]!=' ';i++);
                buffer[i]='\0';
                fname=&buffer[0];
                user_info[fname].sock=new_socket;
                cout<<"Connected to new friend : "<<fname<<endl;

                buffer[i]=' ';
                cout<<buffer;
                //send(sd , buffer , strlen(buffer) , 0 );
            }



        }
        if (FD_ISSET(0, &readfds))
        {
            //cout<<"Reading.."<<endl;
            // standard input
            bzero(buffer,1025);
            if(read(0, buffer, 1024)<=0)
            {
                perror("stdin");
                continue;
            }
            //fname=strtok(buffer,"/");cout<<fname<<endl;
            for(i=0;buffer[i]!='/';i++);
            buffer[i]='\0';
            fname=&buffer[0];
            fmsg=&buffer[i+1];

            if(strcmp(fname,argv[1])==0)
            {
                cout<<"Can't send message to yourself"<<endl;
                continue;
            }
            if(user_info.find(fname)==user_info.end())
            {
                cout<<"Can't find this user in your friendlist"<<endl;
                continue;
            }
            fpno=user_info[fname].port;
            baderror=0;
            while(1)
            {   /* connect: create a connection with the server */
               if(!(new_socket=user_info[fname].sock) || baderror)
               {
                   /* gethostbyname: get the server's DNS entry */
                      fserver = gethostbyname((user_info[fname].ip).c_str());
                      if (fserver == NULL) {
                          fprintf(stderr,"ERROR, no such host as %s\n", (user_info[fname].ip).c_str());
                          exit(0);
                      }

                      /* build the server's Internet address */
                      bzero((char *) &faddr, sizeof(faddr));
                      faddr.sin_family = AF_INET;
                      bcopy((char *)fserver->h_addr,
                        (char *)&faddr.sin_addr.s_addr, fserver->h_length);
                      faddr.sin_port = htons(fpno);

                   if ((new_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
                    {
                        perror("socket failed");
                        //exit(EXIT_FAILURE);
                        runagain=1;
                        break;
                    }
                   if (connect(new_socket, (struct sockaddr*)&faddr, sizeof(faddr)) < 0)
                     {
                         perror("ERROR connecting");
                         runagain=1;
                         break;
                     }

                     for (i = 0; i < max_clients; i++)
                     {
                         //if position is empty
                         if( client_socket[i] == 0 )
                         {
                             client_socket[i] = new_socket;
                             //printf("Adding to list of sockets as %d\n" , i);

                             break;
                         }
                     }
                     user_info[fname].sock=new_socket;
                }
                 bzero(buffer2,1025);
                 strcpy(buffer2,argv[1]);
                 strcat(buffer2," : ");
                 strcat(buffer2,fmsg);
                 if( send(new_socket, buffer2, strlen(buffer2), 0)<=0)
                 {
                     if(errno==EBADF)
                     {
                         baderror=1;
                     }
                     else
                     {
                         perror("send");
                         runagain=1;
                         break;
                     }
                 }
                 else break;
                 if(runagain)break;
            }
            if(runagain)continue;


        }

        //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (sd>0 && FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    buffer[valread] = '\0';
                    cout<<buffer;
                    //send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }

    }

    return 0;
}
