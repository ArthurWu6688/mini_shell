#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#define NUM 1024
#define OPT_NUM 64

char lineCommand[NUM];
char *myargv[OPT_NUM];
int lastCode = 0;
int lastSig = 0;

int main(){
    while(1){
        //输出提示符
        printf("用户名@主机名:当前路径$ ");
        fflush(stdout);

        //获取用户输入，输入的时候输入\n
        char *s = fgets(lineCommand, sizeof(lineCommand) - 1, stdin);
        assert(s != NULL);
        //清除最后一个\n
        lineCommand[strlen(lineCommand) - 1] = '\0';

        //字符串切割
        myargv[0] = strtok(lineCommand, " ");
        int i = 1;

        //如果没有字串 strtok->NULL myargc[end] == NULL
        while(myargv[i++] = strtok(NULL, " "));

        if(myargv[0] != NULL && strcmp(myargv[0], "ls") == 0){
            myargv[i++] = "--color=auto";
        }
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
            execvp(myargv[0], myargv);
            exit(1);
        }

        int status = 0;
        pid_t ret = waitpid(id, &status, 0);
        assert(ret > 0);
        lastCode = (status >> 8) & 0xFF;
        lastSig = status & 0x7F;
    }
    return 0;
}
