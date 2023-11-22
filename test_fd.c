/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_fd.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdanchev <mdanchev@42lausanne.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/21 11:36:37 by mdanchev          #+#    #+#             */
/*   Updated: 2023/11/21 11:36:37 by mdanchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "microshell.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>


// Si le descripteur est ouvert, la fonction renvoie 0, sinon elle renvoie -1

int is_descriptor_open(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags == -1)
		return -1;
	return (flags);
}

void	descriptors_points(int fd1, int fd2, char *s)
{
	struct stat stat1, stat2;
	if (fstat(fd1, &stat1) == -1 || fstat(fd2, &stat2) == -1)
	{
		perror("Erreur lors de l'obtention des informations sur le fichier");
		return ;
	}
	if (stat1.st_ino == stat2.st_ino)
		printf("fd1 et %s pointent vers le même fichier.\n", s);
	else
		printf("fd1 et %s pointent vers des fichiers différents.\n", s);
}

void	change_descriptor(int fd, int fd2)
{
	//(void)fd2;
	close(fd);
	dup2(fd2, fd);
}

int	main(void)
{
	int fd1, fd2, fd3, fd4;


	fd1 = open("./microshell.c", O_RDONLY);
	fd2 = fd1;
	fd3 = dup(fd1);
	fd4 = open("./mine", O_RDONLY);

	change_descriptor(fd4, fd1);

	printf("fd1 = %d\n", fd1);
	printf("fd2 = %d\n", fd2);
	printf("fd3 = %d\n", fd3);
	printf("fd4 = %d\n", fd4);


	descriptors_points(fd1, fd2, "fd2");
	descriptors_points(fd1, fd3, "fd3");
	descriptors_points(fd1, fd4, "fd4");

	printf("close fd1 = %d\n", close(fd1));
//	printf("close fd2 = %d\n", close(fd2));
//	printf("close fd3 = %d\n", close(fd3));
//	printf("close fd4 = %d\n", close(fd4));

	printf("fd1 est %s\n", is_descriptor_open(fd1) ? "fermé" : "ouvert");
	printf("fd2 est %s\n", is_descriptor_open(fd2) ? "fermé" : "ouvert");
	printf("fd3 est %s\n", is_descriptor_open(fd3) ? "fermé" : "ouvert");
	printf("fd4 est %s\n", is_descriptor_open(fd4) ? "fermé" : "ouvert");
}
