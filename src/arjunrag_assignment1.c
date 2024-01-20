/**
 * @arjunrag_assignment1
 * @author  Arjun Ragavendaar Sripathy <arjunrag@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */

/*References
  Demo code
  https://beej.us/guide/bgnet/html/split/index.html
  geeks for geeks for socket programming understanding
  -https://www.geeksforgeeks.org/socket-programming-cc/
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <regex.h>
#include "../include/global.h"
#include "../include/logger.h"
#define MLENGTH 900000
#define LENGTH 256
#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 900000
#define BUFFER_SIZE 900000
#define UBIT_NAME "Arjun Ragavendaar Sripathy"

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
// int main(int argc, char **argv)
// {
// 	/*Init. Logger*/
// 	cse4589_init_log(argv[2]);

// 	/*Clear LOGFILE*/
// 	fclose(fopen(LOGFILE, "w"));

// 	/*Start Here*/

// 	return 0;
// }
// using array of structures to store loggedin client details

struct bufferdata
{
    int id;
    int status; // 1-yet to be delivered   0-it is delivered
    char message[1024];
    int user_fd;
    char fromip[1024];
    char toip[1024];
    int isbroadcast;
};

struct clblockedlst
{
    int id;
    int status;
    char host_name[1000];
    int port_no;
    char ip_address[1000];
};

struct local_copy_bllist
{
    char ip_add[1024];
    int status;
};

struct loggeduserlist
{
    int id;
    int status;
    int messages_sent;
    int messages_recieved;
    char host_name[1000];
    int port_no;
    char ip_address[1000];
    int bl_count;
    int user_fd;
    int msg_count;
    struct clblockedlst blst[10];
    struct bufferdata bufferdata[1000];
};

int itr = 0;
int itrcl = 0;
int lb = 0;
struct loggeduserlist cl[5], cl1[5];
struct local_copy_bllist local_blocked[10];
char listforclient[1024] = "";
int checkIP(char *ipsring);
void startserver(char **argv);
void startclient(char **argv);
void sortrecords_server();
void sortrecords_client();

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

void sortrecords_server()
{
    struct loggeduserlist temp;
    for (int i = 0; i < itr - 1; i++)
    {
        for (int j = 0; j < itr - 1 - i; j++)
        {
            if (cl[j].port_no > cl[j + 1].port_no)
            {
                temp = cl[j];
                cl[j] = cl[j + 1];
                cl[j + 1] = temp;
            }
        }
    }
}

void sortrecords_client()
{
    struct loggeduserlist temp;
    for (int i = 0; i < itrcl - 1; i++)
    {
        for (int j = 0; j < itrcl - 1 - i; j++)
        {
            if (cl1[j].port_no > cl1[j + 1].port_no)
            {
                temp = cl1[j];
                cl1[j] = cl1[j + 1];
                cl1[j + 1] = temp;
            }
        }
    }
}

void sort_records_blocked(int k)
{
    struct clblockedlst temp;
    int val = cl[k].bl_count;
    for (int i = 0; i < val - 1; i++)
    {
        for (int j = 0; j < val - 1 - i; j++)
        {
            if (cl[k].blst[j].port_no > cl[k].blst[j + 1].port_no)
            {
                temp = cl[k].blst[j];
                cl[k].blst[j] = cl[k].blst[j + 1];
                cl[k].blst[j + 1] = temp;
            }
        }
    }
}

int check_already_inblocked_list(int k, char *client_ip)
{
    int pos = 2000;
    // printf("bllist-count%d\n",cl[k].bl_count);
    for (int j = 0; j < cl[k].bl_count; j++)
    {
        // printf("%s-----%s",cl[k].blst[j].ip_address,client_ip);
        if (strcmp(cl[k].blst[j].ip_address, client_ip) == 0)
        {
            // printf("yessssssss------%s-----%s",cl[k].blst[j].ip_address,client_ip);
            pos = j;
            break;
        }
    }
    return pos;
}

int checkIP(char *IP)
{
    struct sockaddr_in ip_add;
    int ip_status = inet_pton(AF_INET, IP, &ip_add.sin_addr);
    return ip_status;
}

int ip_Exists(char *IP)
{
    int present = 0;
    for (int k = 0; k < itr; k++)
    {
        if (strcmp(IP, cl[k].ip_address) == 0)
        {
            present = 1;
            break;
        }
    }

    return present;
}

int ip_Exists_client(char *IP)
{
    int present = 0;
    for (int k = 0; k < itrcl; k++)
    {
        if (strcmp(IP, cl1[k].ip_address) == 0)
        {
            present = 1;
            break;
        }
    }

    return present;
}

int checkinto_ip_list(char *toip, char *fromip, int position)
{
    int bl = 1;
    for (int i = 0; i < cl[position].bl_count; i++)
    {
        if (strcmp(cl[position].blst[i].ip_address, fromip) == 0 && cl[position].blst[i].status == 1)
        {
            bl = 0;
            break;
        }
    }
    return bl;
}

