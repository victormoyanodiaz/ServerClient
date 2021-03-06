/***************************************************************************
 *            fwClient.h
 *
 *  Copyright  2016  mc
 *  <mcarmen@<host>>
 ****************************************************************************/
#include "fwClient.h"


int setaddrbyname(struct sockaddr_in *addr, char *host)
/**
 * Function that sets the field addr->sin_addr.s_addr from a host name 
 * address.
 * @param addr struct where to set the address.
 * @param host the host name to be converted
 * @return -1 if there has been a problem during the conversion process.
 */
{
  struct addrinfo hints, *res;
	int status;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM; 
 
  if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return -1;
  }
  
  addr->sin_addr.s_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
  
  freeaddrinfo(res);
    
  return 0;  
}



int getPort(int argc, char* argv[])
/**
 * Returns the port specified as an application parameter or the default port
 * if no port has been specified.
 * @param argc the number of the application arguments.
 * @param an array with all the application arguments.
 * @return  the port number from the command line or the default port if 
 * no port has been specified in the command line. Returns -1 if the application
 * has been called with the wrong parameters.
 */
{
  int param;
  int port = DEFAULT_PORT;

  optind=1;
  // We process the application execution parameters.
	while((param = getopt(argc, argv, "h:p:")) != -1){
		switch((char) param){
		  case 'h': break;
			case 'p':
			  // We modify the port variable just in case a port is passed as a 
			  // parameter
				port = atoi(optarg);
				break;
			default:
				printf("Parametre %c desconegut\n\n", (char) param);
				port = -1;
		}
	}
	
	return port;
}


 char * getHost(int argc, char* argv[])
/**
 * Returns the host name where the server is running.
 * @param argc the number of the application arguments.
 * @param an array with all the application arguments.
 * @Return Returns the host name where the server is running.<br />
 * Returns null if the application has been called with the wrong parameters.
 */
 {
  char * hostName = NULL;
  int param;

  optind=1;
    // We process the application execution parameters.
	while((param = getopt(argc, argv, "h:p:")) != -1){
		switch((char) param){
			case 'p': break;
			case 'h':
        hostName = (char*) malloc(sizeof(char)*strlen(optarg)+1);
				// Un cop creat l'espai, podem copiar la cadena
				strcpy(hostName, optarg);
				break;
			default:
				printf("Parametre %c desconegut\n\n", (char) param);
				hostName = NULL;
		}
	}

	printf("in getHost host: %s\n", hostName); //!!!!!!!!!!!!!!
	return hostName;
 }




void print_menu()
/**
 * Shows the menu options.
 */
{
		// Mostrem un menu perque l'usuari pugui triar quina opcio fer

		printf("\nAplicació de gestió del firewall\n");
		printf("  0. Hello\n");
		printf("  1. Llistar les regles filtrat\n");
		printf("  2. Afegir una regla de filtrat\n");
		printf("  3. Modificar una regla de filtrat\n");
		printf("  4. Eliminar una regla de filtrat\n");
		printf("  5. Eliminar totes les regles de filtrat.\n");
		printf("  6. Sortir\n\n");
		printf("Escull una opcio: ");
}

void readRule(char* addRule)
/*
*	This functions guides the user for getting a rule. Stores the values in 
*	buffer passed by parameters
*/
{

	char yes;
	char srcdst[10];
	unsigned short nsrcdst;
	char addrQuad[16];
	struct in_addr addr;
	unsigned short netmask;
	unsigned short src_dst_port = 0;
	unsigned short port = 0;
	
	
	memset(addRule,0,12);
	printf("Introdueix la regla seguint el format:\n");
	printf("src|dst Address Netmask [sport|dport] [port]\n");
	
	//Analisi del input
	//src
	printf("src|dst: ");
	scanf("%s",srcdst);
	if(!strcmp(srcdst,"src")) nsrcdst= SRC;
	else nsrcdst= DST;
	
	//addr
	printf("Address: ");
	scanf("%s",addrQuad);
	inet_aton(addrQuad,&addr);
	
	printf("Netmask: ");
	scanf("%hu",&netmask);
	
	printf("¿Quieres especificar un puerto?(s/n)");
	yes = getchar();
	yes = getchar();

	if(yes == 115) //115 is 's' in ascii table
	{
		printf("sport|dport: ");
		scanf("%s",srcdst);
		
		if(!strcmp(srcdst,"src")) src_dst_port= SRC;
		else src_dst_port= DST;
		printf("port: ");
		scanf("%hu",&port);
	}

	
	memcpy(addRule,&addr,sizeof(addr));
	stshort(nsrcdst,addRule+4);
	stshort(netmask,addRule+6);
	stshort(src_dst_port,addRule+8);
	stshort(port,addRule+10);


}


