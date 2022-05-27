#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int	ft_strlen(char *str)
{
	int i = 0;
	
	while (str[i])
		i++;
	return (i);
}

void	fatal(void)
{
	char *error = "error: fatal\n";

	write(2, error, ft_strlen(error));
	exit(EXIT_FAILURE);
}

void	cd(char **cmd)
{
	char *error_arg = "error: cd: bad arguments\n";
	char *error_dir = "error: cd: cannot change to ";
	int i = 0;

	while (cmd[i])
		i++;
	if (i != 2)
		write(2, error_arg, ft_strlen(error_arg));
	else if (chdir(cmd[1]) == -1)
	{
		write(2, error_dir, ft_strlen(error_dir));
		write(2, cmd[1], ft_strlen(cmd[1]));
		write(2, "\n", 1);
	}
}

void	execute_child(char **cmd, char **env)
{
	char *error = "error: cannot execute ";

	execve(cmd[0], cmd, env);
	write(2, error, ft_strlen(error));
	write(2, cmd[0], ft_strlen(cmd[0]));
	write(2, "\n", 1);
}

void	execute(char **cmd, char **env)
{
	int i, j, last;
	int readFrom, writeTo, dupIn, dupOut;
	int pid, fd[2];

	j = 0;
	while (cmd[j] && strcmp(cmd[j], "|"))
		j++;
	if (!cmd[j] && !strcmp(cmd[0], "cd"))
	{
		cd(cmd);
		return ;
	}
	j = i = last = 0;
	if ((dupIn = dup(STDIN_FILENO)) == -1)
		fatal();
	if ((dupOut = dup(STDOUT_FILENO)) == -1)
		fatal();
	if ((readFrom = dup(STDIN_FILENO)) == -1)
		fatal();
	while (!last)
	{
		while(cmd[j] && strcmp(cmd[j], "|"))
			j++;
		if (!cmd[j])
			last = 1;
		cmd[j] = NULL;
		if (pipe(fd) == -1)
			fatal();
		if (last)
			writeTo = dupOut;
		else
			writeTo = fd[1];
		if (dup2(writeTo, STDOUT_FILENO) == -1)
			fatal();
		close(writeTo);
		if (dup2(readFrom, STDIN_FILENO) == -1)
			fatal();
		close(readFrom);
		if ((pid = fork()) == -1)
			fatal();
		else if (!pid)
			execute_child(&cmd[i], env);
		else
			readFrom = fd[0];
		i = ++j;
	}
	while(waitpid(pid, 0, 0) != -1)
		;
	if (dup2(dupIn, STDIN_FILENO) == -1)
		fatal();
	close(dupIn);
	close(readFrom);
	close(fd[1]);
}

int	main(int argc, char **argv, char **envp)
{
	int i, j;

	i = j = 1;
	while (i < argc)
	{
		while (argv[j] && strcmp(argv[j], ";"))
			j++;
		argv[j] = NULL;
		if (argv[i])
			execute(&argv[i], envp);
		i = ++j;
	}
}
