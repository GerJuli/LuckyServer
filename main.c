#include <pthread.h>

/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

/* Headerfiles für UNIX/Linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

/* Portnummer */
#define PORT 1234

/* Puffer für eingehende Nachrichten */
#define RCVBUFSIZE 1024



#include <stdio.h>

// Inter process communication/Shared memory
#include <sys/ipc.h>
#include <sys/shm.h>

#include <sri.h>




static void echo(int);

static void error_exit(char *errorMessage);

/* Die Funktion gibt den aufgetretenen Fehler aus und
 * beendet die Anwendung. */

static void error_exit(char *error_message) {
	fprintf(stderr, "%s: %s\n", error_message, strerror(errno));
	exit(EXIT_FAILURE);
}


static void echo(int client_socket)

{
	char echo_buffer[RCVBUFSIZE];
	int recv_size;
	time_t zeit;

	if ((recv_size = recv(client_socket, echo_buffer, RCVBUFSIZE, 0)) < 0)
		error_exit("Fehler bei recv()");
	echo_buffer[recv_size] = '\0';
	time(&zeit);
	printf("Message from Client : %s \t%s", echo_buffer, ctime(&zeit));
}

int condition_from_sock(int client_socket, condition *receive_cond)
/* This function reads the socket and converts the input into a condition.
 * This condition will be written in the given condition.
 *
 * it will only work while only full condition parameters are send to the socket
 *
 *  Params:
 *  	int client_socket:
 *  		The socket where that data will be received from. This is the return type of accept()
 *  	condition *receive_cond:
 *  		In this pointer to a condition in that the new condition will be written in
 *  return:
 *  	int:
 *  		EXIT_SUCCESS or error code
 *
 *
 */

{
	char echo_buffer[RCVBUFSIZE];
	int recv_size;

	if ((recv_size = recv(client_socket, echo_buffer, RCVBUFSIZE, 0)) < 0)
		error_exit("Fehler bei recv()");
	//Check if message complete(check for Brackets)
	int len_message = strlen(echo_buffer);
	if (echo_buffer[0] != '[' || echo_buffer[len_message - 1] != ']') {
		printf("Message not in right format! Message:'%s'\n", echo_buffer);
		error_exit("Message not in right format");
	}
	else{
		printf("Message; '%s'\n", echo_buffer);
	}
	char *substr = malloc(len_message - 1);
	strncpy(substr, echo_buffer + 1, len_message - 2);
	substr[len_message - 2] = '\0';

	char delim[] = ",";

	char *ptr = strtok(substr, delim);
	int threshold = atoi(ptr);
	ptr = strtok(NULL, delim); //changing pointer position to next delimiter
	int target_phase = atoi(ptr);
	receive_cond->threshold = threshold;
	receive_cond->target_phase = target_phase;
	return EXIT_SUCCESS;
}


void* server_x(void *param) {
	// ftok to generate unique key
	key_t key = ftok("shmfile",65);

	// shmget returns an identifier in shmid
	int shmid = shmget(key,shm_size,0666|IPC_CREAT);

	cond_pair *conds = (cond_pair*) shmat(shmid,(void*)0,0);
	*conds = *(cond_pair*)param;
	condition *primary_cond = (condition*) &conds->primary;
	condition *secondary_cond = (condition*) &conds->secondary;
	printf("[Loop] Start prim cond: "); print_condition(*primary_cond);

	struct sockaddr_in server, client;
	int sock, fd;

	int len;

	/* Create the Socket. */
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
		error_exit("Error during creation of a socket");

	/* Create socket address of server */
	memset(&server, 0, sizeof(server));
	/* IPv4-Connection */
	server.sin_family = AF_INET;
	/* INADDR_ANY: accept any IP-address */
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Portnummer */
	server.sin_port = htons(PORT);

	/* Erzeuge die Bindung an die Serveradresse
	 * (genauer: an einen bestimmten Port). */
	if (bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0)
		error_exit("Kann das Socket nicht \"binden\"");

	/* Teile dem Socket mit, dass Verbindungswünsche
	 * von Clients entgegengenommen werden. 5 steht für Nummer der aktzeptierten Verbindungen */
	if (listen(sock, 5) == -1)
		error_exit("Fehler bei listen");

	printf("Server ready - waiting for messages ...\n");
	/* Here we work of connections from clients in an endless loop.
	 * TODO: Shutdown command by the client
	 * The call of accept() blocks the program until a client connects
	 * */
	while (1) {
		len = sizeof(client);
		fd = accept(sock, (struct sockaddr*) &client, &len);
		if (fd < 0)
			error_exit("Error to accept");
		//printf("Client with Adress: %s\n", inet_ntoa(client.sin_addr));
		/* Daten vom Client auf dem Bildschirm ausgeben */

		//echo(fd);
		static condition receive_cond;
		condition_from_sock(fd, &receive_cond);
		//Writes data to the primary condition
		*primary_cond = receive_cond;
		printf("[Server] Primary condition: "); print_condition(*primary_cond);

		//Writes data to the secondary condition
		*secondary_cond = receive_cond;

		/* Close connection. */
		close(fd);
	}

	return EXIT_SUCCESS;
}

int main(){

	server_x(&standard_cond_pair);

	return 0;
}
