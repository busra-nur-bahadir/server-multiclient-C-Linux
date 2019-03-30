#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"

#define SIZE sizeof(struct sockaddr_in)
#define MAX 10
#define Room_size 5

int client[MAX];
int ActiveClients = 0;

//Max Room size is 5
//and max client size in a Room is 2 # max client size is 10
// -1 means there is no client
int Room[5][2] = {{-1, -1},
                  {-1, -1},
                  {-1, -1},
                  {-1, -1},
                  {-1, -1}};

char nick[MAX][128];

int sin_size = sizeof(struct sockaddr_in);

void findMax(int *maxfd)
{
    int i;
    *maxfd = client[0];
    for (i = 1; i < MAX; i++)
        if (client[i] > *maxfd)
            *maxfd = client[i];
}

int main()
{
    int rm; //room pointer to insert
    int sockfd;
    int maxfd;
    int nread;
    int found;
    int i, j, k;

    char buf[1024];
    char x[10];
    char x1[1024];

    fd_set fds;
    struct sockaddr_in server = {AF_INET, 5000, INADDR_ANY};
    struct sockaddr_in their_addr[MAX];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error creating SOCKET\n");
        return (0);
    }
    if (bind(sockfd, (struct sockaddr *)&server, SIZE) == -1)
    {
        printf("bind failed\n");
        return (0);
    }
    if (listen(sockfd, 5) == -1)
    {
        printf("listen failed\n");
        return (0);
    }

    findMax(&maxfd);
    for (;;)
    {

        findMax(&maxfd);
        maxfd = (maxfd > sockfd ? maxfd : sockfd) + 1;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        for (i = 0; i < MAX; i++)
            if (client[i] != 0)
                FD_SET(client[i], &fds);

        /* Wait for some input or connection request. */
        select(maxfd, &fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

        {
            /* if there is a request for a new connection */
            if (FD_ISSET(sockfd, &fds))
            {
                /* If no of active clients is less than MAX accept the request */
                if (ActiveClients < MAX)
                {
                    found = 0;
                    for (i = 0; i < MAX && !found; i++)
                    {
                        if (client[i] == 0)
                        {
                            x[0] = 0;  //to clear to char array
                            x1[0] = 0; //to clear to char array

                            client[i] = accept(sockfd, (struct sockaddr *)&their_addr[i], &sin_size);

                            printf(ANSI_COLOR_RED "IP: %s with port: %d joined \n", inet_ntoa(their_addr[i].sin_addr), ntohs(their_addr[i].sin_port));
                            nread = recv(client[i], nick[i], sizeof(buf), 0);
                            nick[i][nread] = '\0';
                            printf(ANSI_COLOR_RED "user nick: %s \n", nick[i]);

                            //for print the room status
                            for (j = 0; j < 5; ++j)
                            {
                                sprintf(x, "%d", j + 1); // integer to string
                                strcat(x1, "ROOM");
                                strcat(x1, x);
                                for (k = 0; k < 2; ++k)
                                {
                                    if (Room[j][k] != -1)
                                    {
                                        strcat(x1, "   ");
                                        strcat(x1, nick[Room[j][k]]);
                                    }
                                }
                                strcat(x1, "\n");
                            }
                            strcat(x1, "Enter Room name: ex(3) \n");
                            send(client[i], x1, sizeof(x1), 0);

                            //loop runs until client selects an avaible room
                        loop:

                            x[0] = 0;  //to clear to char array
                            x1[0] = 0; //to clear to char array

                            nread = recv(client[i], buf, sizeof(buf), 0);
                            buf[nread] = '\0';
                            rm = atoi(buf) - 1;
                            if (Room[rm][1] != -1 && Room[rm][0] != -1)
                            {
                                send(client[i], "Room is full enter again: \n", 29, 0);
                                goto loop;
                            }
                            else
                            {   strcpy(x1,nick[i]);
                                strcat(x1, " entered the chat Room");
                                sprintf(x, "%d", (rm+1)); // integer to string
                                strcat(x1, x);
                                printf("%s \n",x1);
                                strcat(x1, "\n");
                                if (Room[rm][0] == -1){
                                    Room[rm][0] = i;
                                    send(client[Room[rm][1]], x1, sizeof(x1), 0);
                                }
                                else{
                                    Room[rm][1] = i;
                                    send(client[Room[rm][0]], x1, sizeof(x1), 0);
                                }
                            }
                            found = 1;
                            ActiveClients++;
                            x[0] = 0; 
                            x1[0] = 0; //to clear to char array
                            strcpy(x1, "Entered Room");
                            strcat(x1, buf);
                            strcat(x1, "\n");
                            send(client[i], x1, sizeof(x1), 0);
                        }
                    }
                }
            }
            /*If one of the clients has some input, read and send it to all others.*/
            for (i = 0; i < MAX; i++)
                if (FD_ISSET(client[i], &fds))
                {
                    nread = recvfrom(client[i], buf, sizeof(buf), 0, (struct sockaddr *)&their_addr[i], &sin_size);

                    /* If error or eof, terminate the connection */
                    if (nread < 1)
                    {
                        printf(ANSI_COLOR_BLUE "IP: %s with port: %d nick:%s left \n", inet_ntoa(their_addr[i].sin_addr), ntohs(their_addr[i].sin_port),nick[i]);
                        x1[0] = 0;
                        strcpy(x1, nick[i]);
                        strcat(x1, " is left the chat room \n");
                        for (k = 0; k < Room_size; ++k)
                        {
                            if (Room[k][0] == i)
                            {
                                Room[k][0] = -1; //to empty that space
                                send(client[Room[k][1]], x1, sizeof(x1), 0);
                            }
                            if (Room[k][1] == i)
                            {
                                Room[k][1] = -1; //to empty that space
                                send(client[Room[k][0]], x1, sizeof(x1), 0);
                            }
                        }

                        close(client[i]);
                        client[i] = 0;

                        ActiveClients--;
                    }
                    else
                    {
                        /* send the message */
                        for (k = 0; k < Room_size; ++k)
                        {
                            if (Room[k][0] == i)
                            {
                                send(client[Room[k][1]], buf, nread, 0);
                                buf[nread] = 0;
                                printf(ANSI_COLOR_YELLOW "ROOM %d %s said to %s : %s \n", k + 1, nick[i], nick[Room[k][1]], buf);
                            }
                            else if (Room[k][1] == i)
                            {
                                send(client[Room[k][0]], buf, nread, 0);
                                buf[nread] = 0;
                                printf(ANSI_COLOR_YELLOW "ROOM %d %s said to %s : %s \n", k + 1, nick[i], nick[Room[k][0]], buf);
                            }
                        }
                    }
                }
        }
    }
}