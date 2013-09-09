/******************************************************************************
* *
* LISTSERV V2 - send command to LISTSERV on remote node via TCPGUI interface *
* *
* Copyright L-Soft international 1996-97 - All rights reserved *
* *
* Syntax: *
* *
* lcmdx hostname[:port] address password command *
* *
* Connects to 'hostname' on port 'port' (default=2306) using the LISTSERV *
* TCPGUI protocol, then executes the LISTSERV command 'command' from the *
* origin 'address'. 'password' is the personal LISTSERV password associated *
* with the command origin ('address') - see the description of the PW ADD *
* command for more information on LISTSERV passwords. The reply from *
* LISTSERV is echoed to standard output (the command is executed *
* synchronously). *
* *
******************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef INTUX
#include <net/errno.h>
#endif

#define DEFAULT_PORT 2306

#ifdef ultrix
/* Use read() rather than recv() to bypass a bug in Ultrix */
#define recv(a, b, c, d) read(a, b, c)
#endif

static int receive(int ss, char *buf, int len)
{
char *w, *e;
int l;

for (w = buf, e = buf + len; w < e;) {
l = recv(ss, w, e - w, 0);
if (l <= 0)
return(l);
w += l;
}
return(len);
}

int LSV_send_command(char *hostname, unsigned short port, char *origin,
char *pw, char *command, FILE *writeto)
{
char buf[256], *reply = 0, *cmd, *w, *r, *e;
unsigned char *wb;
int rc, ss, len, orglen, n;
unsigned int ibuf[2];
struct sockaddr_in sa_connect;
struct hostent *H;

/* Initialize */
cmd = malloc(strlen(command) + strlen(pw) + 5);
sprintf(cmd, "%s PW=%s", command, pw);
orglen = strlen(origin);

/* Create a socket */
if ((ss = socket(AF_INET, SOCK_STREAM, 0)) < 0)
goto Socket_Error;

/* Prepare sa_connect structure */
memset(&sa_connect, 0, sizeof(sa_connect));
sa_connect.sin_family = AF_INET;
sa_connect.sin_port = htons(port);
if ((H = gethostbyname(hostname)) && H->h_addr_list[0])
memcpy(&sa_connect.sin_addr, H->h_addr_list[0], 4);
else
goto Socket_Error;

/* Connect to the TCPGUI port */
if (connect(ss, (struct sockaddr *)&sa_connect,
sizeof(sa_connect)) < 0)
goto Socket_Error;

/* Send the protocol level request and the command header */
wb = (unsigned char *)buf;
len = strlen(cmd);
n = len + orglen + 1; /* Byte length */
*wb++ = '1'; /* Protocol level: 1 */
*wb++ = 'B'; /* Mode: binary */
*wb++ = '\r';
*wb++ = '\n';
*wb++ = n / 256; /* Request length byte 1 */
*wb++ = n & 255; /* Request length byte 2 */
*wb++ = orglen; /* Origin length: 1 */
for (r = origin; *r;)
*wb++ = (unsigned char)*r++;

if (send(ss, buf, (char *)wb - buf, 0) < 0)
goto Socket_Error;

/* Await confirmation */
for (w = buf;;) {
n = recv(ss, w, buf + sizeof(buf) - w, 0);
if (n <= 0)
goto Socket_Error;
w += n;
for (r = buf; r < w && *r != '\n'; r++);
if (r != w)
break;
}

/* Anything other than 250 is an error */
if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0')
goto Protocol_Error;

/* Finish sending the command text */
if (send(ss, cmd, len, 0) < 0)
goto Socket_Error;

/* Read the return code and reply length */
if (receive(ss, (char *)ibuf, 8) <= 0)
goto Socket_Error;

/* Exit if the return code is not 0 */
if (ntohl(ibuf[0]))
goto Protocol_Error;

/* Read the reply */
len = ntohl(ibuf[1]);
reply = malloc(len + 1);
if (receive(ss, (char *)reply, len) <= 0)
goto Socket_Error;

/* Cut it into individual lines, and output it */
for (r = reply, e = reply + len; r < e;) {
for (w = r; w < e && *w != '\r'; w++);
*w++ = '\0';
fprintf(writeto, "%s\n", r);
r = w;
if (r < e && *r == '\n')
r++;
}

/* Close the socket and return */
rc = 0;
goto Done;

Protocol_Error:
rc = 1000;
goto Done;

Socket_Error:
rc = errno;
goto Done;

Done:
free(cmd);
if (reply)
free(reply);
if (ss >= 0)
close(ss);
return(rc);
}


#ifndef NO_MAIN
int main(int argc, char **argv)
{
char cmd[8192], hostname[80], *w, *r;
int rc, n;
unsigned short port;

/* Parse positional parameters */
if (argc < 5) {
printf("\
Syntax: lcmdx hostname[:port] address password command\n");
return(EINVAL);
}

port = DEFAULT_PORT;
for (r = argv[1], w = hostname; *r && *r != ':';)
*w++ = *r++;
*w = '\0';
if (*r == ':')
port = atoi(++r);

for (n = 4, w = cmd; n < argc; n++) {
if (w != cmd)
*w++ = ' ';
for (r = argv[n]; *r; *w++ = *r++);
}
*w = '\0';

/* Execute the command */
rc = LSV_send_command(hostname, port, argv[2], argv[3], cmd,
stdout);
if (rc == 1000)
printf("\
>>> Protocol error while communicating with LISTSERV.");
else if (rc != 0)
printf("\
>>> Error - unable to initiate communication with LISTSERV (errno=%d).\n", rc);

return(rc);
}
#endif
