#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

void lsh_loop();
char * lsh_read_line();
char ** lsh_split_line(char *);
int lsh_execute(char **);

// command buildin
int lsh_cd(char ** args);
int lsh_help(char ** args);
int lsh_exit(char ** args);

int main(int argc, char **argv) {
    // load config file if any

    lsh_loop();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}

void lsh_loop() {
    // 这里面是命令循环
    char *line; //读取命令
    char **args; // 解析命令
    int status;
    do {
        printf("> ");
        line = lsh_read_line();
        // printf("you have inputed is %s\n", line);
        args = lsh_split_line(line);
        for (int i = 0;i < sizeof(args) / sizeof(char*);i++) {
            printf("%s\n", args[i]);
        }
        status = lsh_execute(args);
    } while (status);
}

char* lsh_read_line(void) {
    int size = LSH_RL_BUFSIZE;
    char *buffer = (char*)malloc(sizeof(char) * size);
    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    int c;
    int position = 0;
    while(1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position++] = '\0';
            return buffer;
        }
        buffer[position++] = (char)c;
        if (position >= size) {
            size += LSH_RL_BUFSIZE;
            buffer = (char *)realloc(buffer, size);
            if (!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **lsh_split_line(char *line) {
    int size = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = (char **)malloc(sizeof(char *) * size);
    char *token;
    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM);
    while (token) {
        // printf("%s\n", token);
        tokens[position] = token;
        position++;

        if (position >= size) {
            size += LSH_TOK_BUFSIZE;
            tokens = (char **)realloc(tokens, size * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char ** args) {
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
    } else if (pid < 0) {
        // error in forking
        perror("lsh");
    } else {
        // this is parent progress
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

// list of builtin commands, followed by their corresponding functions
char *builtin_str[] = {
        "cd",
        "help",
        "exit"
};

// I am confused with this part
int (*builtin_func[]) (char **) = {
        &lsh_cd,
        &lsh_help,
        &lsh_exit
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


/*
  Builtin function implementations.
*/
int lsh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args)
{
    int i;
    printf("Stephen Brennan's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char **args)
{
    return 0;
}

int lsh_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}