int checkinfrom_ip_list(char *toip, char *fromip, int position)
{
    int bl = 1;
    for (int i = 0; i < cl[position].bl_count; i++)
    {
        if (strcmp(cl[position].blst[i].ip_address, toip) == 0 && cl[position].blst[i].status == 1)
        {
            bl = 0;
            break;
        }
    }
    return bl;
}
/* begin of the sample codes */
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage:%s [port]\n", argv[0]);
        // printf("%d",argc);
        exit(-1);
    }

    cse4589_init_log(argv[2]);
    /*Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

    // struct loggeduserlist cl[5];
    if (strcmp(argv[1], "s") == 0)
    {
        startserver(argv);
    }
    else
    {
        // printf("Trigger client\n");
        startclient(argv);
    }
    return 0;
}

void startserver(char **argv)
{
    // struct loggeduserlist cl[5];
    int server_socket, head_socket, selret, sock_index, fdaccept = 0;
    socklen_t caddr_len;
    struct sockaddr_in client_addr;
    struct addrinfo hints, *res;
    fd_set master_list, watch_list;

    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* Fill up address structures */
    if (getaddrinfo(NULL, argv[2], &hints, &res) != 0)
        perror("getaddrinfo failed");

    /* Socket */
    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_socket < 0)
        perror("Cannot create socket");

    /* Bind */
    if (bind(server_socket, res->ai_addr, res->ai_addrlen) < 0)
        perror("Bind failed");

    freeaddrinfo(res);

    /* Listen */
    if (listen(server_socket, BACKLOG) < 0)
        perror("Unable to listen on port");

    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;

    while (TRUE)
    {
        memcpy(&watch_list, &master_list, sizeof(master_list));

        // printf("\n[PA1-Server@CSE489/589]$ ");
        // fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if (selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if (selret > 0)
        {
            /* Loop through socket descriptors to check which ones are ready */
            for (sock_index = 0; sock_index <= head_socket; sock_index += 1)
            {

                if (FD_ISSET(sock_index, &watch_list))
                {

                    /* Check if new command on STDIN */ /*demo_code*/
                    if (sock_index == STDIN)
                    {
                        char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);

                        memset(cmd, '\0', CMD_SIZE);
                        if (fgets(cmd, CMD_SIZE - 1, stdin) == NULL) // Mind the newline character that will be written to cmd
                            exit(-1);
                        // int response=1;
                        // Process PA1 commands here ...
                        char copy_cmd[1024] = "";
                        strcpy(copy_cmd, cmd);
                        char *l_cmd = strtok(copy_cmd, " ");
                        /* end of the sample codes */
                        if (strcmp("IP\n", cmd) == 0)
                        {
                            char ip_addr[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN- denotes ipv4 since ip not specified explicitly,hence using ipv4.
                            // for UDP socket protocol can be 0 or IPPROTO_UDP, for TCP it is 0 or IPPROTO_TCP (since requirement is UDP socket IPPROTO_UDP has been used)
                            int udpsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                            struct sockaddr_in address;
                            if (udpsocket < 0)
                            {
                                printf("Creation of UDP socket failed!!\n");
                                // exit(EXIT_FAILURE);
                            }
                            address.sin_family = AF_INET;
                            // address.sin_addr.s_addr=INADDR_ANY;
                            inet_pton(AF_INET, "8.8.8.8", &(address.sin_addr)); // presentation to network bits
                            address.sin_port = htons(53);

                            int connection_result = connect(udpsocket, (struct sockaddr *)&address, sizeof(address));
                            int addlen = sizeof(address);
                            int getsocktresult = getsockname(udpsocket, (struct sockaddr *)&address, &(addlen));
                            inet_ntop(AF_INET, &(address.sin_addr), ip_addr, INET_ADDRSTRLEN); // network to presentation that is IP address string
                            if (connection_result >= 0 && getsocktresult >= 0)
                            {
                                // Successful
                                // printf("IP:%s\n",ip_addr);

                                cse4589_print_and_log("[%s:SUCCESS]\n", "IP");

                                cse4589_print_and_log("IP:%s\n", ip_addr);

                                cse4589_print_and_log("[%s:END]\n", "IP");
                            }
                            else
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "IP");

                                cse4589_print_and_log("[%s:END]\n", "IP");
                            }
                        }
                        else if (strcmp("AUTHOR\n", cmd) == 0)
                        {
                            cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
                            cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "arjunrag");
                            cse4589_print_and_log("[%s:END]\n", "AUTHOR");
                        }
                        else if (strcmp("PORT\n", cmd) == 0)
                        {
                            int arglength = sizeof(argv) / sizeof(argv[0]);
                            if (arglength > 0 && argv[2])
                            {
                                int port_val = atoi(argv[2]);
                                cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
                                cse4589_print_and_log("PORT:%d\n", port_val);
                                cse4589_print_and_log("[%s:END]\n", "PORT");
                            }
                            else
                            {
                                // printf("PORT-error\n");
                                cse4589_print_and_log("[%s:ERROR]\n", "PORT");
                                cse4589_print_and_log("[%s:END]\n", "PORT");
                            }
                        }
                        else if (strcmp("LIST\n", cmd) == 0)
                        {
                            // if(itr==0){
                            //     printf("No Clients Logged In!!");
                            // }
                            sortrecords_server();
                            cse4589_print_and_log("[%s:SUCCESS]\n", "LIST");
                            int c = 1;
                            for (int i = 0; i < itr; i++)
                            {
                                if (cl[i].id != -1 && cl[i].status != 0)
                                {
                                    cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", c, cl[i].host_name, cl[i].ip_address, cl[i].port_no);
                                    c++;
                                }
                            }
                            cse4589_print_and_log("[%s:END]\n", "LIST");
                        }
                        else if (strcmp("STATISTICS\n", cmd) == 0)
                        {
                            // if(itr==0){
                            //     printf("No Clients Logged In!!");
                            // }
                            sortrecords_server();
                            cse4589_print_and_log("[%s:SUCCESS]\n", "STATISTICS");
                            int c = 1;
                            for (int i = 0; i < itr; i++)
                            {
                                if (cl[i].id != -1)
                                {
                                    char logged_status[1024] = "";
                                    if (cl[i].status == 1)
                                    {
                                        strcpy(logged_status, "logged-in");
                                    }
                                    else
                                    {
                                        strcpy(logged_status, "logged-out");
                                    }
                                    cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", c, cl[i].host_name, cl[i].messages_sent, cl[i].messages_recieved, logged_status);
                                    c++;
                                }
                            }
                            cse4589_print_and_log("[%s:END]\n", "STATISTICS");
                        }
                        else if (strcmp("BLOCKED", l_cmd) == 0)
                        {
                            l_cmd = strtok(NULL, "\n");
                            int ip_res = checkIP(l_cmd);
                            int ip_val = ip_Exists(l_cmd);
                            if (ip_res != 0 && ip_val == 1)
                            {

                                cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCKED");
                                for (int k = 0; k < itr; k++)
                                {
                                    if (strcmp(l_cmd, cl[k].ip_address) == 0 && cl[k].id != -1)
                                    {
                                        sort_records_blocked(k);
                                        for (int j = 0; j < cl[k].bl_count; j++)
                                        {
                                            int c = 1;
                                            if (cl[k].blst[j].status == 1)
                                            {
                                                cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", c, cl[k].blst[j].host_name, cl[k].blst[j].ip_address, cl[k].blst[j].port_no);
                                                c++;
                                            }
                                        }
                                    }
                                }
                                cse4589_print_and_log("[%s:END]\n", "BLOCKED");
                            }
                            else
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "BLOCKED");
                                cse4589_print_and_log("[%s:END]\n", "BLOCKED");
                            }
                        }

                        free(cmd);
                    }
                    /* begin of the sample codes */
                    /* Check if new client is requesting connection */
                    else if (sock_index == server_socket)
                    { /* begin of the sample codes */
                        struct hostent *nameofhost;
                        caddr_len = sizeof(struct sockaddr_in);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if (fdaccept < 0)
                            perror("Accept failed.");

                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if (fdaccept > head_socket)
                            head_socket = fdaccept;
                        struct in_addr claddress;
                        nameofhost = NULL;
                        // storing the logged in client details in form of array of structures
                        /* end of the sample codes */
                        // now converting the ipaddress which is in form of network address to ip address of string form and storing it in a variable
                        char client_ip_address[INET_ADDRSTRLEN], client_ip_address_copy[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_address, INET_ADDRSTRLEN);
                        // printf("%s\n",client_ip_address);
                        //  struct sockaddr_in peeradd;
                        //  socklen_t peer_len=sizeof(peeradd);
                        //  if(getpeername(fdaccept,(struct sockaddr *)&peeradd,&peer_len)<0){
                        //      perror("getpeername failed");
                        //  }
                        /* end of the sample codes */
                        int p_no = ntohs(client_addr.sin_port); // ntohs(peeradd.sin_port);//ntohs(client_addr.sin_port); //unsigned short int is the actual data type of port in struct sockaddr_in
                        // printf("%d\n",p_no);
                        strcpy(client_ip_address_copy, client_ip_address);
                        // retrieve host name via gethostbyaddress function
                        inet_pton(AF_INET, client_ip_address_copy, &claddress);
                        /* since gethostbyaddress requires first parameter to
                        be address of type struct in_address we are converting it using pton*/
                        int lval = sizeof(claddress);
                        nameofhost = gethostbyaddr(&claddress, lval, AF_INET);
                        char *actual_h_name = nameofhost->h_name;
                        // printf("%s\n",actual_h_name);
                        cl[itr].id = itr + 1;          // id of list
                        cl[itr].messages_sent = 0;     // none
                        cl[itr].messages_recieved = 0; // none
                        cl[itr].user_fd = fdaccept;
                        cl[itr].status = 1;
                        strcpy(cl[itr].host_name, actual_h_name);      // name of the host(canonical name)
                        cl[itr].port_no = p_no;                        //(port no)
                        strcpy(cl[itr].ip_address, client_ip_address); // IP Address
                        char tstr[100];
                        sprintf(tstr, "%d", cl[itr].id);
                        strcat(listforclient, tstr);
                        strcat(listforclient, "|");
                        strcat(listforclient, actual_h_name);
                        strcat(listforclient, "|");
                        strcat(listforclient, client_ip_address);
                        strcat(listforclient, "|");
                        //  char tport[100];
                        // sprintf(tport,"%d",cl[itr].port_no);
                        // strcat(listforclient,tport);
                        // strcat(listforclient,"\n");
                        itr++;
                        // printf("\nRemote Host connected %s!\n",cl[itr-1].ip_address);
                    }
                    /* Read from existing clients */
                    else
                    {
                        /* Initialize buffer to receieve response */
                        char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if (recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0)
                        {
                            close(sock_index);
                            // printf("Remote Host terminated connection!\n");

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else
                        {
                            // Process incoming data from existing clients here ...

                            // printf("\nClient sent me: %s\n", buffer);
                            char temp_string[1024] = "";
                            strcpy(temp_string, buffer);
                            char *token;
                            int k = 0;
                            token = strtok(temp_string, "|");
                            while (token != NULL)
                            {
                                k++;
                                token = strtok(NULL, "|");
                                if (k == 3)
                                    break;
                            }
                            // printf("%s\n",token);
                            char *recvdfromclient = strtok(buffer, "|");
                            // printf("%s\n--%d",recvdfromclient,itr);
                            regex_t regex;
                            int regxval = regcomp(&regex, "REFRESHSTART", 0);
                            int reg_res = regexec(&regex, recvdfromclient, 0, NULL, 0);
                            // if(strcmp("REFRESHSTART",recvdfromclient)==0){
                            if (reg_res == 0)
                            {
                                char loggedinlist[1024] = "";
                                cl[itr - 1].port_no = atoi(token);
                                strcat(listforclient, token); // appending port data;
                                strcat(listforclient, "\n");
                                if (itr > 0)
                                {
                                    // char *finallist=(char*)malloc(sizeof(char)*MLENGTH);
                                    // for(int i=0;i<itr;i++){
                                    //     strcat(loggedinlist,"REFRESHSTART");
                                    //     strcat(loggedinlist,"|");
                                    //     char tstr[100];
                                    //     //itoa(cl[i].id,tstr,10);
                                    //     sprintf(tstr,"%d",cl[i].id);
                                    //     strcat(loggedinlist,tstr);
                                    //     strcat(loggedinlist,"|");
                                    //     strcat(loggedinlist,cl[i].host_name);
                                    //     strcat(loggedinlist,"|");
                                    //     char tport[100]="";
                                    //     //itoa(cl[i].port_no,tport,10);
                                    //     sprintf(tport,"%d",cl[i].port_no);
                                    //     strcat(loggedinlist,tport);
                                    //     strcat(loggedinlist,"|");
                                    //     strcat(loggedinlist,cl[i].ip_address);
                                    //     strcat(loggedinlist,"\n");

                                    //     strcpy(finallist,loggedinlist);
                                    //         }
                                    // printf("ECHOing it back to the remote host ...%s\n",loggedinlist);
                                    char fstr[1200] = "";
                                    strcat(fstr, "REFRESHSTART|");
                                    strcat(fstr, listforclient);
                                    send(fdaccept, fstr, strlen(fstr), 0);
                                    //     if(send(fdaccept, finallist, strlen(finallist), 0) == strlen(finallist))
                                    // printf("Done!\n");
                                }
                                else
                                {
                                    // printf("Sending ClientList Error!!!!---Line-278\n ");
                                    exit(EXIT_FAILURE);
                                }
                            }
                            else if (strcmp("EXIT", recvdfromclient) == 0)
                            {
                                int tk = 1;
                                while (recvdfromclient != NULL)
                                {
                                    tk++;
                                    recvdfromclient = strtok(NULL, "|");
                                    if (tk == 2)
                                        break;
                                }

                                for (int i = 0; i < itr; i++)
                                {
                                    if (cl[i].id != -1 && cl[i].port_no == atoi(recvdfromclient))
                                    {
                                        cl[i].id = -1;
                                        // cl[i].status=0;
                                        break;
                                    }
                                }

                                // printf("%d\n",atoi(recvdfromclient));
                            }
                            else if (strcmp("REFRESHNOW", recvdfromclient) == 0)
                            {
                                char refresh_list[1024] = "";
                                char fstr1[1200] = "";
                                char tstr[100];
                                for (int i = 0; i < itr; i++)
                                {
                                    sprintf(tstr, "%d", cl[i].id);
                                    strcat(refresh_list, tstr);
                                    strcat(refresh_list, "|");
                                    strcat(refresh_list, cl[i].host_name);
                                    strcat(refresh_list, "|");
                                    strcat(refresh_list, cl[i].ip_address);
                                    strcat(refresh_list, "|");
                                    char tport[100] = "";
                                    // itoa(cl[i].port_no,tport,10);
                                    sprintf(tport, "%d", cl[i].port_no);
                                    strcat(refresh_list, tport); // appending port data;
                                    strcat(refresh_list, "\n");
                                }

                                strcat(fstr1, "REFRESHNOW|");
                                strcat(fstr1, refresh_list);
                                send(sock_index, fstr1, strlen(fstr1), 0);
                            }
                            else if (strcmp("BLOCK", recvdfromclient) == 0)
                            {
                                int trace = 0;
                                char fromclient_ip[1024] = "";
                                int fromclient_port;
                                char toclient_ip[1024] = "";
                                while (recvdfromclient != NULL)
                                {
                                    trace++;
                                    recvdfromclient = strtok(NULL, "|");
                                    if (trace == 1)
                                    {
                                        strcpy(fromclient_ip, recvdfromclient);
                                    }
                                    else if (trace == 2)
                                    {
                                        fromclient_port = atoi(recvdfromclient);
                                    }
                                    else if (trace == 3)
                                    {
                                        strcpy(toclient_ip, recvdfromclient);
                                    }
                                }
                                int exists_val;
                                for (int k = 0; k < itr; k++)
                                {
                                    if (strcmp(cl[k].ip_address, fromclient_ip) == 0)
                                    {
                                        // printf("fromip---%s-----listfromip-------%s---------%d\n",fromclient_ip,cl[k].ip_address,k);
                                        exists_val = check_already_inblocked_list(k, toclient_ip);
                                        // printf("exists_val--%d\n",exists_val);
                                        // printf("toip------%s",toclient_ip);
                                        if (exists_val == 2000)
                                        {
                                            // printf("inside\n");
                                            for (int j = 0; j < itr; j++)
                                            {
                                                if (strcmp(toclient_ip, cl[j].ip_address) == 0)
                                                {
                                                    strcpy(cl[k].blst[cl[k].bl_count].ip_address, toclient_ip);
                                                    strcpy(cl[k].blst[cl[k].bl_count].host_name, cl[j].host_name);
                                                    cl[k].blst[cl[k].bl_count].port_no = cl[j].port_no;
                                                    cl[k].blst[cl[k].bl_count].status = 1;
                                                    cl[k].blst[cl[k].bl_count].id = cl[j].id;
                                                    cl[k].bl_count++;
                                                    // printf("blcount--%d",cl[k].bl_count);
                                                    break;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            cl[k].blst[exists_val].status = 1;
                                        }
                                    }
                                }

                                // for(int k=0;k<itr;k++){
                                //   cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",k+1,cl[k].host_name,cl[k].ip_address,cl[k].port_no);
                                //   if(cl[k].bl_count>0){
                                //       printf("Blocked:\n");
                                //       for(int l=0;l<cl[k].bl_count;l++){
                                //          printf("%s\n",cl[k].blst[l].ip_address);
                                //       }

                                //   }
                                // }
                            }
                            else if (strcmp("UNBLOCK", recvdfromclient) == 0)
                            {
                                int trace = 0;
                                char fromclient_ip[1024] = "";
                                int fromclient_port;
                                char toclient_ip[1024] = "";
                                while (recvdfromclient != NULL)
                                {
                                    trace++;
                                    recvdfromclient = strtok(NULL, "|");
                                    if (trace == 1)
                                    {
                                        strcpy(fromclient_ip, recvdfromclient);
                                    }
                                    else if (trace == 2)
                                    {
                                        fromclient_port = atoi(recvdfromclient);
                                    }
                                    else if (trace == 3)
                                    {
                                        strcpy(toclient_ip, recvdfromclient);
                                    }
                                }

                                for (int k = 0; k < itr; k++)
                                {
                                    if (cl[k].port_no == fromclient_port)
                                    {
                                        int exists_val = check_already_inblocked_list(k, toclient_ip);
                                        cl[k].blst[exists_val].status = 0;
                                        break;
                                    }
                                }
                            }
                            else if (strcmp("SEND", recvdfromclient) == 0)
                            {
                                int trace = 0, frompos;
                                char msg_val[20000] = "";
                                char toclient_ip[1024] = "";
                                char fromip[1024] = "";
                                while (recvdfromclient != NULL)
                                {
                                    trace++;
                                    recvdfromclient = strtok(NULL, "|");
                                    if (trace == 1)
                                    {
                                        strcpy(toclient_ip, recvdfromclient);
                                    }
                                    else if (trace == 2)
                                    {
                                        strcpy(msg_val, recvdfromclient);
                                    }
                                }
                                // printf("%s----%s\n",toclient_ip,msg_val);

                                for (int k = 0; k < itr; k++)
                                {
                                    if (cl[k].user_fd == sock_index)
                                    {
                                        strcpy(fromip, cl[k].ip_address);
                                        frompos = k;
                                        break;
                                    }
                                }
                                char msg_to_send[20000] = "";
                                int send_index, position;
                                for (int k = 0; k < itr; k++)
                                {
                                    if (strcmp(toclient_ip, cl[k].ip_address) == 0)
                                    {
                                        send_index = cl[k].user_fd;
                                        position = k;
                                        break;
                                    }
                                }
                                strcat(msg_to_send, "SEND");
                                strcat(msg_to_send, "|");
                                strcat(msg_to_send, fromip);
                                strcat(msg_to_send, "|");
                                strcat(msg_to_send, msg_val);
                                strcat(msg_to_send, "|");
                                strcat(msg_to_send, toclient_ip);

                                int block_res_to = checkinto_ip_list(toclient_ip, fromip, position);
                                int block_res_from = checkinfrom_ip_list(toclient_ip, fromip, frompos);
                                int logout_status = cl[position].status;
                                // printf("result from block1---%d, result from block2-----%d",block_res_to,block_res_from);
                                //  if(block_res_to==1 && block_res_from==1 && logout_status==1 ){
                                //   char status_to_send[1024]="";
                                //     strcat(status_to_send,"STATUSSEND");
                                //     strcat(status_to_send,"|");
                                //     strcat(status_to_send,"1");
                                //    int res_success=send(sock_index,status_to_send,strlen(status_to_send),0);

                                // }
                                if (block_res_to == 1 && block_res_from == 1 && logout_status == 1 && cl[position].id != -1 && send(send_index, msg_to_send, strlen(msg_to_send), 0) > 0)
                                {
                                    // fflush(NULL);
                                    cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                                    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", fromip, toclient_ip, msg_val);
                                    cse4589_print_and_log("[RELAYED:END]\n");
                                    cl[frompos].messages_sent++;
                                    cl[position].messages_recieved++;
                                }
                                else
                                {
                                    if (cl[position].id == -1)
                                    {
                                        cse4589_print_and_log("[RELAYED:ERROR]\n");
                                        // cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n",fromip,toclient_ip,msg_val);
                                        cse4589_print_and_log("[RELAYED:END]\n");
                                    }
                                    else if (logout_status == 0 && block_res_to == 1 && block_res_from == 1 && cl[position].id != -1)
                                    {
                                        // printf("positionvalval---%d\n",position);
                                        cl[frompos].messages_sent++;
                                        cl[position].bufferdata[cl[position].msg_count].id = cl[position].msg_count + 1;
                                        strcpy(cl[position].bufferdata[cl[position].msg_count].message, msg_val);
                                        strcpy(cl[position].bufferdata[cl[position].msg_count].fromip, fromip);
                                        strcpy(cl[position].bufferdata[cl[position].msg_count].toip, toclient_ip);
                                        cl[position].bufferdata[cl[position].msg_count].user_fd = send_index;
                                        cl[position].bufferdata[cl[position].msg_count].status = 1;
                                        cl[position].msg_count++;
                                        // printf("send-count---%d\n",cl[k].msg_count);
                                        cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                                        cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", fromip, toclient_ip, msg_val);
                                        cse4589_print_and_log("[RELAYED:END]\n");

                                        // for(int l=0;l<cl[position].msg_count;l++){
                                        //     printf("ipaddress-%s ,%d,%s,%s,%s,%d\n",
                                        //         cl[position].ip_address,
                                        //         cl[position].bufferdata[l].id,
                                        //         cl[position].bufferdata[l].message,cl[position].bufferdata[l].fromip,
                                        //         cl[position].bufferdata[l].toip,cl[position].bufferdata[l].user_fd);
                                        // }

                                        // cse4589_print_and_log("[RELAYED:ERROR]\n");
                                        // cse4589_print_and_log("[RELAYED:END]\n");
                                    }

                                    // else if(block_res_to!=1 || block_res_from!=1){
                                    //     char error_to_send[1024]="";
                                    //   strcat(error_to_send,"STATUSSEND");
                                    //   strcat(error_to_send,"|");
                                    //   strcat(error_to_send,"0");
                                    //  int res_error=send(sock_index,error_to_send,strlen(error_to_send),0);
                                    // }
                                }
                            }
                            else if (strcmp("BROADCAST", recvdfromclient) == 0)
                            {
                                int fl = 0, frompos, position, fl1 = 0;
                                recvdfromclient = strtok(NULL, "|");
                                char b_msg[1024] = "";
                                char brd_msg[1024] = "";
                                char fromip[1024] = "";

                                for (int k = 0; k < itr; k++)
                                {
                                    if (cl[k].user_fd == sock_index)
                                    {
                                        strcpy(fromip, cl[k].ip_address);
                                        frompos = k;
                                        break;
                                    }
                                }

                                strcpy(b_msg, recvdfromclient);
                                // printf("%s",b_msg);
                                strcat(brd_msg, "BROAD");
                                strcat(brd_msg, "|");
                                strcat(brd_msg, b_msg);
                                strcat(brd_msg, "|");
                                strcat(brd_msg, fromip);

                                for (int k = 0; k < itr; k++)
                                {
                                    if (cl[k].user_fd != sock_index && checkinto_ip_list(cl[k].ip_address, fromip, k) == 1 && checkinfrom_ip_list(cl[k].ip_address, fromip, frompos) == 1 && cl[k].status == 1)
                                    {
                                        if (send(cl[k].user_fd, brd_msg, strlen(brd_msg), 0) < 0)
                                        {
                                            cse4589_print_and_log("[RELAYED:ERROR]\n");
                                            cse4589_print_and_log("[RELAYED:END]\n");
                                            fl = 1;
                                            break;
                                        }
                                        cl[k].messages_recieved++;
                                    }
                                    else
                                    {
                                        if (cl[k].user_fd == sock_index && checkinto_ip_list(cl[k].ip_address, fromip, k) == 1 && checkinfrom_ip_list(cl[k].ip_address, fromip, frompos) == 1 && cl[k].status == 1)
                                        {
                                            cl[k].messages_sent++;
                                            strcpy(fromip, cl[k].ip_address);

                                            // if loggedout we are storing the messages
                                        }
                                        else if (cl[k].status == 0 && checkinto_ip_list(cl[k].ip_address, fromip, k) == 1 && checkinfrom_ip_list(cl[k].ip_address, fromip, frompos) == 1)
                                        {
                                            fl1 = 1;

                                            cl[k].bufferdata[cl[k].msg_count].id = cl[k].msg_count + 1;
                                            strcpy(cl[k].bufferdata[cl[k].msg_count].message, b_msg);
                                            strcpy(cl[k].bufferdata[cl[k].msg_count].fromip, fromip);
                                            strcpy(cl[k].bufferdata[cl[k].msg_count].toip, cl[k].ip_address);
                                            cl[k].bufferdata[cl[k].msg_count].user_fd = cl[k].user_fd;
                                            cl[k].bufferdata[cl[k].msg_count].status = 1;
                                            cl[k].bufferdata[cl[k].msg_count].isbroadcast = 1;
                                            cl[k].msg_count++;
                                            // printf("Broadcast-count---%d\n",cl[k].msg_count);
                                        }
                                    }
                                }

                                // printf("Broadcast Triggered!!");
                                if (fl == 0 || fl1 == 1)
                                {
                                    cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                                    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", fromip, "255.255.255.255", b_msg);
                                    cse4589_print_and_log("[RELAYED:END]\n");
                                }
                            }
                            else if (strcmp("LOGOUT", recvdfromclient) == 0)
                            {

                                for (int k = 0; k < itr; k++)
                                {
                                    if (cl[k].user_fd == sock_index && cl[k].id != -1)
                                    {
                                        cl[k].status = 0;
                                        break;
                                    }
                                }
                            }
                            else if (strcmp("LOGINAGAIN", recvdfromclient) == 0)
                            {
                                // printf("fired loginagainfromserver");
                                int tk = 1, pos = -1, s_index = -1;
                                while (recvdfromclient != NULL)
                                {
                                    tk++;
                                    recvdfromclient = strtok(NULL, "|");
                                    if (tk == 2)
                                        break;
                                }

                                for (int i = 0; i < itr; i++)
                                {
                                    if (cl[i].user_fd == sock_index && cl[i].id != -1)
                                    {
                                        cl[i].status = 1;
                                        pos = i;
                                        break;
                                    }
                                }

                                // printf("%d\n",sock_index);
                                // printf("message_count--%d",cl[pos].msg_count);
                                // printf("message-count---%d\n",cl[pos].msg_count);
                                char msg_to_send[900000] = "";
                                strcat(msg_to_send, "SENDAGAIN");
                                for (int p = 0; p < cl[pos].msg_count; p++)
                                {

                                    // printf("message-status---%d\n",cl[pos].status);
                                    if (cl[pos].bufferdata[p].status == 1)
                                    {

                                        char fromip[1024] = "";
                                        char toclient_ip[1024] = "";
                                        char msg_val[1024] = "";
                                        int send_index, position, frompos;
                                        strcpy(fromip, cl[pos].bufferdata[p].fromip);
                                        strcpy(toclient_ip, cl[pos].bufferdata[p].toip);
                                        strcpy(msg_val, cl[pos].bufferdata[p].message);
                                        for (int k = 0; k < itr; k++)
                                        {
                                            if (strcmp(fromip, cl[k].ip_address) == 0)
                                            {
                                                // strcpy(fromip,cl[k].ip_address);
                                                frompos = k;
                                                break;
                                            }
                                        }

                                        for (int k = 0; k < itr; k++)
                                        {
                                            if (strcmp(toclient_ip, cl[k].ip_address) == 0)
                                            {
                                                send_index = cl[k].user_fd;
                                                s_index = send_index;
                                                position = k;
                                                break;
                                            }
                                        }

                                        int block_res_to = checkinto_ip_list(toclient_ip, fromip, position);
                                        int block_res_from = checkinfrom_ip_list(toclient_ip, fromip, frompos);
                                        int logout_status = cl[position].status;
                                        // printf("result from block1---%d, result from block2-----%d, sockindex------%d",block_res_to,block_res_from,send_index);
                                        // printf("%s\n",msg_to_send);

                                        if (block_res_to == 1 && block_res_from == 1)
                                        {

                                            strcat(msg_to_send, "|");
                                            strcat(msg_to_send, fromip);
                                            strcat(msg_to_send, "|");
                                            strcat(msg_to_send, msg_val);
                                            strcat(msg_to_send, "|");
                                            strcat(msg_to_send, toclient_ip);
                                            // strcat(msg_to_send,";");

                                            // printf("inside\n");   && send(send_index,msg_to_send,strlen(msg_to_send),0)>0
                                            // fflush(stdout);
                                            // sleep(1);
                                            // cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                                            // if(cl[pos].bufferdata[p].isbroadcast==1){
                                            //    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n",fromip,"255.255.255.255",msg_val);

                                            // }else{
                                            //    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n",fromip,toclient_ip,msg_val);

                                            // }
                                            //  cse4589_print_and_log("[RELAYED:END]\n");

                                            // if(cl[pos].bufferdata[p].isbroadcast!=1){
                                            //     cl[frompos].messages_sent++;
                                            // }

                                            cl[position].messages_recieved++;
                                            cl[pos].bufferdata[p].status = 0;
                                        }
                                    }
                                }

                                int send_again_result = send(s_index, msg_to_send, strlen(msg_to_send), 0);

                                //  char login_val_stat[1000]="";
                                //  strcpy(login_val_stat,"YES|LOGINSUCCESS");
                                // int login_status=send(s_index,login_val_stat,strlen(login_val_stat),0);
                                // fflush(stdout);
                            }

                            fflush(stdout);
                        }

                        free(buffer);
                    }
                }
            }
        }
    }
}
/* begin of the sample codes */
void startclient(char **argv)
{
    // struct loggeduserlist cl[5];
    char *cmdinp;
    int already_logged_in_user = 0;
    int current_login = 0;
    int serverc_socket, head_socket, selret, sock_index, fdaccept = 0, caddr_len, connc_val = 0, sockresult, server_port_no, s_socket;
    struct sockaddr_in serv_addr, temp_addr;
    struct addrinfo hints, *res;
    fd_set master_list, watch_list;
    char *server_ip_address = (char *)malloc(sizeof(char) * LENGTH);

    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* Fill up address structures */
    if (getaddrinfo(NULL, argv[2], &hints, &res) != 0)
        perror("getaddrinfo failed");

    /* Socket */
    serverc_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverc_socket < 0)
        perror("Cannot create socket");

    /* Bind */
    // if(bind(serverc_socket, res->ai_addr, res->ai_addrlen) < 0 )
    //     perror("Bind failed");
    temp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    temp_addr.sin_family = AF_INET;
    temp_addr.sin_port = htons(atoi(argv[2]));

    if (bind(serverc_socket, (struct sockaddr *)&temp_addr, sizeof(temp_addr)) < 0)
        perror("Bind failed");

    freeaddrinfo(res);

    /* Listen */
    if (listen(serverc_socket, BACKLOG) < 0)
        perror("Unable to listen on port");

    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_SET(0, &master_list);
    FD_ZERO(&watch_list);

    // /* Register the listening socket */
    // FD_SET(server_socket, &master_list);
    // /* Register STDIN */
    // FD_SET(STDIN, &master_list);

    head_socket = serverc_socket;
    char *recieve_buffer = (char *)malloc(sizeof(char) * MLENGTH);

    while (TRUE)
    {
        memcpy(&watch_list, &master_list, sizeof(master_list));

        // printf("\n[PA1-Server@CSE489/589]$ ");
        // fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(connc_val + 1, &watch_list, NULL, NULL, NULL);
        if (selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if (selret > 0)
        {
            /* Loop through socket descriptors to check which ones are ready */
            for (sock_index = 0; sock_index <= connc_val; sock_index += 1)
            {

                if (FD_ISSET(sock_index, &watch_list))
                {
                    memset(recieve_buffer, '\0', MLENGTH);
                    /* Check if new command on STDIN */
                    if (sock_index == STDIN)
                    {
                        // printf("hello:375\n");
                        char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);

                        memset(cmd, '\0', CMD_SIZE);
                        if (fgets(cmd, CMD_SIZE - 1, stdin) == NULL) // Mind the newline character that will be written to cmd
                            exit(-1);
                        // int response=1;
                        // Process PA1 commands here ...
                        /* end of the sample codes */
                        char *str_copy1, str_copy[1000];
                        char ip_copy[1000] = "";
                        char port_copy[1000] = "";
                        int login_val = 1;
                        strcpy(str_copy, cmd);
                        str_copy1 = strtok(str_copy, " ");
                        int component = 1;
                        while (str_copy1 != NULL)
                        {
                            if (component == 2)
                            {
                                strcpy(ip_copy, str_copy1);
                            }
                            if (component == 3)
                            {
                                strcpy(port_copy, str_copy1);
                            }
                            str_copy1 = strtok(NULL, " ");
                            component++;
                            if (component < 3 && str_copy1 == NULL)
                            {
                                login_val = 0;
                                break;
                            }
                        }

                        if (component >= 3)
                        {
                            struct sockaddr_in ip_add;
                            int ip_status = inet_pton(AF_INET, ip_copy, &ip_add.sin_addr);
                            if (ip_status == 0)
                            {
                                login_val = 0;
                            }
                            if (login_val != 0)
                            {
                                for (int i = 0; i < strlen(port_copy) - 1; i++)
                                {
                                    // printf("%c\n",port_copy[i]);
                                    int val = (int)port_copy[i];
                                    // printf("%d\n",val);
                                    if ((val) < 48 || (val) > 57)
                                    {
                                        // printf("digit\n");
                                        login_val = 0;
                                        break;
                                    }
                                }
                                if (login_val != 0 && (atoi(port_copy) < 0 || atoi(port_copy) > 65536))
                                {
                                    // printf("totalval\n");
                                    login_val = 0;
                                }
                            }
                        }

                        char *logincmd = strtok(cmd, " ");

                        // printf("%s\n",logincmd);
                        if (strcmp("LOGIN\n", cmd) == 0 || (strcmp("LOGIN", logincmd) == 0 && login_val == 0))
                        {
                            cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                            cse4589_print_and_log("[%s:END]\n", "LOGIN");
                        }
                        else if (strcmp("IP\n", cmd) == 0)
                        {
                            char ip_addr[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN- denotes ipv4 since ip not specified explicitly,hence using ipv4.
                            // for UDP socket protocol can be 0 or IPPROTO_UDP, for TCP it is 0 or IPPROTO_TCP (since requirement is UDP socket IPPROTO_UDP has been used)
                            int udpsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                            struct sockaddr_in address;
                            if (udpsocket < 0)
                            {
                                printf("Creation of UDP socket failed!!\n");
                                // exit(EXIT_FAILURE);
                            }
                            address.sin_family = AF_INET;
                            // address.sin_addr.s_addr=INADDR_ANY;
                            inet_pton(AF_INET, "8.8.8.8", &(address.sin_addr)); // presentation to network bits
                            address.sin_port = htons(53);

                            int connection_result = connect(udpsocket, (struct sockaddr *)&address, sizeof(address));
                            int addlen = sizeof(address);
                            int getsocktresult = getsockname(udpsocket, (struct sockaddr *)&address, &(addlen));
                            inet_ntop(AF_INET, &(address.sin_addr), ip_addr, INET_ADDRSTRLEN); // network to presentation that is IP address string
                            if (connection_result >= 0 && getsocktresult >= 0)
                            {
                                // Successful
                                // printf("IP:%s\n",ip_addr);

                                cse4589_print_and_log("[%s:SUCCESS]\n", "IP");

                                cse4589_print_and_log("IP:%s\n", ip_addr);

                                cse4589_print_and_log("[%s:END]\n", "IP");
                            }
                            else
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "IP");

                                cse4589_print_and_log("[%s:END]\n", "IP");
                            }
                        }
                        else if (strcmp("AUTHOR\n", cmd) == 0)
                        {
                            cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
                            // printf("I,%s, have read and understood the course academic integrity policy.\n",UBIT_NAME);
                            cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "arjunrag");
                            cse4589_print_and_log("[%s:END]\n", "AUTHOR");
                        }
                        else if (strcmp("PORT\n", cmd) == 0)
                        {
                            int arglength = sizeof(argv) / sizeof(argv[0]);
                            if (arglength > 0 && argv[2])
                            {
                                int port_val = atoi(argv[2]);
                                cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
                                cse4589_print_and_log("PORT:%d\n", port_val);
                                cse4589_print_and_log("[%s:END]\n", "PORT");
                            }
                            else
                            {
                                // printf("PORT-error\n");
                                cse4589_print_and_log("[%s:ERROR]\n", "PORT");

                                cse4589_print_and_log("[%s:END]\n", "PORT");
                            }
                        }
                        else if (strcmp("LOGIN", logincmd) == 0)
                        {

                            int valid_login = 1;
                            // spliting the login command based on the spaces
                            logincmd = strtok(NULL, " ");
                            // if(strcmp("LOGIN",logincmd)==0)
                            // printf("hello\n");
                            // printf("outside!!!\n");
                            server_ip_address = logincmd;

                            if (already_logged_in_user == 1 && current_login == 0)
                            {
                                // printf("hello from loginagain");
                                char str_val[1000] = "";
                                strcat(str_val, "LOGINAGAIN");
                                strcat(str_val, "|");
                                strcat(str_val, server_ip_address);
                                int send_now = send(s_socket, str_val, strlen(str_val), 0);
                                if (send_now < 0)
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                    cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                    // current_login=0;
                                }
                                else
                                {
                                    for (int k = 0; k < itrcl; k++)
                                    {
                                        if (cl1[k].ip_address == server_ip_address)
                                        {
                                            cl1[k].status = 1;
                                            break;
                                        }
                                    }

                                    // cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
                                    // cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                    current_login = 1;
                                }
                            }
                            else
                            {

                                if (server_ip_address)
                                {
                                    logincmd = strtok(NULL, " ");
                                    if (logincmd)
                                    {
                                        server_port_no = atoi(logincmd);
                                    }
                                    else
                                    {
                                        // printf("Port Number Missing!!\n");
                                        valid_login = 0;
                                        // cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                        // cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                        // return ;
                                        // exit(EXIT_FAILURE);
                                    }
                                }
                                else
                                {
                                    // printf("IP Address ERROR!!!!\n");
                                    valid_login = 0;
                                    // cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                    // cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                    return;
                                    // exit(EXIT_FAILURE);
                                }

                                s_socket = socket(AF_INET, SOCK_STREAM, 0); // AF_INET-for ipv4,SOCK_STREAM for TCP and 0 for IP protocol
                                // printf("585--%d\n",s_socket);

                                if (s_socket < 0)
                                {
                                    // printf("Scoket creation error for client!!");
                                    valid_login = 0;
                                    // cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                    // cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                    // return;
                                    // exit(8);
                                }
                                else
                                {
                                    FD_SET(s_socket, &master_list);
                                    if (s_socket > connc_val)
                                    {
                                        connc_val = s_socket;
                                    }
                                    serv_addr.sin_family = AF_INET;
                                    serv_addr.sin_port = htons(server_port_no);
                                    int present_to_network = inet_pton(AF_INET, server_ip_address, &serv_addr.sin_addr); // converstion from presenttion to network bits
                                    // printf("601--%d\n",present_to_network);
                                    if (present_to_network < 0)
                                    {
                                        // printf("There Exists error in the conversion!!!\n");
                                        valid_login = 0;
                                        // cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                        // cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                        // return ;
                                        // exit(7);
                                    }
                                    else if (present_to_network == 0)
                                    {
                                        // printf("Invalid Ip Address!!\n");
                                        valid_login = 0;
                                        // cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                        // cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                        // return ;
                                        // exit(6);
                                    }
                                    int connection_result;
                                    char store_ip[INET_ADDRSTRLEN];
                                    connection_result = connect(s_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                                    // printf("618--%d\n",connection_result);
                                    if (valid_login == 1 && connection_result >= 0)
                                    {
                                        // printf("LOGIN Successfull!!\n");
                                        cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
                                        cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                        // creating connection using UDP socket inorder to get the updated client list from the server
                                        int connc_res = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // for UDP socket protocol can be 0 or IPPROTO_UDP, for TCP it is 0 or IPPROTO_TCP (since requirement is UDP socket IPPROTO_UDP has been used)
                                        already_logged_in_user = 1;
                                        current_login = 1;
                                        if (connc_res < 0)
                                        {
                                            // printf("Connection failed!!\n");
                                            cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                            cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                            // return;
                                            // exit(EXIT_FAILURE);
                                        }
                                        else
                                        {

                                            struct sockaddr_in conn_addr;
                                            memset((char *)&conn_addr, 0, sizeof conn_addr);
                                            conn_addr.sin_family = AF_INET;                       // for IPv4
                                            inet_pton(AF_INET, "8.8.8.8", &(conn_addr.sin_addr)); // presentation to network bits conversion
                                            conn_addr.sin_port = htons(53);                       // for host to network conversion

                                            int result1 = connect(connc_res, (struct sockaddr *)&conn_addr, sizeof(conn_addr));
                                            int addlen = sizeof(conn_addr);
                                            int getsocktresult1 = getsockname(connc_res, (struct sockaddr *)&conn_addr, &(addlen));
                                            if (result1 < 0 || getsocktresult1 < 0)
                                            {
                                                printf("Error:Line--453\n");
                                                exit(EXIT_FAILURE);
                                            }
                                            char temp_buffer[120] = "";
                                            inet_ntop(AF_INET, &(conn_addr.sin_addr), store_ip, INET_ADDRSTRLEN); // network to presentation that is IP address string
                                            // itoa(server_port_no,temp_buffer,10);
                                            sprintf(temp_buffer, "%d", server_port_no);
                                            char *str3 = "REFRESHSTART";
                                            char final_string_buf[120] = "";
                                            strcat(final_string_buf, "REFRESHSTART");
                                            strcat(final_string_buf, "|");
                                            strcat(final_string_buf, temp_buffer);
                                            strcat(final_string_buf, "|");
                                            strcat(final_string_buf, store_ip);
                                            strcat(final_string_buf, "|");
                                            strcat(final_string_buf, argv[2]);
                                            strcat(final_string_buf, "\n");
                                            int l = strlen(final_string_buf);
                                            int send_to_server = send(s_socket, final_string_buf, l, 0);
                                            if (send_to_server < 0)
                                            {
                                                printf("SEND ERROR -LineNo:471!!!\n");
                                                exit(EXIT_FAILURE);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // printf("686 line\n");
                                        cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                        cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                        current_login = 0;
                                        // return;
                                    }
                                }
                            }
                        }
                        else if (strcmp("REFRESH\n", cmd) == 0)
                        {
                            int flag = 0;
                            for (int i = 0; i < itrcl; i++)
                            {
                                if (cl1[i].port_no == atoi(argv[2]))
                                {
                                    flag = 1;
                                    break;
                                }
                            }
                            if (flag == 1 && current_login == 1)
                            {
                                // //printf("REFRESHED!!!-%s\n",cl[0].host_name);
                                //  int connection_result;
                                char store_ip[INET_ADDRSTRLEN];
                                //    connection_result=connect(s_socket,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
                                // //creating connection using UDP socket inorder to get the updated client list from the server
                                //     int connc_res=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);//for UDP socket protocol can be 0 or IPPROTO_UDP, for TCP it is 0 or IPPROTO_TCP (since requirement is UDP socket IPPROTO_UDP has been used)

                                //     if(connc_res<0){
                                //        // printf("Connection failed!!\n");
                                //     cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                //     cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                //     //return;
                                //         //exit(EXIT_FAILURE);
                                //     }else{

                                struct sockaddr_in conn_addr;
                                memset((char *)&conn_addr, 0, sizeof conn_addr);
                                conn_addr.sin_family = AF_INET;                       // for IPv4
                                inet_pton(AF_INET, "8.8.8.8", &(conn_addr.sin_addr)); // presentation to network bits conversion
                                conn_addr.sin_port = htons(53);                       // for host to network conversion

                                // int result1=connect(connc_res,(struct sockaddr*)&conn_addr,sizeof(conn_addr));
                                // int addlen=sizeof(conn_addr);
                                // int getsocktresult1=getsockname(connc_res,(struct sockaddr*)&conn_addr,&(addlen));
                                //  if(result1<0 || getsocktresult1<0){
                                //    printf("Error:Line--453\n");
                                //    exit(EXIT_FAILURE);
                                //  }
                                char temp_buffer[120] = "";
                                inet_ntop(AF_INET, &(conn_addr.sin_addr), store_ip, INET_ADDRSTRLEN); // network to presentation that is IP address string
                                // itoa(server_port_no,temp_buffer,10);
                                sprintf(temp_buffer, "%d", server_port_no);
                                char *str3 = "REFRESHNOW";
                                char final_string_buf[120] = "";
                                strcat(final_string_buf, "REFRESHNOW");
                                strcat(final_string_buf, "|");
                                strcat(final_string_buf, temp_buffer);
                                strcat(final_string_buf, "|");
                                strcat(final_string_buf, store_ip);
                                strcat(final_string_buf, "|");
                                strcat(final_string_buf, argv[2]);
                                strcat(final_string_buf, "\n");
                                int l = strlen(final_string_buf);
                                int send_to_server = send(s_socket, final_string_buf, l, 0);
                                if (send_to_server < 0)
                                {
                                    // printf("SEND ERROR -LineNo:471!!!\n");
                                    exit(EXIT_FAILURE);
                                }
                                cse4589_print_and_log("[%s:SUCCESS]\n", "REFRESH");
                                cse4589_print_and_log("[%s:END]\n", "REFRESH");

                                //  int c2=1;
                                // for(int i=0;i<itrcl;i++){

                                //      if(cl1[i].id!=-1){
                                //     cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",c2,cl1[i].host_name,cl1[i].ip_address,cl1[i].port_no);
                                //     c2++;
                                //     }
                                // }

                                //}
                            }
                            else
                            {
                                if (current_login == 1)
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "REFRESH");
                                    cse4589_print_and_log("[%s:END]\n", "REFRESH");
                                }
                            }
                        }
                        else if (strcmp("LIST\n", cmd) == 0)
                        {
                            sortrecords_client();
                            if (current_login == 1)
                            {
                                cse4589_print_and_log("[%s:SUCCESS]\n", "LIST");
                                int c1 = 1;

                                for (int i = 0; i < itrcl; i++)
                                {
                                    if (cl1[i].id != -1)
                                    {
                                        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", c1, cl1[i].host_name, cl1[i].ip_address, cl1[i].port_no);
                                        c1++;
                                    }
                                }

                                cse4589_print_and_log("[%s:END]\n", "LIST");
                            }
                        }
                        else if (strcmp("EXIT\n", cmd) == 0)
                        {
                            char str_val[1000] = "";
                            strcat(str_val, "EXIT");
                            strcat(str_val, "|");
                            strcat(str_val, argv[2]);
                            int send_now = send(s_socket, str_val, strlen(str_val), 0);
                            if (send_now < 0)
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "EXIT");
                                cse4589_print_and_log("[%s:END]\n", "EXIT");
                            }
                            // current_login=0;
                            close(s_socket);
                            cse4589_print_and_log("[%s:SUCCESS]\n", "EXIT");
                            cse4589_print_and_log("[%s:END]\n", "EXIT");
                            exit(0);
                        }
                        else if (strcmp("BLOCK", logincmd) == 0)
                        {
                            logincmd = strtok(NULL, "\n");
                            int connc_res = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                            char store_ip[INET_ADDRSTRLEN];
                            struct sockaddr_in conn_addr;
                            memset((char *)&conn_addr, 0, sizeof conn_addr);
                            conn_addr.sin_family = AF_INET;                       // for IPv4
                            inet_pton(AF_INET, "8.8.8.8", &(conn_addr.sin_addr)); // presentation to network bits conversion
                            conn_addr.sin_port = htons(53);                       // for host to network conversion

                            int result1 = connect(connc_res, (struct sockaddr *)&conn_addr, sizeof(conn_addr));
                            int addlen = sizeof(conn_addr);
                            int getsocktresult1 = getsockname(connc_res, (struct sockaddr *)&conn_addr, &(addlen));
                            if (result1 < 0 || getsocktresult1 < 0)
                            {
                                printf("Error:Line--453\n");
                                exit(EXIT_FAILURE);
                            }

                            inet_ntop(AF_INET, &(conn_addr.sin_addr), store_ip, INET_ADDRSTRLEN);
                            char block_val[1000] = "";

                            strcat(block_val, "BLOCK");
                            strcat(block_val, "|");
                            strcat(block_val, store_ip);
                            strcat(block_val, "|");
                            strcat(block_val, argv[2]);
                            strcat(block_val, "|");
                            strcat(block_val, logincmd);

                            int ip_res = checkIP(logincmd);
                            int ip_val = ip_Exists_client(logincmd);
                            int b_pos = 2001;
                            for (int n = 0; n < lb; n++)
                            {
                                if (strcmp(local_blocked[n].ip_add, logincmd) == 0 && local_blocked[n].status == 1)
                                {
                                    b_pos = 2000;
                                    break;
                                }
                                else if (strcmp(local_blocked[n].ip_add, logincmd) == 0 && local_blocked[n].status == 0)
                                {
                                    b_pos = n;
                                    break;
                                }
                            }

                            if (ip_res != 0 && ip_val == 1 && b_pos != 2000)
                            {

                                int send_now = send(s_socket, block_val, strlen(block_val), 0);
                                if (send_now < 0)
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
                                    cse4589_print_and_log("[%s:END]\n", "BLOCK");
                                }
                                else
                                {
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCK");
                                    cse4589_print_and_log("[%s:END]\n", "BLOCK");
                                    if (b_pos == 2001)
                                    {
                                        strcpy(local_blocked[lb].ip_add, logincmd);
                                        local_blocked[lb].status = 1;
                                        lb++;
                                    }
                                    else if (b_pos != 2000)
                                    {
                                        local_blocked[lb].status = 1;
                                    }

                                    // for(int i=0;i<itrcl;i++){
                                    //    if(cl1[i].port_no==atoi(argv[2])){
                                    //        strcpy(cl1[i].clblockedlst.ip_address,logincmd);
                                    //    }
                                    //  }
                                }
                            }
                            else
                            {

                                cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
                                cse4589_print_and_log("[%s:END]\n", "BLOCK");
                            }
                        }
                        else if (strcmp("UNBLOCK", logincmd) == 0)
                        {
                            logincmd = strtok(NULL, "\n");
                            int connc_res = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                            char store_ip[INET_ADDRSTRLEN];
                            struct sockaddr_in conn_addr;
                            memset((char *)&conn_addr, 0, sizeof conn_addr);
                            conn_addr.sin_family = AF_INET;                       // for IPv4
                            inet_pton(AF_INET, "8.8.8.8", &(conn_addr.sin_addr)); // presentation to network bits conversion
                            conn_addr.sin_port = htons(53);                       // for host to network conversion

                            int result1 = connect(connc_res, (struct sockaddr *)&conn_addr, sizeof(conn_addr));
                            int addlen = sizeof(conn_addr);
                            int getsocktresult1 = getsockname(connc_res, (struct sockaddr *)&conn_addr, &(addlen));
                            if (result1 < 0 || getsocktresult1 < 0)
                            {
                                printf("Error:Line--453\n");
                                exit(EXIT_FAILURE);
                            }

                            inet_ntop(AF_INET, &(conn_addr.sin_addr), store_ip, INET_ADDRSTRLEN);
                            char unblock_val[1000] = "";
                            strcat(unblock_val, "UNBLOCK");
                            strcat(unblock_val, "|");
                            strcat(unblock_val, store_ip);
                            strcat(unblock_val, "|");
                            strcat(unblock_val, argv[2]);
                            strcat(unblock_val, "|");
                            strcat(unblock_val, logincmd);

                            int ip_res = checkIP(logincmd);
                            int ip_val = ip_Exists_client(logincmd);
                            int b_pos = 2000;
                            for (int n = 0; n < lb; n++)
                            {
                                if (strcmp(local_blocked[n].ip_add, logincmd) == 0 && local_blocked[n].status == 1)
                                {
                                    b_pos = n;
                                    break;
                                }
                            }
                            if (ip_res != 0 && ip_val == 1 && b_pos != 2000)
                            {
                                int send_now = send(s_socket, unblock_val, strlen(unblock_val), 0);
                                if (send_now < 0)
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
                                    cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
                                }
                                else
                                {
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "UNBLOCK");
                                    cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
                                    local_blocked[b_pos].status = 0;
                                    // for(int i=0;i<itrcl;i++){
                                    //    if(cl1[i].port_no==atoi(argv[2])){
                                    //        strcpy(cl1[i].clblockedlst.ip_address,logincmd);
                                    //    }
                                    //  }
                                }
                            }
                            else
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
                                cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
                            }
                        }
                        else if (strcmp("SEND", logincmd) == 0)
                        {
                            int c_comp = 1;
                            char client_msg[10000] = "";
                            char to_ip[1024] = "";
                            while (c_comp != 3)
                            {
                                c_comp++;
                                if (c_comp == 2)
                                {
                                    logincmd = strtok(NULL, " ");
                                    strcpy(to_ip, logincmd);
                                }
                                else if (c_comp == 3)
                                {
                                    logincmd = strtok(NULL, "\n");
                                    // if(strlen(logincmd)>256){
                                    //    strncpy(client_msg,logincmd+(0),256);
                                    // }else{

                                    strcpy(client_msg, logincmd);
                                    // }
                                }
                            }

                            char send_val[10000] = "";
                            strcat(send_val, "SEND");
                            strcat(send_val, "|");
                            strcat(send_val, to_ip);
                            strcat(send_val, "|");
                            client_msg[strlen(client_msg)] = '\0';
                            strcat(send_val, client_msg);
                            // printf("%d",strlen(client_msg));
                            // for(int h=0;h<strlen(client_msg);h++){
                            //   printf("%d ",client_msg[h]);
                            // }
                            int checkfromip = 0;
                            int ip_res = checkIP(to_ip);
                            int ip_val = ip_Exists_client(to_ip);
                            // for(int p=0;p<itrcl;p++){
                            //    if(cl1[p].port_no==atoi(argv[2])){
                            //         checkfromip=1;
                            //         break;
                            //    }
                            // }
                            // printf("ipres---%d,ip_val---%d",ip_res,ip_val);
                            if (ip_res != 0 && ip_val == 1 && current_login == 1 && strlen(client_msg) <= 256)
                            {
                                int send_now = send(s_socket, send_val, strlen(send_val), 0);
                                if (send_now < 0)
                                {
                                    // printf("val1----%d,val2------%d",ip_res,ip_val);
                                    cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                    cse4589_print_and_log("[%s:END]\n", "SEND");
                                }

                                else
                                {
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "SEND");
                                    cse4589_print_and_log("[%s:END]\n", "SEND");
                                    // count_messages(argv[2]);
                                }
                            }
                            else
                            {
                                // printf("val1----%d,val2------%d",ip_res,ip_val);
                                cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                cse4589_print_and_log("[%s:END]\n", "SEND");
                            }
                        }
                        else if (strcmp("BROADCAST", logincmd) == 0)
                        {

                            char broadcast_msg[1024] = "";
                            logincmd = strtok(NULL, "\n");
                            strcpy(broadcast_msg, logincmd);
                            char broad_val[1000] = "";
                            strcat(broad_val, "BROADCAST");
                            strcat(broad_val, "|");
                            strcat(broad_val, broadcast_msg);

                            int send_now = send(s_socket, broad_val, strlen(broad_val), 0);
                            if (send_now < 0 || current_login == 0)
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "BROADCAST");
                                cse4589_print_and_log("[%s:END]\n", "BROADCAST");
                            }
                            else
                            {
                                cse4589_print_and_log("[%s:SUCCESS]\n", "BROADCAST");
                                cse4589_print_and_log("[%s:END]\n", "BROADCAST");
                            }
                        }
                        else if (strcmp("LOGOUT\n", cmd) == 0)
                        {
                            if (current_login == 1)
                            {

                                for (int k = 0; k < itrcl; k++)
                                {
                                    if (cl1[k].port_no == atoi(argv[2]))
                                    {
                                        cl1[k].status = 0;
                                        current_login = 0;
                                        break;
                                    }
                                }
                                char logout_msg[1024] = "";
                                strcat(logout_msg, "LOGOUT");
                                strcat(logout_msg, "|");
                                strcat(logout_msg, argv[2]);
                                current_login = 0;
                                int send_now = send(s_socket, logout_msg, strlen(logout_msg), 0);
                                if (send_now < 0)
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "LOGOUT");
                                    cse4589_print_and_log("[%s:END]\n", "LOGOUT");
                                }
                                else
                                {
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "LOGOUT");
                                    cse4589_print_and_log("[%s:END]\n", "LOGOUT");
                                }
                            }
                        }

                        free(cmd);
                    }
                    else
                    { /* begin of the sample codes */
                        int num_bytes;
                        // printf("hello\n");
                        // char recieve_buffer[1024]="";
                        memset(recieve_buffer, '\0', MLENGTH);
                        if (sock_index == 0 && sock_index == serverc_socket)
                            break;
                        else
                        {
                            num_bytes = recv(sock_index, recieve_buffer, MLENGTH, 0);
                            if (num_bytes == -1)
                            {
                                close(sock_index);
                                // printf("Recieve Error!! --Line NO:524!\n");
                                exit(EXIT_FAILURE);
                            }
                            else
                            {
                                recieve_buffer[num_bytes] = '\0';
                                // printf("%s\n",recieve_buffer);
                                /* end of the sample codes */
                                char *buffer_copy = (char *)malloc(sizeof(char) * MLENGTH);
                                char *buffer_second_copy = (char *)malloc(sizeof(char) * MLENGTH);
                                strcpy(buffer_copy, recieve_buffer);
                                strcpy(buffer_second_copy, recieve_buffer);
                                // printf("%s\n",buffer_second_copy);
                                // seperating data based on the delimiters-----
                                char *refresh = strtok(buffer_second_copy, "|");
                                // printf("%s\n",refresh);
                                if (strcmp(refresh, "REFRESHSTART") == 0 || strcmp(refresh, "REFRESHNOW") == 0)
                                {

                                    char *each_row[400];
                                    int i = 0;
                                    if (strcmp(refresh, "REFRESHNOW") == 0)
                                    {
                                        itrcl = 0;
                                    }
                                    each_row[i] = strtok(buffer_copy, "\n");

                                    for (i = 0; each_row[i] != NULL;)
                                    {
                                        each_row[++i] = strtok(NULL, "\n");
                                    }
                                    int record = 0;
                                    while (record < i)
                                    {
                                        char *each_field = strtok(each_row[record], "|");
                                        int field = 0;
                                        while (each_field != NULL)
                                        {
                                            if (record == 0)
                                            {
                                                if (field == 0)
                                                {
                                                    field++;
                                                }
                                                else if (field == 1)
                                                {
                                                    cl1[itrcl].id = atoi(each_field);
                                                    field++;
                                                }
                                                else if (field == 2)
                                                {
                                                    strcpy(cl1[itrcl].host_name, each_field);
                                                    field++;
                                                }
                                                else if (field == 3)
                                                {
                                                    strcpy(cl1[itrcl].ip_address, each_field);
                                                    field++;
                                                }
                                                else if (field == 4)
                                                {
                                                    cl1[itrcl].port_no = atoi(each_field);
                                                    field++;
                                                }
                                            }
                                            else
                                            {

                                                if (field == 0)
                                                {
                                                    cl1[itrcl].id = atoi(each_field);
                                                    field++;
                                                }
                                                else if (field == 1)
                                                {
                                                    strcpy(cl1[itrcl].host_name, each_field);
                                                    field++;
                                                }
                                                else if (field == 2)
                                                {
                                                    strcpy(cl1[itrcl].ip_address, each_field);
                                                    field++;
                                                }
                                                else if (field == 3)
                                                {
                                                    cl1[itrcl].port_no = atoi(each_field);
                                                    field++;
                                                }
                                            }

                                            // printf("%s\n",each_field);
                                            each_field = strtok(NULL, "|");
                                        }
                                        record++;
                                        itrcl++;
                                    }

                                    // if(strcmp(refresh,"REFRESHNOW")==0){
                                    //     int c2=1;
                                    //     for(int i=0;i<itrcl;i++){

                                    //          if(cl1[i].id!=-1){
                                    //         cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",c2,cl1[i].host_name,cl1[i].ip_address,cl1[i].port_no);
                                    //         c2++;
                                    //         }
                                    //     }
                                    // }
                                }
                                else if (strcmp(refresh, "SEND") == 0)
                                {
                                    char msg_from[10000] = "";
                                    char msg[10000] = "";

                                    refresh = strtok(NULL, "|");
                                    strcpy(msg_from, refresh);
                                    refresh = strtok(NULL, "|");
                                    strcpy(msg, refresh);

                                    cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
                                    cse4589_print_and_log("msg from:%s\n[msg]:%s\n", msg_from, msg);
                                    cse4589_print_and_log("[RECEIVED:END]\n");
                                    fflush(stdout);
                                }
                                else if (strcmp(refresh, "BROAD") == 0)
                                {
                                    // char msg_from[1024]="";
                                    char msg[1024] = "";
                                    char fromip_add[1024] = "";

                                    refresh = strtok(NULL, "|");
                                    strcpy(msg, refresh);
                                    refresh = strtok(NULL, "|");
                                    strcpy(fromip_add, refresh);

                                    cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
                                    cse4589_print_and_log("msg from:%s\n[msg]:%s\n", fromip_add, msg);
                                    cse4589_print_and_log("[RECEIVED:END]\n");
                                }
                                else if (strcmp(refresh, "YES") == 0)
                                {
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
                                    cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                }
                                else if (strcmp(refresh, "SENDAGAIN") == 0)
                                {
                                    int trk = 1;
                                    char fromip[1024] = "";
                                    char to_ip[1024] = "";
                                    char msg[1024] = "";
                                    while (refresh != NULL)
                                    {

                                        if (trk == 1)
                                        {

                                            refresh = strtok(NULL, "|");
                                            if (refresh == NULL)
                                            {
                                                break;
                                            }

                                            strcpy(fromip, refresh);

                                            trk++;
                                        }
                                        else if (trk == 2)
                                        {

                                            refresh = strtok(NULL, "|");
                                            if (refresh == NULL)
                                            {
                                                break;
                                            }
                                            strcpy(msg, refresh);

                                            trk++;
                                        }
                                        else if (trk == 3)
                                        {

                                            refresh = strtok(NULL, "|");
                                            if (refresh == NULL)
                                            {
                                                break;
                                            }
                                            strcpy(to_ip, refresh);

                                            cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
                                            cse4589_print_and_log("msg from:%s\n[msg]:%s\n", fromip, msg);
                                            cse4589_print_and_log("[RECEIVED:END]\n");
                                            strcpy(fromip, "");
                                            strcpy(to_ip, "");
                                            strcpy(msg, "");
                                            trk = 1;
                                        }
                                    }

                                    cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
                                    cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                    char final_string_buf[120] = "";
                                    strcat(final_string_buf, "REFRESHNOW");
                                    strcat(final_string_buf, "|");
                                    int send_to_server = send(s_socket, final_string_buf, strlen(final_string_buf), 0);
                                    if (send_to_server < 0)
                                    {
                                        // printf("SEND ERROR -LineNo:471!!!\n");
                                        exit(EXIT_FAILURE);
                                    }
                                }
                                // else if(strcmp(refresh,"STATUSSEND")==0){
                                //     char msg[1024]="";

                                //             refresh=strtok(NULL,"|");
                                //             strcpy(msg,refresh);

                                //             if(strcmp(msg,"1")==0){
                                //  // cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                //  // cse4589_print_and_log("[%s:END]\n", "SEND");
                                //  //            }else{
                                // cse4589_print_and_log("[%s:SUCCESS]\n", "SEND");
                                //  cse4589_print_and_log("[%s:END]\n", "SEND");
                                //             }
                                // }
                            }
                        }

                        fflush(stdout);
                    }
                    /* Check if new client is requesting connection */
                    // else if(sock_index == serverc_socket){
                    //  caddr_len = sizeof(client_addr);
                    //  fdaccept = accept(serverc_socket, (struct sockaddr *)&client_addr, &caddr_len);
                    //  if(fdaccept < 0)
                    //      perror("Accept failed.");

                    //  printf("\nRemote Host connected!\n");

                    //  /* Add to watched socket list */
                    //  FD_SET(fdaccept, &master_list);
                    //  if(fdaccept > head_socket) head_socket = fdaccept;
                    // }
                    /* Read from existing clients */
                    // else{
                    //  /* Initialize buffer to receieve response */
                    //  char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                    //  memset(buffer, '\0', BUFFER_SIZE);

                    //  if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                    //      close(sock_index);
                    //      printf("Remote Host terminated connection!\n");

                    //      /* Remove from watched list */
                    //      FD_CLR(sock_index, &master_list);
                    //  }
                    //  else {
                    //      //Process incoming data from existing clients here ...

                    //      printf("\nClient sent me: %s\n", buffer);
                    //      printf("ECHOing it back to the remote host ... ");
                    //      if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
                    //          printf("Done!\n");
                    //      fflush(stdout);
                    //  }

                    //  free(buffer);
                    // }
                }
            }
        }
    }
}

