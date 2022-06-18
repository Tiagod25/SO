#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int pids[10];
int ords[10];
char *coms[10];
int tempo_inat = 0;
int tempo_exec = 0;
int contador = 0;

int readlineFich(char *buffer, int size, char *path)
{
    int fd = open(path, O_RDONLY);
    int i = 0;
    char c;
    int bytes = 0;
    if (buffer == NULL || size == 0)
        return 0;
    while ((bytes = read(fd, &c, 1)) != 0 && i < size - 1)
    {
        if (c == '\n')
        {
            buffer[i] = 0;
            return i;
        }
        buffer[i++] = c;
    }
    if (bytes == 0)
    {
        return -1;
    }
    buffer[i] = '\0';
    return i;
}

int countChars(char *s, char c)
{
    return *s == '\0'
               ? 0
               : countChars(s + 1, c) + (*s == c);
}

int getPid(int numero)
{
    for (int i = 0; i < 10; i++)
    {
        if (ords[i] == numero)
        {
            return pids[i];
        }
    }
    return -1;
}
int getPos(int numero)
{
    for (int i = 0; i < 10; i++)
    {
        if (ords[i] == numero)
        {
            return i;
        }
    }
    return -1;
}

int firstFree()
{
    for (int i = 0; i < 10; i++)
    {
        if (ords[i] == -1)
            return i;
    }
    return -1;
}

void removeTarefa(int posicao)
{
    ords[posicao] = -1;
}

