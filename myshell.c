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

        waitpid(id, NULL, 0);
    }
    return 0;
}
