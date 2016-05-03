/***************************************************************************
 *            fwServer.c
 *
 *  Copyright  2016  mc
 *  <mc@<host>>
 ****************************************************************************/

#include "fwServer.h"

/**
 * Returns the port specified as an application parameter or the default port
 * if no port has been specified.
 * @param argc the number of the application arguments.
 * @param an array with all the application arguments.
 * @return  the port number from the command line or the default port if 
 * no port has been specified in the command line. Returns -1 if the application
 * has been called with the wrong parameters.
 */
int getPort(int argc, char* argv[])
{
  int param;
  int port = DEFAULT_PORT;

  optind=1;
  // We process the application execution parameters.
	while((param = getopt(argc, argv, "p:")) != -1){
		switch((char) param){
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


 /**
 * Function that sends a HELLO_RP to the  client
 * @param sock the communications socket
 */
void process_HELLO_msg(int sock)
{
  struct hello_rp hello_resp;

  char message[12]= "Hello World";
  memcpy(&hello_resp.msg,message,sizeof(message));
  stshort(MSG_HELLO_RP,&hello_resp);

  send(sock,&hello_resp,sizeof(hello_resp),0);

  //TODO
}

void process_FINISH(int sock)
{
	char toClient[MAX_BUFF_SIZE];
	memset(toClient,0,sizeof(toClient));
	
	stshort(MSG_OK,&toClient);
	send(sock,&toClient,sizeof(toClient),0);
	
}

void process_LIST(int sock, struct FORWARD_chain *chain)
{
	int i;
	char bufferRules[MAX_BUFF_SIZE];
	
	memset(bufferRules,0,sizeof(bufferRules));
	if (chain->first_rule!=NULL)
	{
		struct fw_rule *current_rule = chain->first_rule;
		stshort(MSG_RULES,bufferRules);
		stshort((unsigned short)chain->num_rules,bufferRules+2);
		for (i=0;i<chain->num_rules;i++)
		{
			memcpy(bufferRules+4+i*RULE_SIZE,&(current_rule->rule),RULE_SIZE);
			current_rule= current_rule->next_rule;
		}
		send(sock,bufferRules,sizeof(bufferRules),0);
	}
	else
	{
		stshort(MSG_OK,bufferRules);
		send(sock,bufferRules,sizeof(bufferRules),0);
	}
}

void process_ADD(int sock, rule* addRule, struct FORWARD_chain *chain)
{
	struct fw_rule* nodeRule;
	rule* fowardRule;
	
	fowardRule = (rule*) malloc (sizeof(rule));
	memmove(fowardRule,addRule,sizeof(rule));
	
	nodeRule= (struct fw_rule*) malloc(sizeof(struct fw_rule));
	nodeRule->rule= *fowardRule;
	nodeRule->next_rule= NULL;
	
	
	if( chain->num_rules == 0)
	{
		chain->first_rule= nodeRule;
	}
	else
	{
		struct fw_rule* currentRule= chain->first_rule;
		while(currentRule->next_rule != NULL)
		{
			currentRule= currentRule->next_rule;
		}
		currentRule->next_rule= nodeRule;
	}
	(*(int*)chain) = (*(int*)chain)+1;

	//send OK to the client
	char toClient[MAX_BUFF_SIZE];
	memset(toClient,0,sizeof(toClient));
	stshort(MSG_OK,&toClient);
	send(sock,&toClient,sizeof(toClient),0);
	
}

void process_CHANGE(int sock, unsigned short ruleNumber, rule* addRule, struct FORWARD_chain *chain)
{
	struct fw_rule* currentNodeRule;
	currentNodeRule = chain->first_rule;

	unsigned short currentRulePosition= 1; //first rule

	char toClient[4];
	memset(toClient,0,sizeof(toClient));

	if(ruleNumber >  chain->num_rules)
	{
		
		stshort(MSG_ERR,&toClient);
		send(sock,&toClient,sizeof(toClient),0);
	}
	else
	{
		if(ruleNumber == 1)
		{
		//if the rule we want to eliminate is the first one, we have to set the first_rule to other one.
		chain->first_rule=currentNodeRule->next_rule;
		free(currentNodeRule);
		}
		else
		{
			struct fw_rule* previousNodeRule;
			while( currentRulePosition < ruleNumber)
			{
				previousNodeRule= currentNodeRule;
				currentNodeRule = currentNodeRule->next_rule;
				currentRulePosition+=1;
			}
			//be sure of keeping a reference to the next rule of the rule we want to 
			//eliminate, so we dont lose access to following rules.
			previousNodeRule->next_rule= currentNodeRule->next_rule; 
			free(currentNodeRule);
		}

		(*(int*)chain) = (*(int*)chain)-1;

		struct fw_rule* nodeRule;
		rule* fowardRule;
	
		fowardRule = (rule*) malloc (sizeof(rule));
		memmove(fowardRule,addRule,sizeof(rule));
	
		nodeRule= (struct fw_rule*) malloc(sizeof(struct fw_rule));
		nodeRule->rule= *fowardRule;
		nodeRule->next_rule= NULL;
	
	
		if( chain->num_rules == 0)
		{
			chain->first_rule= nodeRule;
		}
		else
		{
			struct fw_rule* currentRule= chain->first_rule;
			while(currentRule->next_rule != NULL)
			{
				currentRule= currentRule->next_rule;
			}
			currentRule->next_rule= nodeRule;
		}
		(*(int*)chain) = (*(int*)chain)+1;

		//send OK to the client
		stshort(MSG_OK,&toClient);
		send(sock,&toClient,sizeof(toClient),0);
	}	
}

void process_DELETE(int sock, unsigned short ruleNumber, struct FORWARD_chain *chain)
{
	struct fw_rule* currentNodeRule;
	currentNodeRule = chain->first_rule;

	unsigned short currentRulePosition= 1; //first rule

	char toClient[4];
	memset(toClient,0,sizeof(toClient));

	if(ruleNumber >  chain->num_rules)
	{
		
		stshort(MSG_ERR,&toClient);
		send(sock,&toClient,sizeof(toClient),0);
	}
	else
	{
		if(ruleNumber == 1)
		{
		//if the rule we want to eliminate is the first one, we have to set the first_rule to other one.
		chain->first_rule=currentNodeRule->next_rule;
		free(currentNodeRule);
		}
		else
		{
			struct fw_rule* previousNodeRule;
			while( currentRulePosition < ruleNumber)
			{
				previousNodeRule= currentNodeRule;
				currentNodeRule = currentNodeRule->next_rule;
				currentRulePosition+=1;
			}
			//be sure of keeping a reference to the next rule of the rule we want to 
			//eliminate, so we dont lose access to following rules.
			previousNodeRule->next_rule= currentNodeRule->next_rule; 
			free(currentNodeRule);
		}

		(*(int*)chain) = (*(int*)chain)-1;
		fflush(stdout);

		//send OK to the client
		stshort(MSG_OK,&toClient);
		send(sock,&toClient,sizeof(toClient),0);
	}

}

void process_FLUSH(int sock, struct FORWARD_chain *chain)
{

	while(chain->num_rules != 0)
	{
		process_DELETE(sock,1,chain);
	}

	char toClient[4];
	memset(toClient,0,sizeof(toClient));

	if((chain-> num_rules == 0) && (chain->first_rule == NULL))
	{
		//send OK to Client
		stshort(MSG_OK,&toClient);
		send(sock,&toClient,sizeof(toClient),0);
	}
	else
	{
		//send error
		stshort(MSG_ERR,&toClient);
		send(sock,&toClient,sizeof(toClient),0);
	}
}

int process_msg(int sock, struct FORWARD_chain *chain)
 /**
 * Receives and process the request from a client.
 * @param the socket connected to the client.
 * @param chain the chain with the filter rules.
 * @return 1 if the user has exit the client application therefore the 
 * connection whith the client has to be closed. 0 if the user is still 
 * interacting with the client application.
 */
{
  unsigned short op_code;
  int finish = 0;
  rule addRule;
  char buffer[MAX_BUFF_SIZE];
  int bytes= recv(sock,buffer,MAX_BUFF_SIZE,0);
  printf("Se han leido %d bytes\n",bytes);

  op_code = ldshort(buffer);
  
  unsigned short ruleToEliminate;
  switch(op_code)
  {
    case MSG_HELLO:
      process_HELLO_msg(sock);
      break;
    case MSG_LIST:
	  process_LIST(sock, chain);
      break;
    case MSG_ADD:
      memcpy(&addRule, buffer+2, sizeof(addRule));
	  process_ADD(sock, &addRule, chain);
      break;
    case MSG_CHANGE:
      ruleToEliminate = ldshort(buffer+2);	
      memcpy(&addRule,buffer+4,sizeof(addRule));
      process_CHANGE(sock,ruleToEliminate,&addRule,chain);
      break;
    case MSG_DELETE:
      //memcpy(&fromClient,buffer+2,2);
      ruleToEliminate = ldshort(buffer+2);
      process_DELETE(sock,ruleToEliminate,chain);
      break;
    case MSG_FLUSH:
      process_FLUSH(sock,chain);
      break;
    case MSG_FINISH:
	  process_FINISH(sock);
      finish = 1;
      break;
    default:
      perror("Message code does not exist.\n");
  }

  return finish;
}



int main(int argc, char *argv[]){

  int port = getPort(argc, argv);
  int finish=0;
  struct FORWARD_chain chain;

  chain.num_rules=0;
  chain.first_rule=NULL;

  int s= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in server;
  server.sin_family= AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = INADDR_ANY;


  bind(s,(struct sockaddr*)&server,sizeof(server));

  struct sockaddr_in client;
  socklen_t addr = sizeof(client);
  pid_t pid;

  while(1)
  {
	//server alive
	listen(s,MAX_QUEUED_CON);

	int socketClient= accept(s,(struct sockaddr*)&client,&addr);
	printf("Conexió Realitzada\n");

	pid= fork();
	if(pid == 0)
	{
		//case of children
    	do {
		//already connected with client
      	finish = process_msg(socketClient,&chain);
    	}while(!finish);

    	close(socketClient);
    	break;
	}

  }

  return 0;
 }
