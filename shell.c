#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXCOMMAND 1024
#define MAXARGS 20
#define MAXPATH 100
#define MAXNAME 20

char *yerac(char *name)
{
    char *x=(char*)malloc(strlen(name)+1);
    strcpy(x,name);
    return x;
}

int pipeControl(char **args)
{
    int i=-1,j=0;
    while(args[++i] && args[i][0]!='|');
    if(args[i]) // pipe var
    {
        char *args2[MAXARGS]; // pipedan sonraki komutlar

        free(args[i]);
        args[i]=NULL;

        while(args[++i])
            args2[j++]=args[i];

        args2[j]=NULL;

        int pipet[2];
        pid_t p1,p2;

        pipe(pipet);

        if(fork()==0) // birinci komut
        {
            close(pipet[0]);
            dup2(pipet[1],STDOUT_FILENO);
            close(pipet[1]);

            if(execvp(args[0],args)<0)
            {
                printf("Hatali komut girdiniz!");
                exit(0);
            }
        }
        else if(fork()==0) // ikinci komut
        {
            close(pipet[1]);
            dup2(pipet[0],STDOUT_FILENO);
            close(pipet[0]);

            if(execvp(args2[0],args2)<0)
            {
                printf("Hatali komut girdiniz!");
                exit(0);
            }
        }
        else
        {
            wait(NULL);
            wait(NULL);
        }
        return 1;
    }

    return 0;
}


void calistir(char **args)
{
    if(!strcmp(args[0],"exit")) // exit yazilirsa parent process sonlandirilir. program kapanir.
        exit(1);

    if(!strcmp(args[0],"cd"))   // unistd kutuphanesinde bulunan fonksiyon ile
    {                           // calisan path degistirilebilir.
        chdir(args[1]);
        return;
    }


    if(pipeControl(args))
        return;

    if(fork()==0)
    {
        if(execvp(args[0],args)<0)
        {
            printf("Hatali komut girdiniz!\n");
            exit(0);
        }
    }
    wait(NULL); // child process tamamlanmasi icin beklenir

}

int main()
{
    char command[MAXCOMMAND];
    char *para;
    char path[MAXPATH];
    char user[MAXNAME];
    char *args[MAXARGS];
    int i,j;
    while(1)
    {
        i=0; // parametre sayisini tutar
        //[1;31 kirmizi
        getcwd(path,sizeof(path)); // unistd.h daki fonksiyon ile calisan path alinir.
        getlogin_r(user,sizeof(user)); //
        printf("\033[1;32m%s:",user); //user'in kirmizi gorunmesi icin
        printf("\033[01;33m%s$ ",path);
        printf("\033[0m"); // pathten sonra rengi resetlemek icin

        fgets(command,MAXCOMMAND,stdin); // komut okunur

        para=strtok(command," \n\t"); // komut ve parametreleri ayrilir

        if(para) // komut girilmisse
        {
            while(para)
            {
                args[i]=yerac(para); // her paramaetre args string dizisine atilir
                i++;
                para=strtok(NULL," \n\t");
            }

            args[i]=NULL; // execvp son arguman NULL olsun diye

            calistir(args);

            for(j=0;j<i;j++) // argumanlar icin aclian yerler temizlenir
                free(args[i]);
        }
    }
    return 0;
}
