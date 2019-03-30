#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define printf_own(X) printf(ANSI_COLOR_CYAN "%s", X);
#define SIZE sizeof(struct sockaddr_in)

//to append nick and message together
char *append(char *string1, char *string2)
{
    char *result = NULL;
    asprintf(&result, "%s%s", string1, string2);
    return result;
}

int main()
{

    int flag = 0; //checks if client has been entered a room 
    char nick[128]; //nick name of client
    int sockfd, nread;
    char buf[1024], enter, resp;
    fd_set fds;
    char IP[20];
    struct sockaddr_in server = {AF_INET, 5000};

    printf_own("\n\n\n\n\nEnter IP address of the Server\n");
    scanf("%s%c", IP, &enter);
    server.sin_addr.s_addr = inet_addr(IP);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error creating SOCKET\n");
        return (0);
    }
    if (connect(sockfd, (struct sockaddr *)&server, SIZE) == -1)
    {
        printf("Connect failed\n");
        return (0);
    }

    printf_own("Enter a nick name:\n");
    scanf("%s%c", nick, &enter);
    send(sockfd, nick, strlen(nick), 0);
    strcat(nick, ": ");

    //for selecting room
    while (flag < 2)
    {
        printf_own("\n");
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        FD_SET(0, &fds);

        /* Wait for some input. */
        select(sockfd + 1, &fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
        if (FD_ISSET(sockfd, &fds))
        {
            nread = recv(sockfd, buf, sizeof(buf), 0);
            buf[nread] = 0;
            printf(ANSI_COLOR_YELLOW "%s " ANSI_COLOR_CYAN "\n", buf);
            if (strcmp(buf, "Room is full enter again: \n") != 0)
                flag++;
        }
        //flag == 0 printf Room status and increase flag 1
        //when client enters an avaible Room flag inrease again and become 2 
        //then this part is not working again  
        if (flag == 1)
        { 
            scanf("%s%c", buf, &enter);
            send(sockfd, buf, strlen(buf), 0);
        }
    }

    printf_own("Enter a message (E to exit)\n");
    do
    {
        printf_own("\n");
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        FD_SET(0, &fds);
        /* Wait for some input. */
        select(sockfd + 1, &fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

        /* If either device has some input,read it and copy it to the other. */
        if (FD_ISSET(0, &fds))
        {
            nread = read(0, buf, sizeof(buf));
            /* If error or eof, terminate. */
            if (nread < 1)
            {
                close(sockfd);
                exit(0);
            }
            else if ((buf[0] == 'e' || buf[0] == 'E') && nread == 2)
            {
                close(sockfd);
                exit(0);
            }
            else
            {
                strcpy(buf, append(nick, buf));
                send(sockfd, buf, nread + strlen(nick), 0);
            }
        }

        if (FD_ISSET(sockfd, &fds))
        {
            nread = recv(sockfd, buf, sizeof(buf), 0);

            /* If error or eof, terminate. */
            if (nread < 1)
            {
                close(sockfd);
                exit(0);
            }
            buf[nread] = 0;
            printf(ANSI_COLOR_YELLOW " %s " ANSI_COLOR_CYAN "\n", buf);
        }

    } while (1);
}