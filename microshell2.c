/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell2.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdanchev <mdanchev@42lausanne.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/21 09:56:34 by mdanchev          #+#    #+#             */
/*   Updated: 2023/11/21 13:46:50 by mdanchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "microshell.h"

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
	return (1);
}

int status_(int status)
{
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		return (WTERMSIG(status) + 128);
	return (1);
}

int		cd_(char **av, int i)
{
	if (i != 2)
		return (error_("error: cd: bad arguments", NULL));
	else if (chdir(av[1]) < 0)
		return (error_("error: cd: cannot change directory to ", av[1]));
	return (0);
}

int commands(char **av, char **env)
{
	int	i;
	int fd[2];
	int tmp_fd;
	int pid;
	int status;

	status = 0;
	i = 0;
	tmp_fd = dup(STDIN_FILENO);
	while (av[i] && av[++i])
	{
		av += i;
		i = 0;
		while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			i++;
		if (strcmp(*av, "cd") == 0)
			status = cd_(av, i);
		else if(i != 0 && av[i] && strcmp(av[i], "|") == 0)
		{
			pipe(fd);
			pid = fork();
			if (pid < 0)
			{
				close(fd[0]);
				close(fd[1]);
				return (error_("error: fatal", NULL));
			}
			else if (pid == 0)
			{
				av[i] = NULL;
				close(fd[0]);
				dup2(fd[1], STDOUT_FILENO);
				close(fd[1]);
				dup2(tmp_fd, STDIN_FILENO);
				close(tmp_fd);
				execve(av[0], av, env);
				return (error_("error: cannot execute ", av[0]));
			}
			else
			{
				close(fd[1]);
				dup2(fd[0], tmp_fd);
				close(fd[0]);
				waitpid(pid, NULL, 0);
			}
		}
		else if (i != 0 && (av[i] == NULL || strcmp(av[i], ";") == 0))
		{
			pid = fork();
			if (pid < 0)
			{
				close(fd[0]);
				close(fd[1]);
				return (error_("error: fatal", NULL));
			}
			else if (pid == 0)
			{
				av[i] = NULL;
				dup2(tmp_fd, STDIN_FILENO);
				close(tmp_fd);
				execve(*av, av, env);
				error_("error: cannot execute ", av[0]);
				if (errno == ENOEXEC)
					return (126);
				return (127);
			}
			else
			{
				waitpid(pid, &status, 0);
				status = status_(status);
				dup2(STDIN_FILENO, tmp_fd);
			}
		}
	}
	close(tmp_fd);
	return (status);

}

int	main(int ac, char **av, char **env)
{
	int status;

	if (ac <= 1)
		return (0);
	status = commands(av, env);
	return (status);
}