int main()
{
    char buf[100];
    //mkfifo("/home/tiago/server_to_client",0666);
    //int escrita=open("/home/tiago/server_to_client",O_WRONLY);
    //char directory_named_pipe[] = "/home/tiago/named_pipe";
    //char directory_logs[] = "/mnt/c/users/joao sousa/desktop/projetoSO/log.txt";
    char directory_named_pipe[] = "/home/rjsf/named_pipe";
    char directory_logs[] = "/mnt/c/Users/rjsf3/Desktop/MIEI/SO/1920/Sistemas-Operativos/log.txt";
    int fd = open(directory_named_pipe, O_RDONLY);
    int a = open(directory_logs, O_WRONLY | O_APPEND);
    if (a == -1)
    {
        a = open(directory_logs, O_WRONLY | O_APPEND | O_CREAT);
    }
    //int leituraLog=open("log.txt",O_RDONLY);
    int bytes;
    while ((bytes = readlineFich(buf, 100, directory_named_pipe)) > 0)
    {
        //ver primeira palavra, se é executar,tempo....
        char copia[100];
        //printf("%s\n",&(buf[0]));
        strncpy(copia, buf, sizeof(buf));
        char *token = strtok(buf, " ");
        if (strcmp(token, "tempo-inactividade") == 0 || strcmp(token, "tempo-execucao") == 0)
        {
            char *tempo = strtok(NULL, " ");
            token = strtok(buf, "-");
            token = strtok(NULL, "-");
            int deadline = atoi(tempo);
            if (strcmp(token, "inactividade") == 0)
            {
                tempo_inat = deadline;

                printf("Tempo inactividade = %d\n", tempo_inat);
                //write(1, "Tempo inactividade = ", 22);
                //write(1, &tempo_inat, 5);
            }
            else if (strcmp(token, "execucao") == 0)
            {
                tempo_exec = deadline;
                printf("Tempo exec = %d\n", tempo_exec);
                //write(1, "Tempo exec = ", 14);
                //write(1, &tempo_exec, 5);
            }
        }
        else if (strcmp(token, "executar") == 0)
        {
            int ocorrencias = countChars(&(copia[0]), '|') + 1;
            char *listaComandos[ocorrencias];
            char *comandos = strtok(&(copia[0]), "|");
            int i = 0;
            while (comandos != NULL)
            {
                //printf("%s\n",comandos);
                listaComandos[i] = comandos;
                comandos = strtok(NULL, "|");
                i++;
            }
            //printf("%d",ocorrencias);
            if (ocorrencias == 1)
            {
                int espacos = countChars(listaComandos[0], ' ');
                strtok(listaComandos[0], " ");
                char *comandoExec[espacos + 1];
                comandoExec[espacos] = NULL;
                char *cmd2;
                int temp = 0;
                while ((cmd2 = strtok(NULL, " ")) != NULL)
                {
                    printf("%s\n", cmd2);
                    comandoExec[temp++] = cmd2;
                }
                printf("%d\n", espacos);
                execvp(comandoExec[0], comandoExec);
            }
            else
            {
                int fd[ocorrencias - 1][2];
                for (int m = 0; m < (ocorrencias - 1); m++)
                {
                    pipe(fd[m]);
                }
                for (int flag = 0; flag < ocorrencias; flag++)
                {
                    if (flag == 0)
                    {
                        int forkval = fork();
                        if (forkval == 0)
                        {
                            dup2(fd[0][1], 1);
                            close(fd[0][0]);
                            int espacos = countChars(listaComandos[flag], ' ');
                            strtok(listaComandos[0], " ");
                            char *comandoExec[espacos + 1];
                            comandoExec[espacos] = NULL;
                            char *cmd2;
                            int temp = 0;
                            while ((cmd2 = strtok(NULL, " ")) != NULL)
                            {
                                comandoExec[temp++] = cmd2;
                            }
                            execvp(comandoExec[0], comandoExec);
                            close(fd[0][1]);
                            _exit(0);
                        }
                        close(fd[0][1]);
                    }
                    else if (flag == ocorrencias - 1)
                    {
                        int forkval = fork();
                        if (forkval == 0)
                        {
                            dup2(fd[flag - 1][0], 0);
                            close(fd[flag - 1][1]);
                            char buffer[100];
                            int espacos = countChars(listaComandos[flag], ' ');
                            char *comandoExec[espacos + 1];
                            if (espacos == 0)
                            {
                                comandoExec[0] = listaComandos[flag];
                                comandoExec[1] = NULL;
                            }
                            else
                            {
                                comandoExec[0] = strtok(listaComandos[flag], " ");
                                comandoExec[espacos] = NULL;
                                int pos = 1;
                                char *cmd1 = strtok(NULL, " ");
                                while (cmd1 != NULL)
                                {
                                    comandoExec[pos++] = cmd1;
                                    cmd1 = strtok(NULL, " ");
                                }
                            }
                            //execlp("wc","wc","-l",NULL);
                            execvp(comandoExec[0], comandoExec);
                            close(fd[flag - 1][0]);
                            exit(0);
                        }
                    }
                    else
                    {
                        int forkval = fork();
                        if (forkval == 0)
                        {
                            dup2(fd[flag - 1][0], 0);
                            dup2(fd[flag][1], 1);
                            close(fd[flag - 1][1]);
                            close(fd[flag][0]);
                            char buffer[100];
                            int espacos = countChars(listaComandos[flag], ' ');
                            char *comandoExec[espacos + 1];
                            if (espacos == 0)
                            {
                                comandoExec[0] = listaComandos[flag];
                                comandoExec[1] = NULL;
                            }
                            else
                            {
                                comandoExec[0] = strtok(listaComandos[flag], " ");
                                comandoExec[espacos] = NULL;
                                int pos = 1;
                                char *cmd1 = strtok(NULL, " ");
                                while (cmd1 != NULL)
                                {
                                    comandoExec[pos++] = cmd1;
                                    cmd1 = strtok(NULL, " ");
                                }
                            }
                            execvp(comandoExec[0], comandoExec);
                            close(fd[flag - 1][0]);
                            close(fd[flag][1]);
                            exit(0);
                        }
                        close(fd[flag - 1][0]);
                        close(fd[flag][1]);
                    }
                }
            }
        }
        else if (!strcmp(token, "terminar"))
        {
            char *charnum = strtok(NULL, " ");
            int numero = atoi(charnum);

            if (getPid(numero) > -1)
            {
                kill(getPid(numero), SIGKILL);
                int posicao = getPos(numero);
                ords[posicao] = -1;
                //char* info=coms[posicao];
                //write(escritaLog,info,sizeof(info));
            }
            else
            {
                char eof = '\n';
                write(a, charnum, 1);
                write(a, &eof, 1);
                write(1, "PROCESSO ESCOLHIDO NAO EXISTE\n", 30);
            }
        }
        else if (!strcmp(token, "listar"))
        {
            for (int i = 0; i < 10; i++)
            {
                if (ords[i] > -1)
                {
                    char *linha = coms[i];
                    //write(escrita,linha,sizeof(linha));
                    write(1, linha, sizeof(linha));
                }
            }
        }
        else if (!strcmp(token, "ajuda"))
        {
            char *ajuda = "tempo-inactividade nsegs\ntempo-execucao nsegs\nexecutar p1 | p2 ... | pn\n";
            //write(escrita,ajuda,sizeof(ajuda));
            //write(1,ajuda,50);
            //printf("tempo-inactividade nsegs\ntempo-execucao nsegs\nexecutar p1 | p2 ... | pn\n");
            write(1, "tempo-inactividade nsegs\ntempo-execucao nsegs\nexecutar p1 | p2 ... | pn\n", 72);
        }
    }
}
