/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Escuela Politécnia Superior de Linares
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 1
 *******************************************************
 * Fichero: cliente.c
 * Versión: 3.3
 * Curso: 2026/2027
 * Descripción: Cliente sencillo TCP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * PONGA SU NOMBRE AQUÍ:
 * Estudiante 1: Sara Núñez Torres
 * Estudiante 2 [si lo hubiera]:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <conio.h>		// Biblioteca de entrada salida básica
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.
#include "protocol.h"	// Declarar constantes y funciones de la práctica

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib

 //Check if the day exists in the month selected
int days_in_month(int month) { 
	if (month == 2) //28 days on February
		return 28;
	else if (month == 4 || month == 6 || month == 9 || month == 11) //April, June, September and November have 30 days
		return 30;
	else
		return 31; //The remaining months have 31 days
}

int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];
	int received = 0, sent = 0;
	int status;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16] = "127.0.0.1";
	char default_ip6[64] = "::1";

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicialización de idioma
	setlocale(LC_ALL, "es_ES.UTF8");

	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");

	do {
		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			//Si se introduce 6 se empleará IPv6
			ipversion = AF_INET6;
		}
		else { //Distinto de 6 se elige la versión IPv4
			ipversion = AF_INET;
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0); 
		//SOCKET: Creates a socket and links it to a service provider.
		//SOCKET: The first parameter "ipversion" indicates the address family defined previously.
		//SOCKET:  AF_INET is used for IPv4 and AF_INET6 for IPv6.
		//SOCKET: The second parameter "SOCK_STREAM" specifies the type of the new socket.
		//SOCKET: In this case, it is a connection-oriented socket used by TCP.
		//SOCKET: The last parameter indicates the protocol that will be used for the communication.
		//SOCKET: The value 0 indicates that the default protocol for the family will be used.
		//SOCKET: If the operation is successful, it returns a valid socket descriptor that is stored in "sockfd".
		//SOCKET: If an error occurs, it returns INVALID_SOCKET (-1).

		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR: %d\r\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		else {
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*) & server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*) & server_in6;
				address_size = sizeof(server_in6);
			}

			//Cada nueva conexión establece el estado incial en
			status = S_INIT;

			if (connect(sockfd, server_in, address_size) == 0) {
			//SOCKET: Requests to establish a connection with a remote machine.
			//SOCKET: The first parameter corresponds to the descriptor of the created socket, stored in "sockfd".
			//SOCKET: The second parameter "server_in" refers to the protocol address of the destination socket.
			//SOCKET: The third parameter "address_size" specifies the size in bytes of the server address structure.
			//SOCKET: Returns 0 if the operation was successful.
			//SOCKET: In case of an error, it returns SOCKET_ERROR (-1).

				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la máquina de estados
				do {
					switch (status) {
					case S_INIT:
						// Se recibe el mensaje de bienvenida
						break;
					case S_HELO:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el dominio (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							status = S_QUIT;
						}
						else {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HELO, input, CRLF);
						}
						break;
					case S_MAIL:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							status = S_QUIT;
						}
						else
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", PASS, input, CRLF);
						break;

					case S_DATA:
						printf("Estancias disponibles: \r\n- Sala de juntas(cw00).\r\n- Sala coworking grande norte(cw01).\r\n- Sala coworking grande sur(cw02). \r\n- Sala coworking junior(cw03). \r\n");
						printf("CLIENTE> Introduzca código de estancia (enter para salir): ");
						gets_s(input, sizeof(input));

						// Validate room code
						while (strlen(input) != 4 && strlen(input) != 0) { // Checks if it has exactly 4 characters and is not empty
							printf("CLIENTE> ERROR: El código de estancia debe tener exactamente 4 caracteres.\r\n");
							printf("CLIENTE> Introduzca código de estancia (enter para salir): ");
							gets_s(input, sizeof(input));
						}

						if (strlen(input) == 0) { //Press enter to exit
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							status = S_QUIT;
						}
						else {
							unsigned int dayint;
							unsigned int monthint;

							// Validate day and month
							do {
								printf("CLIENTE> Introduzca el día: ");
								scanf_s("%u", &dayint);

								printf("CLIENTE> Introduzca el mes: ");
								scanf_s("%u", &monthint);
								
								if (monthint < 1 || monthint > 12 || dayint < 1 || dayint > days_in_month(monthint)) {
									printf("CLIENTE> ERROR: Día o mes inválido.\r\n");
								}

								while (getchar() != '\n'); // Clears the input buffer after reading the data.

							} while (monthint < 1 || monthint > 12 || dayint < 1 || dayint > days_in_month(monthint));

							// Generate ROOM message
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %u %u%s", ROOM, input, dayint, monthint, CRLF);
						}
						//Temporary code to test ERROR 1
						/*sprintf_s(buffer_out, sizeof(buffer_out), "HELLO%s", CRLF);
						status = S_QUIT;*/

						//Temporary code to test ERROR 2
						/*sprintf_s(buffer_out, sizeof(buffer_out), "ROOM cw00 20%s", CRLF);
						status = S_QUIT;*/

						//Temporary code to test ERROR 3
						/*sprintf_s(buffer_out, sizeof(buffer_out), "ROOM cw00 32 2%s", CRLF);
						status = S_QUIT;*/

						break;
					}

					if (status != S_INIT) {
						sent = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						//SOCKET: Sending data using TCP.
						//SOCKET: The first parameter "sockfd" identifies the local socket used to send the data.
						//SOCKET: The second parameter "buffer_out" contains the data to be sent.
						//SOCKET: The third parameter indicates the size in bytes of the data to be sent.
						//SOCKET: The last parameter modifies the behaviour of the call.
						//SOCKET: By using the value 0, the normal operation of the protocol is used.
						//SOCKET: Returns the number of bytes sent if the operation is successful.
						//SOCKET: Returns 0 if the connection has been closed in an agreed manner.
						//SOCKET: If the operation fails, it returns SOCKET_ERROR (-1).

						if (sent == SOCKET_ERROR) {
							status = S_QUIT;
							continue;// La sentencia continue hace que la ejecución dentro de un
									 // bucle salte hasta la comprobación del mismo.
						}
					}

					received = recv(sockfd, buffer_in, 512, 0);
					//SOCKET: Receiving data using TCP.
					//SOCKET: The first parameter "sockfd" identifies the local socket used to receive the data.
					//SOCKET: The second parameter "buffer_in" is the buffer where the received data is stored.
					//SOCKET: The third parameter indicates the size in bytes of the reception buffer (the data cannot exceed this size).
					//SOCKET: The last parameter modifies the behaviour of the function.
					//SOCKET: The value 0 indicates normal operation, where all available data
					//SOCKET: in the TCP buffer is copied into "buffer_in" without exceeding the limit imposed by the buffer.
					//SOCKET: The data read is removed from the buffer.
					//SOCKET: Returns the number of bytes received if the operation is successful.
					//SOCKET: Returns 0 if the connection has been closed orderly by the remote end.
					//SOCKET: If the operation fails, it returns SOCKET_ERROR (-1).

					if (received <= 0) {
						DWORD error = GetLastError();
						if (received < 0) {
							printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
							status = S_QUIT;
						}
						else {
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							status = S_QUIT;
						}
					}
					else {
						buffer_in[received] = 0x00;
						printf(buffer_in);

						// Motor de transición de estados
						switch (status) {
							case S_INIT:
								if (strcmp(buffer_in, ST220, 3) == 0) {
									status = S_HELO;
								}
								break;
							case S_HELO:
								if (strcmp(buffer_in, ST250, 3) == 0) { //Mirar qué codigo de estado necesita según RFC 5321
									status = S_MAIL;
								}
								break;
							case S_MAIL:

						}
						if (status != S_DATA && strncmp(buffer_in, OK, 2) == 0){
							status++;
						}
						//Si la autenticación no es correcta se vuelve al estado S_USER
						if (status == S_PASS && strncmp(buffer_in, OK, 2) != 0) {
							status = S_HELO;
						}
					}

				} while (status != S_QUIT);
			}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			
			if (closesocket(sockfd) == SOCKET_ERROR) {
				//SOCKET: Closes the socket and frees the resources associated with it
				//SOCKET: The parameter "sockfd" specifies the socket descriptor to be closed.
				//SOCKET: Returns 0 if the socket has been closed successfully.
				//SOCKET: Returns SOCKET_ERROR (-1) if the operation fails.

				printf("CLIENTE> ERROR AL CERRAR EL SOCKET\r\n");
			}

		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	return(0);
}
/*
 * AI TOOL USAGE
 *
 * Tool: ChatGPT (GPT-5.6 Luna)
 *
 * Query 1:
 * "I have written comments in English to explain the use of socket primitives in this practical assignment. 
 * Please review them and check for grammatical mistakes or suggest more appropriate terms."
 *
 * Result:
 * A detailed review of the English wording and improvements to the comments, using appropriate terminology 
 * and expressions.
 *
 * Query 2:
 * "I have declared two variables as 'unsigned int dayint' and 'unsigned int monthint'. When printing their 
 * values, should I use %u? Explain why %u is used instead of %i or %d."
 *
 * Result:
 * The %u format specifier should be used because both variables are declared as unsigned int.
 *
 * Example:
 * unsigned int dayint;
 * unsigned int monthint;
 *
 * printf("Day: %u\n", dayint);
 * printf("Month: %u\n", monthint);
 *
 * Explanation:
 * - %d is used to print an int (signed integer).
 * - %i is also used to print an int. When used with scanf, it can interpret input numbers differently from %d.
 * - %u is used to print an unsigned int (unsigned integer).
 *
 * Therefore, %u is the appropriate format specifier for these variables. The same specifier should be used when
 * reading them with scanf_s().
 *
 * Query 3:
 * "What is the purpose of the gets_s(input, sizeof(input)) function in C? Explain how it works and provide an 
 * example of how to use it to read a room code entered by the user."
 *
 * Result:
 * The gets_s() function reads a string entered by the user from the keyboard, including spaces, until the 
 * Enter key is pressed. The second argument, sizeof(input), specifies the size of the buffer, allowing 
 * the function to limit the number of characters stored and helping prevent writing beyond the buffer's bounds.
 *
 * Example:
 * char input[100];
 *
 * printf("Enter the room code: ");
 * gets_s(input, sizeof(input));
 *
 * In this example, the user can enter a room code, such as "cw00", which is then stored in the input variable.
 */