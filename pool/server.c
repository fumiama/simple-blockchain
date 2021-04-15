#include "server.h"

BLKCHNHEAD blkh;
CLISTAT clis[CLISZ];

//增加任务
void add_task(const BLOCK* blk) {
	BLKCHN* bc = malloc(sizeof(BLKCHN));
	bc->next = blkh.head;
	bc->blk = blk;
	blkh.head = bc;
	blkh.len++;
}

//有空闲(wait,fin)客户端，返回序号 否则返回EOF
int get_idle_cli() {
	int i = 0;
	while(i < CLISZ) {
		if(clis[i].cmd == CMD_CLI_WAIT || clis[i].cmd == CMD_CLI_FINTSK) break;
		i++;
	}
	return (i < CLISZ)?i:EOF;
}

//从tail开始执行任务
int deliver_scan_n2_task(uint64_t n1, BLOCK* blk, const uint16_t zerobit_cnt) {
	int cli;
	while((cli = get_idle_cli()) < 0) sleep(1);
	if(clis[cli].cmd == CMD_CLI_WAIT || clis[cli].cmd == CMD_CLI_FINTSK) {
		clis[cli].n1 = n1;
		clis[cli].blk = blk;
		clis[cli].zerobit_cnt = zerobit_cnt;
		clis[cli].cmd = CMD_CLI_START;
		send(clis[cli].accept_fd, clis+cli, sizeof(CLISTAT), 0);	//将数据推至矿机
	}
	return clis[cli].cmd;
}

void scan_finished_n2(const char* folder) {
	int i = 0;
	while(i) {
		if(clis[i].cmd == CMD_CLI_FINTSK) {
			clis[i].blk->n1 = clis[i].n1;
			clis[i].blk->n2 = clis[i].n2;
			append_chain(folder, clis[i].blk);
			for(int j = 0; j < CLISZ; j++) {
				if(clis[j].blk == clis[i].blk) {
					clis[j].cmd = CMD_CLI_FINTSK;
					//通知所有客户端终止运算
					send(clis[j].accept_fd, clis+j, CLISZ, 0);
				}
			}
		}
		if(++i >= CLISZ) i = 0;
		sleep(1);
	}
}

void take_a_task(const uint16_t zerobit_cnt) {
	BLKCHN* bc = blkh.head;
	BLKCHN* prev_bc = NULL;
	while(bc->next) {
		prev_bc = bc;
		bc = bc->next;
	}
	int i;
	for(i = 0; i < CLISZ; i++) {
		deliver_scan_n2_task(i, bc->blk, zerobit_cnt);
	}
	i = 0;
	while(1) {	//等待任务完成
		if(i >= CLISZ) i = 0;
		if(clis[i++].cmd == CMD_CLI_FINTSK) break;
		sleep(1);
	}
}

int bindServer(uint16_t port, unsigned int try_times) {
	int fail_count = 0;
	int result = -1;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	while(!~(result = bind(fd, (struct sockaddr *)&server_addr, struct_len)) && fail_count++ < try_times) sleep(1);
	if(!~result && fail_count >= try_times) {
		puts("Bind server failure!");
		return 0;
	} else {
		puts("Bind server success!");
		return fd;
	}
}

int listenSocket(int fd, unsigned int try_times) {
	int fail_count = 0;
	int result = -1;
	while(!~(result = listen(fd, 10)) && fail_count++ < try_times) sleep(1);
	if(!~result && fail_count >= try_times) {
		puts("Listen failed!");
		return 0;
	} else{
		puts("Listening....");
		return 1;
	}
}

int checkBuffer(CLISTAT *cst) {
    if(cst->data[0] == 'f') {	//说明计算出结果
		sscanf(cst->data+1, "%lu", &(cst->n2));
		cst->cmd = CMD_CLI_FINTSK;
	} else if(cst->data[0] == 'w') {	//客户端可以开始计算
		cst->cmd = CMD_CLI_WAIT;
	} else if(cst->data[0] == 'a') {	//添加block请求
		add_task((BLOCK*)(cst->data+1));
	} else if(cst->data[0] == 'e') {	//终止连接请求
		cst->cmd = CMD_CLI_ENDCON;
	}
}

void handle_quit(int signo) {
    printf("Handle sig %d\n", signo);
    pthread_exit(NULL);
}

#define cli_p(x) ((CLISTAT*)(x))

