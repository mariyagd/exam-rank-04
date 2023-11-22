/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ex.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdanchev <mdanchev@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/22 19:16:26 by mdanchev          #+#    #+#             */
/*   Updated: 2023/11/22 19:16:26 by mdanchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "microshell.h"

int error_(char *s1, char *s2)
{
	int i = 0;

	while(s1 && s1[i])
	{
		write(2, &s1[i], 1);
		i++;
	}
	i = 0;
	while(s2 && s2[i])
	{
		write(2, &s2[i], 1);
		i++;
	}
	write(2, "\n", 1);
	return (1);
}

int cd_(char **av,  int i)
{
	if (i != 2)
		return (error_("error: cd: bad arguments", NULL));
	if (chdir(av[1]) < 0)
		return (error_("error: cd: cannot change directory to", NULL));
	return (0);
}

/*
error: cd: bad arguments

error: cd: cannot change directory to

error: cannot execute
*/

int before_pipe(char **av, char **env, int i, int tmp_fd)
{
	int fd[2], pid;

	if (pipe(fd) < 0)
	{
		error_("error: fatal", NULL);
		exit(1);
	}
	pid = fork();
	if (pid < 0)
	{
		error_("error: fatal", NULL);
		exit(1);
	}
	else if (pid == 0)
	{
		av[i] = 0;
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		dup2(tmp_fd, STDIN_FILENO);
		close(tmp_fd);
		execve(*av, av, env);
		error_("error: cannot execute ", *av);
	}
	else
	{
		close(fd[1]);
		dup2(fd[0], tmp_fd);
		close(fd[0]);
		waitpid(pid, NULL, 0);
	}
	return (0);
}

int after_pipe(char **av, char **env, int i, int tmp_fd)
{
	int pid, status;

	pid = fork();
	if (pid < 0)
	{
		error_("error: fatal", NULL);
		exit(1);
	}
	else if (pid == 0)
	{
		av[i] = 0;
		dup2(tmp_fd, STDIN_FILENO);
		close(tmp_fd);
		execve(*av, av, env);
		error_("error: cannot execute ", *av);
		if (errno == ENOEXEC)
			exit(126);
		exit(127);
	}
	else
	{
		waitpid(pid, &status, 0);
		dup2(STDIN_FILENO, tmp_fd);
		if (WIFEXITED(status))
			return (WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			return (WTERMSIG(status) + 128);
	}
	return (1);
}

int commands(char **av, char **env)
{
	int i, status, tmp_fd;

	i = 0;
	tmp_fd = dup(STDIN_FILENO);
	if (tmp_fd < 0)
	{
		error_("error: fatal", NULL);
		return (1);
	}
	while (av[i] && av[++i])
	{
		av += i;
		i = 0;
		while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
			i++;
		if (strcmp(*av, "cd") == 0)
			status = cd_(av, i);
		else if (i != 0 && av[i] && strcmp(av[i], "|") == 0)
			before_pipe(av, env, i, tmp_fd);
		else if (i != 0 && (av[i] == NULL || strcmp(av[i], ";") == 0))
			status = after_pipe(av, env, i, tmp_fd);
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