/*References
  Demo code
  https://beej.us/guide/bgnet/html/split/index.html
  geeks for geeks for socket programming understanding
  -https://www.geeksforgeeks.org/socket-programming-cc/
*/
// int main(int argc, char **argv)
// {
//     if(argc != 3) {
//         printf("Usage:%s [port]\n", argv[0]);
//         //printf("%d",argc);
//         exit(-1);
//     }
// cse4589_init_log(argv[2]);
// /*Clear LOGFILE*/
// fclose(fopen(LOGFILE, "w"));

//     // struct loggeduserlist cl[5];
//     if(strcmp(argv[1],"s")==0)
//         {
//             startserver(argv);
//         }else{
//             //printf("Trigger client\n");
//             startclient(argv);
//         }
// int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
// struct sockaddr_in client_addr;
// struct addrinfo hints, *res;
// fd_set master_list, watch_list;

// /* Set up hints structure */
// memset(&hints, 0, sizeof(hints));
//  hints.ai_family = AF_INET;
//  hints.ai_socktype = SOCK_STREAM;
//  hints.ai_flags = AI_PASSIVE;

// /* Fill up address structures */
// if (getaddrinfo(NULL, argv[2], &hints, &res) != 0)
//  perror("getaddrinfo failed");

// /* Socket */
// server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
// if(server_socket < 0)
//  perror("Cannot create socket");