void acceptTimer(void *p) {
    CLISTAT *timer = cli_p(p);
    signal(SIGQUIT, handle_pipe);
    signal(SIGPIPE, handle_pipe);
    while(timer->thread && !pthread_kill(timer->thread, 0)) {
        sleep(1);
        puts("Check accept status");
        if(timer->cmd == CMD_CLI_ENDCON) {
            pthread_kill(timer->thread, SIGQUIT);
            close(timer->accept_fd);
            timer->thread = 0;
        }
    }
    free(p);
}

void handle_pipe(int signo) {
    puts("Pipe error");
}

int sendData(int accept_fd, char *data, size_t length) {
    if(!~send(accept_fd, data, length, 0)) {
        puts("Send data error");
        return 0;
    } else {
        printf("Send data: ");
        puts(data);
        return 1;
    }
}

void handleAccept(void *p) {
    int accept_fd = cli_p(p)->accept_fd;
    if(accept_fd > 0) {
        puts("Connected to the client.");
        signal(SIGQUIT, handle_quit);
        signal(SIGPIPE, handle_pipe);
        pthread_t thread;
        if (pthread_create(&thread, NULL, (void *)&acceptTimer, p)) puts("Error creating timer thread");
        else puts("Creating timer thread succeeded");
        sendData(accept_fd, "Welcome to simple blockchain pool server.", 42);
        char *buff = calloc(BLKSZ+BUFSIZ, sizeof(char));
        if(buff) {
            cli_p(p)->data = buff;
            while(cli_p(p)->thread && (cli_p(p)->numbytes = recv(accept_fd, buff, BUFSIZ, 0)) > 0) {
                buff[cli_p(p)->numbytes] = 0;
                printf("Get %zd bytes: %s\n", cli_p(p)->numbytes, buff);
                puts("Check buffer");
                if(!checkBuffer(cli_p(p))) break;
            }
            printf("Recv %zd bytes\n", cli_p(p)->numbytes);
        } else puts("Error allocating buffer");
        close(accept_fd);
    } else puts("Error accepting client");
}

void acceptClient(int fd) {
	while(1) {
		puts("Ready for accept, waitting...");
		int p = 0;
		while(p < CLISZ && clis[p].thread && !pthread_kill(clis[p].thread, 0)) p++;
		if(p < CLISZ) {
			printf("Run on thread No.%d\n", p);
			struct sockaddr_in client_addr;
			clis[p].accept_fd = accept(fd, (struct sockaddr *)&client_addr, &struct_len);
			clis[p].cmd = CMD_CLI_WAIT;
			if (pthread_create(&(clis[p].thread), NULL, (void *)&handleAccept, &clis[p])) puts("Error creating thread");
			else puts("Creating thread succeeded");
		} else {
			puts("Max thread cnt exceeded");
			sleep(1);
		}
	}
}

void handleTasks(const void *p_zerobit_cnt) {
	signal(SIGQUIT, handle_pipe);
    signal(SIGPIPE, handle_pipe);
	while(1) {
		take_a_task(*((uint16_t*)p_zerobit_cnt));
		sleep(1);
	}
}

#define showUsage(program) printf("Usage: %s [-d] port try_listen_times zerobit_cnt\n\t-d: As daemon\n", program)

int main(int argc, char *argv[]) {
    if(argc != 4 && argc != 5) showUsage(argv[0]);
    else {
        int port = 0;
        int as_daemon = !strcmp("-d", argv[1]);
        sscanf(argv[as_daemon?2:1], "%d", &port);
        if(port > 0 && port < 65536) {
            int times = 0;
            sscanf(argv[as_daemon?3:2], "%d", &times);
            if(times > 0) {
                if(!as_daemon || (as_daemon && (daemon(1, 1) >= 0))) {
                    signal(SIGQUIT, handle_pipe);
                    signal(SIGPIPE, handle_pipe);
					int fd;
                    if((fd = bindServer(port, times))) if(listenSocket(fd, times)) {
						pthread_t take_thread;
						uint16_t zbc = 0;
            			sscanf(argv[as_daemon?4:3], "%hu", &zbc);
						if(zbc > 0 && zbc < 256) {
							if (pthread_create(&take_thread, NULL, (void *)&handleTasks, &zbc)) puts("Error creating task-handling thread");
							acceptClient(fd);
						} else puts("Zerobit count error");
					}
                } else puts("Start daemon error");
            } else printf("Error times: %d\n", times);
        } else printf("Error port: %d\n", port);
    }
    exit(EXIT_FAILURE);
}
