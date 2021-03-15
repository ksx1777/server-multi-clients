#include "server.h"

/* variáveis globais */
struct sockaddr_in *client_list;

new_arg_list console_args[] = {
	{"-ip", "endereço ip do servidor na rede."},
	{"-port", "porta usada para iniciar conexões com o endereço ip."},
	{"-queue", "número máximo de conexões simultâneas."}
};

new_socket_conf socket_conf[] = {0x0, 0x0, 0x0};

int atual_conn = 0;
int sofd;
pthread_mutex_t queue_mutex;
int *connected;

void help(void){
	printf("[i] comandos disponíveis: \n");
	for(int i = 0; i < 3; i ++){
		printf(" %s : %s\n", console_args[i].name, console_args[i].help);
	}
}

char *get_argument(int argc, char **arg_value, char *arg_name){
	for(int i = 0; i < argc; i++) if(scmp(arg_value[i], arg_name) == TRUE && i+1 < argc) return arg_value[i+1];
	return (char)FALSE;
}

/* função para configurar socket do servidor */
int socket_bind(char *ip, short port, int queue_length){
	struct sockaddr_in server_info;
	server_info.sin_addr.s_addr = inet_addr(ip);
	server_info.sin_port = htons(port);
	server_info.sin_family = AF_INET;

	int s = socket(AF_INET, SOCK_STREAM, FALSE);
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) != FALSE) err(EXIT_FAILURE, "");
	else if(bind(s, (struct sockaddr*)&server_info, sizeof(server_info)) != FALSE) err(EXIT_FAILURE, "");
	else if(listen(s, queue_length) != FALSE) err(EXIT_FAILURE, "[f]");
	return s;
}

int init_new_conn(int s, int conn_num){
	int conn_length = sizeof(struct sockaddr_in);
	int socket_fd = accept(s, (struct sockaddr*)&client_list[conn_num], &conn_length);
	if(socket_fd == -1) err(EXIT_FAILURE, "");
	printf("[*] nova conexão aberta vinda do endereço %s:%i (%i)\n", inet_ntoa(client_list[conn_num].sin_addr), client_list[conn_num].sin_port, conn_num+1);
	return socket_fd;
}

int send_packet(int conn_num, char *content, int length){
	if(send(connected[conn_num], content, length, MSG_NOSIGNAL) == -1){
		printf("[*] cliente %s:%i (%i) desconectou-se.\n", inet_ntoa(client_list[conn_num].sin_addr), client_list[conn_num].sin_port, conn_num+1);
		close(connected[conn_num]);
		mset(&connected[conn_num], 0x0, sizeof(connected[conn_num]));
		return -1;
	}
	return FALSE;
}

void *connections_handler(void *arg){
	usleep((rand() % 100) * 1000);
	int c = 0;	

	while(TRUE){
		pthread_mutex_lock(&queue_mutex);
		for(atual_conn = 0; atual_conn < socket_conf->max_conn;){
			if(connected[atual_conn] == 0x0){
				connected[atual_conn] = init_new_conn(sofd, atual_conn);
				c = atual_conn;
				break;
			} else atual_conn++;
		}
		pthread_mutex_unlock(&queue_mutex);
	

		while(TRUE){
			/* código que o servidor executará enquanto conectado com o cliente */
			usleep(200 * 1000);
			if(send_packet(c, "Teste\n", 6) == -1){
				break;
			}
		}
			/* conexão se encerra no final do loop */
		printf("[*] conexão fechada com o endereço %s:%i (%i)\n\n", inet_ntoa(client_list[c].sin_addr), client_list[c].sin_port, c+1);
	}
	return NULL;
}

int main(int argc, char** argv){
	/* configuração definida pelo console */
	socket_conf->ip = get_argument(argc, argv, "-ip");
	if(socket_conf->ip == FALSE) socket_conf->ip = "127.0.0.1";
	if(get_argument(argc, argv, "-port") != FALSE) socket_conf->port = stoi(get_argument(argc, argv, "-port"));
	else socket_conf->port = 4444;
	if(get_argument(argc, argv, "-queue") != FALSE) socket_conf->max_conn = stoi(get_argument(argc, argv, "-queue"));
	else socket_conf->max_conn = 2;
	if(argc >= 2 && scmp(argv[1], "-help") == TRUE) help();
	else printf("[*] use o parâmetro \"-help\" para listar os comandos.\n");

	printf("\n[i] configuração do servidor:\n"
		   "  endereço ip usado na espera por conexões : %s\n"
		   "  porta usada para se conectar ao endereço : %hi\n"
		   "  número máximo de conexões simultâneas : %hi\n\n", socket_conf->ip, socket_conf->port, socket_conf->max_conn);

	/* viaráveis que armazenam dados relacionados ao servidor */
	client_list = (struct sockaddr_in*)calloc(socket_conf->max_conn, sizeof(struct sockaddr_in));
	connected = (int*)calloc(socket_conf->max_conn, sizeof(int));

	/* funções relacionadas ao gerenciamento do servidor */

	if(pthread_mutex_init(&queue_mutex, NULL)) {
    	err(EXIT_FAILURE, "%s", "Não foi possível iniciar a fila de threads.\n");
	}

	printf("[*] aguardando conexão no endereço %s:%hi ...\n", socket_conf->ip, socket_conf->port);
	sofd = socket_bind(socket_conf->ip, socket_conf->port, socket_conf->max_conn);
	pthread_t *threads = calloc(socket_conf->max_conn, sizeof(pthread_t));

  	for(int i = 0; i < socket_conf->max_conn-1; i++) {
    	if(pthread_create(&threads[i], NULL, connections_handler, NULL)) {
      		err(EXIT_FAILURE, "%s", "Erro ao criar thread");
		}
	}

	connections_handler(NULL);
	return TRUE;
}