void process_hello_operation(int sock)
/**
 * Sends a HELLO message and prints the server response.
 * @param sock socket used for the communication.
 */
{
  struct hello_rp hello;
  unsigned short op_rp;
  char buffer[MAX_BUFF_SIZE];
  
  memset(buffer,0,sizeof(buffer));
  stshort(MSG_HELLO,buffer);

  send(sock,buffer, sizeof(buffer), 0); //envia el hello request al server
  
  
  memset(&hello,0,sizeof(hello));
  memset(buffer,0,sizeof(buffer));
  recv(sock,buffer,sizeof(buffer),0); //rep el hello world del server 
  
  memcpy(&hello,buffer,sizeof(hello));
  op_rp= ldshort(&hello);
  if (op_rp == MSG_HELLO_RP) printf("%s\n",hello.msg);
  else printf("Ha existit algun error en el Hello World\n");
	  
	  
}


void process_exit_operation(int sock)
/**
 * Closes the socket connected to the server and finishes the program.
 * @param sock socket used for the communication.
 */
{
	
	char fromServer[MAX_BUFF_SIZE];
	unsigned short op_rp;
	
	stshort(MSG_FINISH,fromServer);
	send(sock,fromServer, sizeof(fromServer), 0);
	
	memset(&fromServer,0,sizeof(fromServer));
	recv(sock,&fromServer,sizeof(fromServer),0);
	op_rp= ldshort(&fromServer);
	if (op_rp == MSG_OK) exit(0);
	else printf("Error en la finalització.\n");
}

void process_list_rules(int sock)
{
	char toServer[MAX_BUFF_SIZE];
    char rulesInfo[MAX_BUFF_SIZE];
    char buffer[RULE_SIZE]; //this buffer is for temporaly storing each rule. 
    int i;
    unsigned short op_rp;
    unsigned short number_rules;
    
    struct in_addr ip_addr;
    char ipQuad[16];
    unsigned short src;
    char srcdst[3];
    unsigned short netmask;
    unsigned short sport; 
    char sportDport[5];
    unsigned short port;
    
    stshort(MSG_LIST,&toServer);
    send(sock,&toServer,sizeof(toServer),0);
    printf("Regles de FORWARD:\n");
    memset(rulesInfo,0,sizeof(rulesInfo));
    recv(sock,&rulesInfo,sizeof(rulesInfo),0);
    op_rp= ldshort(rulesInfo);
    if(op_rp == MSG_RULES)
    {
		memset(buffer,0,sizeof(buffer));
		number_rules = ldshort(rulesInfo+2); //the first 2 positions are for storing the operation code. The next 2 for the number of rules.
		for (i=0; i< number_rules; i++)
		{
			memcpy(buffer,rulesInfo+4+i*RULE_SIZE,RULE_SIZE);
			memcpy(&ip_addr,buffer,sizeof(ip_addr));
			strcpy(ipQuad,inet_ntoa(ip_addr));
			
			src= ldshort(buffer+4);
			if(src == SRC) strcpy(srcdst,"src");
			else strcpy(srcdst,"dst");
			
			netmask = ldshort(buffer+6);
			
			sport=ldshort(buffer+8);
			if(sport== SRC) strcpy(sportDport,"sport");
			else strcpy(sportDport,"dport");
			
			port =ldshort(buffer+10);
			
			if( port == 0 && sport== 0) printf("%d. %s %s/%hu\n",i+1,srcdst,ipQuad,netmask);
			else printf("%d. %s %s/%hu %s %hu\n",i+1,srcdst,ipQuad,netmask,sportDport,port);
			
		}
	}
	else if (op_rp== MSG_OK)
	{
		printf("La lista de reglas de FORWARD se encuentra vacia.\n");
	}
	else
	{
		printf("Error recibiendo la lista de las reglas.\n");
	}
		
    
  
}

void process_add_rule(int sock)
{
	//request
	char addRule[MAX_BUFF_SIZE];
	
	stshort(MSG_ADD,addRule);
	readRule(addRule+2);


	send(sock,&addRule,sizeof(addRule),0);

	//response
	unsigned short op_rp;
	memset(addRule,0,sizeof(addRule));
	recv(sock,&addRule,sizeof(addRule),0);
    op_rp= ldshort(&addRule);
    if(op_rp == MSG_OK)
    {
		printf("La regla introducida se ha añadido correctamente a la lista de reglas del servidor\n");
	}
	else
	{
		printf("Ha habido algún fallo añadiendo la regla a la lista del servidor.\n");
	}
}

