#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#define NUM 1024
#define OPT_NUM 64

#define NONE_REDIR 0
#define INPUT_REDIR 1
#define OUTPUT_REDIR 2
#define APPEND_REDIR 3
#define trimSpace(start) do{\
    while(isspace(*start)) ++start;\
}while(0);

char lineCommand[NUM];
char *myargv[OPT_NUM];
int lastCode = 0;
int lastSig = 0;

int redirType = NONE_REDIR;
char *redirFile = NULL;

//"ls -a -l -i > myfile.txt" -> "ls -a -l -i" > >> < "myfile.txt"
void  commandCheck(char* commands){
    assert(commands);
    char *start = commands;
    char *end = commands + strlen(commands);

    while(start < end){
        if(*start == '>'){
            *start = '\0';
            ++start;
            if(*start == '>'){
                redirType = APPEND_REDIR;
                ++start;
            }else{
                redirType = OUTPUT_REDIR;
                //++start;
            }
            trimSpace(start);
            redirFile = start;
            break;
        }else if(*start == '<'){
            *start = '\0';//将start所在的位置置为'\0'使字符串分割
            ++start;
            trimSpace(start);//过滤空格
            //填写重定向信息
            redirType = INPUT_REDIR;
            redirFile = start;
            break;
        }else{
            ++start;
        }
    }
}

int main(){
    while(1){
        //初始化 redirType和 redirFile    
        int redirType = NONE_REDIR;
        char *redirFile = NULL;
        errno = 0;

        //输出提示符
        printf("用户名@主机名:当前路径$ ");
        fflush(stdout);

        //获取用户输入，输入的时候输入\n
        char *s = fgets(lineCommand, sizeof(lineCommand) - 1, stdin);
        assert(s != NULL);
        //清除最后一个\n
        lineCommand[strlen(lineCommand) - 1] = '\0';

        commandCheck(lineCommand);

        //字符串切割
        myargv[0] = strtok(lineCommand, " ");
        int i = 1;

        if(myargv[0] != NULL && strcmp(myargv[0], "ls") == 0){
            myargv[i++] = "--color=auto";
        }

        //如果没有子串 strtok->NULL myargc[end] == NULL
        while(myargv[i++] = strtok(NULL, " "));

        //如果是cd命令 不需要创建子进程 让shell自己执行对应的程序 本质就是执行系统接口
        //像这种不需要让子进程来执行 而是让shell自己执行的命令 ---- 内建/内置命令
        if(myargv[0] != NULL && strcmp(myargv[0], "cd") == 0){
            if(myargv[1] != NULL) chdir(myargv[1]);
            continue;
        }
        if(myargv[0] != NULL && myargv[1] != NULL && strcmp(myargv[0], "echo") == 0){
            if(strcmp(myargv[1], "$?") == 0){
                printf("%d, %d\n", lastCode, lastSig);
            }else{
                printf("%s\n", myargv[1]);
            }
            continue;
        }

#ifdef DEBUG
        for(int i = 0; myargv[i]; ++i){
            printf("[%d]%s\n", i, myargv[i]);
        }
#endif
        //执行命令
        pid_t id = fork();
        assert(id != -1);

        if(id == 0){
            //因为命令是由子进程执行的，真正重定向的工作一定是子进程来完成的
            //如何重定向是父进程要给子进程供信息的
            switch(redirType){
                case NONE_REDIR:
                    //什么都不做
                    break;
                case INPUT_REDIR:
                    {
                        int fd = open(redirFile, O_RDONLY, 0666);
                        if(fd < 0){
                            perror("open error!\n");
                            exit(errno);
                        }
                        //重定向的文件已经成功打开了
                        dup2(fd, 0);
                    }
                    break;
                case OUTPUT_REDIR:
                case APPEND_REDIR:
                    { 
                        int flags = O_WRONLY | O_CREAT;
                        if(redirType == APPEND_REDIR){
                            flags |= O_APPEND;
                        }else{
                            flags |= O_TRUNC;

                            int fd = open(redirFile, flags, 0666);
                            if(fd < 0){
                                perror("open error\n");
                                exit(errno);
                            }
                            dup2(fd, 1);
                        }
                        break;
                        default:
                        printf("bug?\n");
                        break;
                    }
                    execvp(myargv[0], myargv);
                    exit(1);
            }

            int status = 0;
            pid_t ret = waitpid(id, &status, 0);
            //assert(ret > 0);
            printf("166 ret = %d\n", ret);
            lastCode = (status >> 8) & 0xFF;
            lastSig = status & 0x7F;
        }
        return 0;
    }
}