// /* Bind */
// if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 )
//  perror("Bind failed");

// freeaddrinfo(res);

// /* Listen */
// if(listen(server_socket, BACKLOG) < 0)
//  perror("Unable to listen on port");

// /* ---------------------------------------------------------------------------- */

// /* Zero select FD sets */
// FD_ZERO(&master_list);
// FD_ZERO(&watch_list);

// /* Register the listening socket */
// FD_SET(server_socket, &master_list);
// /* Register STDIN */
// FD_SET(STDIN, &master_list);

// head_socket = server_socket;

// while(TRUE){
//  memcpy(&watch_list, &master_list, sizeof(master_list));

//  //printf("\n[PA1-Server@CSE489/589]$ ");
//  //fflush(stdout);

//  /* select() system call. This will BLOCK */
//  selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
//  if(selret < 0)
//      perror("select failed.");

//  /* Check if we have sockets/STDIN to process */
//  if(selret > 0){
//      /* Loop through socket descriptors to check which ones are ready */
//      for(sock_index=0; sock_index<=head_socket; sock_index+=1){

//          if(FD_ISSET(sock_index, &watch_list)){

//              /* Check if new command on STDIN */
//              if (sock_index == STDIN){
//                  char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

//                  memset(cmd, '\0', CMD_SIZE);
//                  if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
//                      exit(-1);
//                  //int response=1;
//                  //Process PA1 commands here ...
//                  if(strcmp("IP\n",cmd)==0){
//                      char ip_addr[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN- denotes ipv4 since ip not specified explicitly,hence using ipv4.
//                      //for UDP socket protocol can be 0 or IPPROTO_UDP, for TCP it is 0 or IPPROTO_TCP (since requirement is UDP socket IPPROTO_UDP has been used)
//                         int udpsocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
//                         struct sockaddr_in address;
//                          if(udpsocket<0){
//                              printf("Creation of UDP socket failed!!\n");
//                              //exit(EXIT_FAILURE);
//                          }
//                           address.sin_family=AF_INET;
//                        //address.sin_addr.s_addr=INADDR_ANY;
//                        inet_pton(AF_INET,"8.8.8.8",&(address.sin_addr)); //presentation to network bits
//                        address.sin_port=htons(53);

