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
 * Fichero: protocol.h
 * Versión: 4.1
 * Curso: 2026/2027
 * Descripción: Fichero de encabezado para práctica 3
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * PONGA SU NOMBRE AQUÍ:
 * Estudiante 1: Sara Núñez Torres
 * Estudiante 2 [si lo hubiera]:
 *
 ******************************************************/

#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile

 // APPLICATION COMMANDS
#define HELO "HELO"
#define MAIL "MAIL FROM:"
#define RCPT "RCPT TO:"
#define DATA "DATA"
#define QUIT "QUIT"
#define ECHO "ECHO"
#define ROOM "ROOM"

// SMTP SERVER RESPONSES
#define ST220 "220"
#define ST221 "221"
#define ST250 "250"
#define ST354 "354"

// APPLICATION RESPONSES
#define OK "OK"
#define ER "ER"
#define NA "NA"

// END OF RESPONSE
#define CRLF "\r\n"

// STATES
#define S_INIT 0
#define S_HELO 1
#define S_MAIL 2
#define S_RCPT 3
#define S_DATA 4
#define S_WRITE 5
#define S_QUIT 6

// SERVICE PORT
#define TCP_SERVICE_PORT 25

// AUTHORISED USER AND PASSWORD
#define TEST_USER "user"
#define TEST_PASSWORD "1234"

#endif
