/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdanchev <mdanchev@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/21 16:56:45 by mdanchev          #+#    #+#             */
/*   Updated: 2023/11/21 16:57:53 by mdanchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "microshell.h"

int error_(char *s1, char *s2)
{
	int 	i;

	i = 0;
	while(s1 && s1[i])
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
	return (1);
}

int cd_(char **av, int i)
{
	if (i != 2)
		return (error_("error: cd: bad arguments", 0));
	if (chdir(av[1]) < 0)
		return (error_("error: cd: cannot change directory to ", av[1]));
	return (0);
}

/*
 * close_:
 * If an error occurs during the closing operation
 * I print an error message and stop the program
 * as instructed in the requirements.
 *
 * The file descriptor is passed by address to avoid
 * any confusion, although it could also be passed by value.
 *
 * The exit() function is used to terminate the program and
 * it automatically closes all opened file descriptors.
 */

void	close_(int *fd)
{
	if (close(*fd) < 0)
	{
		error_("error: fatal", 0);
		exit(1);
	}
}

/*
 * dup2_:
 * If an error occurs during the dup2 operation
 * I print an error message and stop the program
 * as instructed in the requirements.
 *
 * I pass the file descriptor by value because sometimes
 * I will pass to this function STDIN_FILENO and
 * STDOUT_FILENO as arguments and these are macros
 * (so they don't have adresses).
 *
 * The exit() function is used to terminate the program and
 * it automatically closes all opened file descriptors.
 */

void	dup2_(int oldfd, int newfd)
{
	if (dup2(oldfd, newfd) < 0)
	{
		error_("error: fatal", 0);
		exit(1);
	}
}

/*
 * before_pipe:
 * If pid < 0, an error occurred during fork(). I print an
 * error message and call exit(). The exit() function will
 * automatically close all open descriptors. Nevertheless,
 * it's considered good practice to manually close opened
 * file descriptors like (in this case fd[0], fd[1], and tmp_fd)
 * to ensure resource cleanup and prevent potential resource leaks.
 */

int	before_pipe(char **av, char **env, int i, int *tmp_fd)
{
	int fd[2], pid;

	if (pipe(fd) < 0)
	{
		error_("error: fatal", 0);
		exit(1);
	}
	pid = fork();
	if (pid < 0)
	{
		error_("error: fatal", 0);
		exit(1);
	}
	else if (pid == 0)
	{
		av[i] = 0;
		close_(&fd[0]);
		dup2_(fd[1], STDOUT_FILENO);
		close_(&fd[1]);
		dup2_(*tmp_fd, STDIN_FILENO);
		close_(tmp_fd);
		execve(*av, av, env);
		error_("error: cannot execute ", *av);
	}
	else
	{
		close_(&fd[1]);
		close_(tmp_fd);
		dup2_(fd[0], *tmp_fd);
		waitpid(pid, NULL, WUNTRACED);
	}
	return (0);
}

int after_pipe(char **av, char **env, int i, int *tmp_fd)
{
	int pid, status;

	pid = fork();
	if (pid < 0)
	{
		error_("error: fatal", 0);
		exit(1);
	}
	else if (pid == 0)
	{
		av[i] = 0;
		dup2_(*tmp_fd, STDIN_FILENO);
		close_(	tmp_fd);
		execve(*av, av, env);
		error_("error: cannot execute ", *av);
		if (errno == ENOEXEC)
			exit(126);
		exit (127);
	}
	else
	{
		waitpid(pid, &status, WUNTRACED);
		dup2_(STDIN_FILENO, *tmp_fd);
		if (WIFEXITED(status))
			return (WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			return (WTERMSIG(status) + 128);
	}
	return (1);
}

int is_descriptor_open(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags == -1)
		return -1;
	return (flags);
}

int commands(char **av, char **env)
{
	int i, status, tmp_fd;

	i = 0;
	tmp_fd = dup(STDIN_FILENO);
	if (tmp_fd < 0)
	{
		error_("error: fatal", 0);
		exit(1);
	}
	while (av[i] && av[++i])
	{
		av += i;
		i = 0;
		while (av[i] && strcmp(av[i], "|") != 0 && strcmp(av[i], ";") != 0)
			i++;
		if (strcmp(*av, "cd") == 0)
			status = cd_(av, i);
		else if (i != 0 && av[i] && strcmp(av[i], "|") == 0)
			before_pipe(av, env, i, &tmp_fd);
		else if (i != 0 && (av[i] == NULL || strcmp(av[i], ";") == 0))
			status = after_pipe(av, env, i, &tmp_fd);
	}
	close(tmp_fd);
	return (status);
}

int	main(int ac, char **av, char **env)
{
	if (ac <= 1)
		return (0);
	return (commands(av, env));
}
