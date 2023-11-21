#include<unistd.h>
# include <string.h>
# include <errno.h>

int error_(char *s1, char *s2)
{
    int i;

    i = 0;
    while (s1 && s1[i])
    {
        write(2, &s1[i], 1);
        i++;
    }
    i = 0;
    while (s2 && s2[i])
    {
        write(2, &s2[i], 1);
        i++;
    }
    write(2, "\n", 1);
}

int     cd_(char **av, int i)
{
    if (i != 2)
        return (error_("error: cd: bad arguments", NULL));
    if (chdir(av[1]) < 0)
        return (error_("error: cd: cannot change directory to "), av[1]);
    return (0);
}

void    before_pipe(char **av, char **env, int i, int *tmp_fd)
{
    int fd[2];
    int pid;

    if (pipe(fd) < 0)
        return (error_("error: fatal", NULL);
    pid = fork();
    if (pid < 0)
    {
        close(fd[0]);
        close(fd[1]);
        return (error_("error: fatal", NULL);
    }
    else if (pid == 0)
    {
        av[i] = 0;
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        dup2(*tmp_fd, STDIN_FILENO);
        close(*tmp_fd);
        execve(*av, av, env);
        error_("error: cannot execute ", *av);
    }
    else
    {
        close(fd[1]);
        dup2(fd[0], *tmp_fd);
        close(fd[0]);
        waitpid(pid, NULL, 0);
    }
    return (0);
}

int     after_pipe(char **av, char **env, int i, int *tmp_fd)
{
    int pid;
    int status;

    pid = fork(0);
    if (pid < 0)
    {
        close(fd[0]);
        close(fd[1]);
        return (error_("error: fatal", NULL);
    }
    else if (pid == 0)
    {
        av[i] = NULL;
        dup2(*tmp_fd, STDIN_FILENO);
        close(*tmp_fd);
        execve(*av, av, env);
        error_("error: cannot execute ", *av);
        if (errno == ENOEXEC)
            return (126);
        return (127);
    }
    else
    {
        waitpid(pid, &status, 0);
        status = status_(status);
        dup2(STDIN_FILENO, *tmp_fd);
    }
    return (status);
}

int commands(char **av, char **env)
{
    int status;
    int i;
    int tmp_fd;

    i = 0;
    tmp_fd = dup(STDIN_FILENO);
    while (av[i] && av[++i])
    {
        av += i;
        i = 0;
        while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
            i++;
        if (!strcmp(*av, "cd"))
            status = cd_(av, i);
        else if (i != 0 && av[i] && !strcmp(av[i], "|"))
            before_pipe(av, env, i, &tmp_fd);
        else if (i != 0 && (av[i] == NULL || !strcmp(av[i], ";")))
            status = after_pipe(av, env, i, &tmp_fd);
    }
    close(tmp_fd);
    return (status);
}

int main(int ac, char **av, char **env)
{
    if (ac <= 1)
        return (0);
    return (commands(av, env));
}