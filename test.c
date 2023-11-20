/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdanchev <mdanchev@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/16 19:08:15 by mdanchev          #+#    #+#             */
/*   Updated: 2023/11/16 19:08:15 by mdanchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "microshell.h"

int	error_(char *s1, char *s2)
{
	int		i;

	i = 0;
	if (s1)
	{
		while (s1[i])
		{
			write(2, &s1[i], 1);
			i++;
		}
	}
	i = 0;
	if (s2)
	{
		while (s2[i])
		{
			write(2, &s2[i], 1);
			i++;
		}
	}
	write(2, "\n", 1);
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

int exec(char **av, char **env, int i)
{
	int		fd[2];
	int		status;
	int		has_pipe;
	int		pid;

	has_pipe = av[i] && !strcmp(av[i], "|");
	if (has_pipe && pipe(fd) < 0)
		return (error_("error: fatal", NULL));
	pid = fork();
	if (pid < 0)
		return (error_("error: fatal", NULL));
	else if (pid == 0)
	{
		av[i] = 0;
		if (has_pipe && (close(fd[0]) < 0 || dup2(fd[1], STDOUT_FILENO) < 0 || close(fd[1]) < 0))
			return (error_("error: fatal", NULL));
		execve(*av, av, env);
		return (error_("error: cannot execute ", *av));
	}
	waitpid(pid, &status, 0);
	if (has_pipe && (close(fd[1]) < 0 || dup2(fd[0], STDIN_FILENO) < 0 || close(fd[0]) < 0))
		return (error_("error: fatal", NULL));
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		return (WTERMSIG(status));
	return (0);
}

int main(int ac, char **av, char **env)
{
	int    i = 0;
	int    status = 0;

	if (ac > 1)
	{
		while (av[i] && av[++i])
		{
			av += i;
			i = 0;
			while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
				i++;
			if (!strcmp(*av, "cd"))
				status = cd_(av, i);
			else if (i)
				status = exec(av, env, i);
		}
	}
	return status;
}