//                          int connection_result=connect(udpsocket,(struct sockaddr*)&address,sizeof(address));
//                          int addlen=sizeof(address);
//                          int getsocktresult=getsockname(udpsocket,(struct sockaddr*)&address,&(addlen));
//                          inet_ntop(AF_INET,&(address.sin_addr),ip_addr,INET_ADDRSTRLEN); //network to presentation that is IP address string
//                          if(connection_result>=0 && getsocktresult>=0){
//                              // Successful
//                              printf("IP:%s\n",ip_addr);

//                          // cse4589_print_and_log("[%s:SUCCESS]\n", "IP");

//                          // cse4589_print_and_log("IP:%s\n", ip_addr);

//                          // cse4589_print_and_log("[%s:END]\n", "IP");
//                      }else{
//                          // cse4589_print_and_log("[%s:ERROR]\n", "IP");

//                          // cse4589_print_and_log("[%s:END]\n", "IP");
//                      }

//                  }else if(strcmp("AUTHOR\n",cmd)==0){
//                      // cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
//                      printf("I,%s, have read and understood the course academic integrity policy.\n",UBIT_NAME);
//                      // cse4589_print_and_log("[%s:END]\n", "AUTHOR");
//                  } else if(strcmp("PORT\n",cmd)==0){
//                      int arglength=sizeof(argv)/sizeof(argv[0]);
//                      if(arglength>0 && argv[2]){
//                           int port_val=atoi(argv[2]);
//                           // cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
//                           printf("PORT:%d\n",port_val);
//                           // cse4589_print_and_log("[%s:END]\n", "PORT");
//                      }else{
//                          printf("PORT-error\n");
//                          // cse4589_print_and_log("[%s:ERROR]\n", "PORT");

