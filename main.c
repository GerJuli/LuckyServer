

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

#include "sri.h" //StatLib has to be added to the Path


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

int* conditions_from_sock(int client_socket)

{
	char echo_buffer[RCVBUFSIZE];
	int recv_size;
	static int conditions[2];

	if ((recv_size = recv(client_socket, echo_buffer, RCVBUFSIZE, 0)) < 0)
		error_exit("Fehler bei recv()");

	//Check if message complete(check for Brackets)
	int len_message = strlen(echo_buffer);
	if (echo_buffer[0] != '[' || echo_buffer[len_message - 1] != ']') {
		printf("Message not in right format: %s\n", echo_buffer);
		return -1;
	}

	char *substr = malloc(len_message - 1);
	strncpy(substr, echo_buffer + 1, len_message - 2);
	substr[len_message - 2] = '\0';

	char delim[] = ",";

	char *ptr = strtok(substr, delim);

	conditions[0] = atoi(ptr);
	ptr = strtok(NULL, delim); //changing pointer position to next delimiter
	conditions[1] = atoi(ptr);
	printf("Message : %s\n", echo_buffer);
	return conditions;
}


void* server_x(void *param) {

	cond_pair *pointer_to_conditions = (cond_pair*) param;
	condition *p_primary_cond = (condition*) &pointer_to_conditions->primary;
	condition *p_secondary_cond = (condition*) &pointer_to_conditions->secondary;

	struct sockaddr_in server, client;

	int sock, fd;

	int len;

	/* Create the Socket. */
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
		error_exit("Fehler beim Anlegen eines Sockets");

	/* Erzeuge die Socketadresse des Servers. */
	memset(&server, 0, sizeof(server));
	/* IPv4-Verbindung */
	server.sin_family = AF_INET;
	/* INADDR_ANY: jede IP-Adresse annehmen */
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
	/* Bearbeite die Verbindungswünsche von Clients
	 * in einer Endlosschleife.
	 * Der Aufruf von accept() blockiert so lange,
	 * bis ein Client Verbindung aufnimmt. */
	while (1) {
		//printf("Server:\n");
		len = sizeof(client);
		fd = accept(sock, (struct sockaddr*) &client, &len);
		if (fd < 0)
			error_exit("Error to accept");
		//printf("Client with Adress: %s\n", inet_ntoa(client.sin_addr));
		/* Daten vom Client auf dem Bildschirm ausgeben */

		echo(fd);

		int *recieved_conds;
		recieved_conds = conditions_from_sock(fd);
		int target_phase = recieved_conds[0];
		int threshold = recieved_conds[1];
		//Writes data to the primary condition
		pthread_mutex_lock(&(p_primary_cond->mutx));
		p_primary_cond->target_phase = target_phase;
		p_primary_cond->threshold = threshold;
		usleep(5000000);
		pthread_mutex_unlock(&(p_primary_cond->mutx));

		//Writes data to the secondary condition
		pthread_mutex_lock(&(p_secondary_cond->mutx));
		p_secondary_cond->target_phase = target_phase;
		p_secondary_cond->threshold = threshold;
		pthread_mutex_unlock(&(p_secondary_cond->mutx));
		//pointer_to_condition->target_phase = number_from_sock(fd);

		/* Close connection. */
		close(fd);
	}

	return EXIT_SUCCESS;
}

int main(){
	condition primary_cond = { 0, 0 };
	condition secondary_cond = { 0, 0 };
	cond_pair cond_shared = { primary_cond, secondary_cond };
	pthread_mutex_init(&cond_shared.primary.mutx, NULL);
	pthread_mutex_init(&cond_shared.secondary.mutx, NULL);
	server_x(&cond_shared);



	int same = compare_conditions(standard, standard);
	if (same)
		printf("It is the same Condition: \n");
	else{
		printf("It isn't the same Condition: \n");
	}

	return 0;
}
