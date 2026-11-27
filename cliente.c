/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Escuela Politécnia Superior de Linares
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 3
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
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib

int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];
	char sender[256]; //For the sender's email address
	int received = 0, sent = 0;
	int status;
	char option;
	char subject[1024];
	char from[1024];
	char to[1024];
	int headers = 0;
	int sending = 0;
	int recipient_count = 0; //Number of recipients

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
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*)&server_in6;
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
						// Receive the welcome message from the server
						break;

					case S_HELO:
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
						printf("CLIENTE> Introduzca la direccion del remitente (enter para salir): ");
						gets_s(input, sizeof(input));

						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
							status = S_QUIT;
						}
						else {
							strcpy_s(sender, sizeof(sender), input);
							recipient_count = 0;

							sprintf_s(buffer_out, sizeof(buffer_out), "%s<%s>%s", MAIL, sender, CRLF);
							//To sent de mail command with the following format: "MAIL FROM:<sender_email>\r\n"
						}
						break;

					case S_RCPT:
						printf("CLIENTE> Introduzca un destinatario (enter para finalizar): ");
						gets_s(input, sizeof(input));

						if (strlen(input) == 0) {
							if (recipient_count > 0) {
								printf("CLIENTE> ¿Desea reiniciar el proceso de envio? (S/N): ");
								option = _getche();

								if (option == 's' || option == 'S') {
									status = S_RSET;
								}
								else {
									status = S_DATA;
								}

								continue;
							}
							else {
								printf("CLIENTE> Debe introducir al menos un destinatario\r\n");
								continue;
							}
						}
						else {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s<%s>%s", RCPT, input, CRLF);
						}
						break;

					case S_DATA:
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", DATA, CRLF);
						break;

					case S_WRITE:
						sending = 1;

						/* Task 3: Sending an email in a single line
						* sprintf_s(buffer_out, sizeof(buffer_out), "Subject: Práctica 1%s%s%s%s.%s", CRLF, CRLF, "Sara Núñez Torres", CRLF, CRLF);
						*
						* The previous line is removed and replaced with the following code to allow the user to enter the email subject, 
						* sender and recipient, as well as the message body, separately.
						*/

						// Headers are sent only once, at the beginning of the message. After that, only the body lines are sent.
						if (headers == 0) {

							do {
								printf("CLIENTE> Introduzca el asunto: ");
								gets_s(subject, sizeof(subject));

								if (strlen(subject) + strlen("Subject: ") > 998) { 
									// Check if the subject line exceeds the maximum length of 998 characters (1000 characters with CRLF), including the "Subject: " prefix.
									printf("CLIENTE> El asunto es demasiado largo. Intentelo de nuevo.\r\n");
								}
							} while (strlen(subject) + strlen("Subject: ") > 998);

							do {
								printf("CLIENTE> Introduzca el nombre del remitente: ");
								gets_s(from, sizeof(from));

								if (strlen(from) + strlen("From: ") > 998) {
									printf("CLIENTE> El remitente es demasiado largo. Intentelo de nuevo.\r\n");
								}
							} while (strlen(from) + strlen("From: ") > 998);

							do {
								printf("CLIENTE> Introduzca el nombre del destinatario: ");
								gets_s(to, sizeof(to));

								if (strlen(to) + strlen("To: ") > 998) {
									printf("CLIENTE> El destinatario es demasiado largo. Intentelo de nuevo.\r\n");
								}
							} while (strlen(to) + strlen("To: ") > 998);

							// Check whether all headers fit in the output buffer.
							if (strlen(subject) + strlen(from) + strlen(to) + strlen("Subject: ") + strlen("From: ") + strlen("To: ") + 4 * strlen(CRLF) >= sizeof(buffer_out)) {
								printf("CLIENTE> Las cabeceras ocupan demasiado espacio. Intentelo de nuevo.\r\n");
								headers = 0;
								continue;
							}

							sprintf_s(buffer_out, sizeof(buffer_out), "Subject: %s%sFrom: %s%sTo: %s%s%s", subject, CRLF, from, CRLF, to, CRLF, CRLF);

							headers = 1;
						}
						else {
							printf("CLIENTE> Introduzca una linea del mensaje (punto para terminar): ");
							gets_s(input, sizeof(input));

							if (strlen(input) > 998) {
								printf("CLIENTE> La linea es demasiado larga. Intentelo de nuevo.\r\n");
								continue;
							}

							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);

							// Check if the line is a single dot, which indicates the end of the message body in SMTP.
							if (strcmp(input, ".") == 0) {
								sending = 0;
							}
						}
						break;

					case S_RSET:
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RSET, CRLF);
						break;
					case S_QUIT:
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", QUIT, CRLF);
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
							status = S_EXIT;
							continue;// La sentencia continue hace que la ejecución dentro de un
							// bucle salte hasta la comprobación del mismo.
						}
					}

					if (sending == 0) {
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
							}
							else {
								printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							}
							status = S_EXIT;
						}
						else {
							buffer_in[received] = 0x00;
							printf(buffer_in);

							// State transition engine
							switch (status) {
							case S_INIT:
								if (strncmp(buffer_in, ST220, 3) == 0) {
									status = S_HELO;
								}
								else if (strncmp(buffer_in, ST554, 3) == 0) {
									printf("CLIENTE> Error: Servicio no disponible\r\n");
									status = S_QUIT;
								}
								else {
									printf("CLIENTE> Error en el mensaje de bienvenida\r\n");
									status = S_QUIT;
								}
								break;

							case S_HELO:
								if (strncmp(buffer_in, ST250, 3) == 0) {
									status = S_MAIL;
								}
								else if (strncmp(buffer_in, ST504, 3) == 0) {
									printf("CLIENTE> Error: Comando HELO no soportado\r\n");
									status = S_QUIT;
								}
								else if (strncmp(buffer_in, ST550, 3) == 0) {
									printf("CLIENTE> Error: Comando HELO rechazado por el servidor\r\n");
									status = S_QUIT;
								}
								else {
									printf("CLIENTE> Error al ejecutar HELO\r\n");
									status = S_QUIT;
								}
								break;
							case S_MAIL:
								if (strncmp(buffer_in, ST250, 3) == 0) {
									status = S_RCPT;
								}
								else if (strncmp(buffer_in, ST550, 3) == 0) {
									printf("CLIENTE> Error: Remitente rechazado. Buzón no disponible\r\n");
									status = S_MAIL;
								}
								else if (strncmp(buffer_in, ST503, 3) == 0) {
									printf("CLIENTE> Error: Sequencia de comandos SMTP incorrecta\r\n");
									status = S_QUIT;
								}
								else if (strncmp(buffer_in, ST451, 3) == 0) {
									printf("CLIENTE> Error temporal de procesamiento\r\n");
									status = S_MAIL;
								}
								else {
									printf("CLIENTE> El servidor ha rechazado el remitente\r\n");
									status = S_MAIL;
								}
								break;

							case S_RCPT:
								if (strncmp(buffer_in, ST250, 3) == 0 || strncmp(buffer_in, ST251, 3) == 0) {
									// If the server responds with 250 or 251 (user not local), it means the recipient is accepted.
									recipient_count++;
									status = S_RCPT;
								} 
								else if (strncmp(buffer_in, ST550, 3) == 0) {
									printf("CLIENTE> Error: Destinatario rechazado. Buzón no disponible\r\n");
									status = S_RCPT;
								}
								else if (strncmp(buffer_in, ST553, 3) == 0) {
									printf("CLIENTE> Error: Destinatario rechazado. Dirección no válida\r\n");
									status = S_RCPT;
								}
								else if (strncmp(buffer_in, ST450, 3) == 0) {
									printf("CLIENTE> Error: Buzón temporalmente no disponible\r\n");
									status = S_RCPT;
								}
								else {
									printf("CLIENTE> El servidor ha rechazado el destinatario\r\n");
									status = S_RCPT;
								}
								break;

							case S_DATA:
								if (strncmp(buffer_in, ST354, 3) == 0) {
									status = S_WRITE;
								}
								else if (strncmp(buffer_in, ST503, 3) == 0) {
									printf("CLIENTE> Error: Sequencia de comandos SMTP incorrecta\r\n");
									status = S_QUIT;
								}
								else {
									printf("CLIENTE> El servidor ha rechazado el comando DATA\r\n");
									status = S_QUIT;
								}
								break;

							case S_WRITE:
								if (strncmp(buffer_in, ST250, 3) == 0) {
									headers = 0;

									printf("CLIENTE> ¿Quiere enviar otro correo? (S para otro o cualquier tecla para salir): \r\n");
									option = _getche(); 

									if (option == 's' || option == 'S') {
										status = S_MAIL;
									}
									else {
										status = S_QUIT;
									}
								}
								else if (strncmp(buffer_in, ST552, 3) == 0) {
									printf("CLIENTE> Error: Mensaje rechazado por exceder el limite de almacenamiento o tamaño permitido\r\n");
									status = S_QUIT;
								}
								else {
									printf("CLIENTE> Mensaje rechazado por el servidor\r\n");
									status = S_QUIT;
								}
								break;

							case S_RSET:
								if (strncmp(buffer_in, ST250, 3) == 0) {
									printf("CLIENTE> Transaccion reiniciada correctamente\r\n");
									status = S_MAIL;
									recipient_count = 0; // Reset the recipient count when the transaction is reset
								}
								else {
									printf("CLIENTE> Error al reiniciar la transaccion\r\n");
									status = S_QUIT;
								}
								break;

							case S_QUIT:
								if (strncmp(buffer_in, ST221, 3) == 0) {
									printf("CLIENTE> Sesion SMTP cerrada correctamente\r\n");
								}
								else {
									printf("CLIENTE> Error al cerrar la sesion SMTP\r\n");
								}
								status = S_EXIT;
								break;
							}
						}
					}
				} while (status != S_EXIT);
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
 * "How can I verify that the emails sent by my SMTP client have actually been received by the intended recipients, and how can I use a local email client such as Thunderbird to check the delivered messages?"
 *
 * Result:
 * An explanation of how to configure a local email client such as Thunderbird
 * to access the mailboxes configured in ArGoSoft Mail Server and verify
 * that the messages sent by the SMTP client have been delivered and can
 * be retrieved by the intended recipients.  
 *
 * Query 3:
 * "Explain the difference between an SMTP server accepting a message for delivery with a 250 response and the message actually being delivered to the recipient's mailbox."
 *
 * Result:
 * An explanation of the meaning of the SMTP 250 response after message
 * transmission, clarifying that accepting a message for delivery does not
 * necessarily mean that it has already been retrieved or read by the
 * recipient, and describing the distinction between message acceptance
 * and final delivery.
 *
 * Query 4:
 * "Explain how the SMTP RSET command works and how it can be used to restart a mail transaction without closing the SMTP connection."
 *
 * Result:
 * An explanation of how the RSET command resets the current SMTP mail
 * transaction, discarding the transaction's current information and
 * allowing the client to start a new mail transaction within the same
 * SMTP session, without establishing a new TCP connection.
 * 
 */