void process_change_rule(int sock)
{
	char change_rule[MAX_BUFF_SIZE]; //2bytes from OPCODE + 2Bytes from unsigned short indicating rule to change+ 12bytes
	memset(change_rule,0,sizeof(change_rule));

	unsigned short numberToDelete;
	printf("Please enter the number of the rule you want to change(from 1 to 255):\n");
	scanf("%hu",&numberToDelete);
	while(numberToDelete < 1 || numberToDelete >255)
	{
		printf("The number introduced was not correct. Please try again.\n");
		scanf("%hu",&numberToDelete);
	}


	stshort(MSG_CHANGE,change_rule);
	stshort(numberToDelete,change_rule+2);
	
	readRule(change_rule+4);

	fflush(stdout);
	printf("%s",change_rule);
	fflush(stdout);
	send(sock,change_rule,sizeof(change_rule),0);
	//response
	unsigned short op_rp;
	memset(change_rule,0,sizeof(change_rule));
	recv(sock,change_rule,sizeof(change_rule),0);
    op_rp= ldshort(change_rule);

    if(op_rp == MSG_OK)
    {
		printf("\nRule selected was removed correctly.\n");
	}
	else
	{
		printf("There was a problem removing the rule you selected. Probably the rule didn't exist, check again please\n");
	}

	

}

void process_delete_rule(int sock)
{
	char del_rule[MAX_BUFF_SIZE]; //2bytes from OPCODE + 2Bytes from unsigned short indicating rule to delete
	unsigned short numberToDelete;
	printf("Please enter the number of the rule you want to delete(from 1 to 255):\n");
	scanf("%hu",&numberToDelete);

	while(numberToDelete < 1 || numberToDelete >255)
	{
		printf("The number introduced was not correct. Please try again.\n");
		scanf("%hu",&numberToDelete);
	}

	stshort(MSG_DELETE,del_rule);
	stshort(numberToDelete,del_rule+2);
	send(sock,del_rule,sizeof(del_rule),0);

	//response
	unsigned short op_rp;
	memset(del_rule,0,sizeof(del_rule));
	recv(sock,&del_rule,sizeof(del_rule),0);
    op_rp= ldshort(&del_rule);
    if(op_rp == MSG_OK)
    {
		printf("Rule selected was removed correctly.\n");
	}
	else
	{
		printf("There was a problem removing the rule you selected. Probably the rule didn't exist, check again please\n");
	}

}

void process_flush(int sock)
{
	char op_code[MAX_BUFF_SIZE]; //2bytes from OPCODE 

	stshort(MSG_FLUSH,op_code);
	send(sock,op_code,sizeof(op_code),0);

	//response
	unsigned short op_rp;
	char err_code[MAX_BUFF_SIZE]; //2bytes from OPCODE + 2 bytes error specification
	memset(err_code,0,sizeof(err_code));
	recv(sock,&err_code,sizeof(err_code),0);
    op_rp= ldshort(&err_code);
    if(op_rp == MSG_OK)
    {
		printf("All rules were removed correctly.\n");
	}
	else
	{
		printf("There was a problem removing all the rules.\n");
	}
}


void process_menu_option(int s, int option)
/**
 * Function that process the menu option set by the user by calling
 * the function related to the menu option.
 * @param s The communications socket
 * @param option the menu option specified by the user.
 */
{
  switch(option){
    // Opció HELLO
    case MENU_OP_HELLO:
      process_hello_operation(s);
      break;
    case MENU_OP_LIST_RULES:
      process_list_rules(s);
      break;
    case MENU_OP_ADD_RULE:
      process_add_rule(s);
      break;
    case MENU_OP_CHANGE_RULE:
      process_change_rule(s);
      break;
    case MENU_OP_DEL_RULE:
      process_delete_rule(s);
      break;
    case MENU_OP_FLUSH:
      process_flush(s);
      break;
    case MENU_OP_EXIT:
		process_exit_operation(s);
		break;
    default:
      printf("Invalid menu option\n");
  }
}


int main(int argc, char *argv[]){
  int clientSocket;
  unsigned short port;
  char *hostName;
  int menu_option = 0;

  port = getPort(argc, argv);
  hostName = getHost(argc, argv);

  //Checking that the host name has been set.Otherwise the application is stopped.
	if(hostName == NULL){
		perror("No s'ha especificat el nom del servidor\n\n");
		return -1;
	}

	struct sockaddr_in addr_server; //in this struct we are going to save info about the server
	setaddrbyname(&addr_server,hostName); //put info in addr_server
	addr_server.sin_port=htons(port);
	addr_server.sin_family=AF_INET;

	socklen_t client_addrlen= sizeof(addr_server);
	clientSocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	
	int connection= connect(clientSocket,(const struct sockaddr *)&addr_server,client_addrlen);
	
	if(connection==0)
	{
		printf("Conexió establida correctament\n");
		do{
		  print_menu();
		  // getting the user input.
		  scanf("%d",&menu_option);
		  printf("\n\n");
		  process_menu_option(clientSocket, menu_option);
		
		}while(menu_option != MENU_OP_EXIT); //end while(opcio)
	}
	else
	{
		printf("Conexió errònea.\n");
	}
	
	

	close(clientSocket);
	return 0;
}