//                          // cse4589_print_and_log("[%s:END]\n", "PORT");
//                      }
//                  }

//                  free(cmd);
//              }
//              /* Check if new client is requesting connection */
//              else if(sock_index == server_socket){
//                  caddr_len = sizeof(client_addr);
//                  fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
//                  if(fdaccept < 0)
//                      perror("Accept failed.");

//                  printf("\nRemote Host connected!\n");

//                  /* Add to watched socket list */
//                  FD_SET(fdaccept, &master_list);
//                  if(fdaccept > head_socket) head_socket = fdaccept;
//              }
//              /* Read from existing clients */
//              else{
//                  /* Initialize buffer to receieve response */
//                  char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
//                  memset(buffer, '\0', BUFFER_SIZE);

//                  if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
//                      close(sock_index);
//                      printf("Remote Host terminated connection!\n");

//                      /* Remove from watched list */
//                      FD_CLR(sock_index, &master_list);
//                  }
//                  else {
//                      //Process incoming data from existing clients here ...

//                      printf("\nClient sent me: %s\n", buffer);
//                      printf("ECHOing it back to the remote host ... ");
//                      if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
//                          printf("Done!\n");
//                      fflush(stdout);
//                  }

//                  free(buffer);
//              }
//          }
//      }
//  }
// }

//     return 0;
// }
