#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define FILE_MAX (1 << 16)
#define CHUNK_SIZE 512
char *get_file_name(char *path)
{
	char *p = path + strlen(path);

	for (; p > path && *p != '/'; p--);

	return p > path ? p + 1 : path;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		perror("input format error");
		exit(EXIT_FAILURE);
	}	
	
	int fd1;
	if ((fd1 = open(argv[1], O_RDONLY)) < 0) {
		perror("open file1: error");
		exit(EXIT_FAILURE);
	}
	struct stat fd1_st;
	if (fstat(fd1, &fd1_st) < 0) {
		perror("open file1: error\n");
		exit(EXIT_FAILURE);
	}

	if ((fd1_st.st_mode & S_IFMT) == S_IFDIR) {
		perror("file1 is a directory\n");	
		exit(EXIT_FAILURE);
	}

	char filebuf[FILE_MAX];
	ssize_t byte_read;	
	memset(filebuf + byte_read, 0, FILE_MAX - byte_read);
	struct stat st;
	if (stat(argv[2], &st) == 0) {
		/* file2 existed */
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			/* file2 is a directory */
			char pathbuf[128];
			memset(pathbuf, 0, 128);
			strcpy(pathbuf, argv[2]);
			strcat(pathbuf, "/");
			strcat(pathbuf, get_file_name(argv[1]));
			//printf("pathbuf = %s\n", pathbuf);
			
			int fd2;
			if ((fd2 = open(pathbuf, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
				perror("open new file error");
				exit(EXIT_FAILURE);
			}

			struct stat fd2_st;
			if (fstat(fd2, &fd2_st) < 0) {
				perror("stat file2 error");
				exit(EXIT_FAILURE);	
			}
			
			if (fd1_st.st_ino == fd2_st.st_ino) {
				perror("the two files are linked\n");
				exit(EXIT_FAILURE);
			}

			if ((fd2 = open(pathbuf, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
				perror("open new file error");
				exit(EXIT_FAILURE);
			}
			while ((byte_read = read(fd1, filebuf, CHUNK_SIZE)) > 0) {
				if(write(fd2, filebuf, byte_read) < 0) {
					perror("write to file error");
					exit(EXIT_FAILURE);
				}	
			}
			close(fd2);

			
		} else {
			/* file2 is a file */
			if (st.st_ino == fd1_st.st_ino) {
				perror("the two files are linked\n");
				exit(EXIT_FAILURE);
			}

			int fd2;
			if ((fd2 = open(argv[2], O_RDWR | O_TRUNC)) < 0) {
				perror("open new file error");
				exit(EXIT_FAILURE);
			}
			while ((byte_read = read(fd1, filebuf, CHUNK_SIZE)) > 0) {
				if(write(fd2, filebuf, byte_read) < 0) {
					perror("write to file error");
					exit(EXIT_FAILURE);
				}	
			}

			close(fd2);
		}
	} else {
		/* file2 not existed */
		int fd2;
		if ((fd2 = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
			perror("open new file perror");
			exit(EXIT_FAILURE);
		}

		while ((byte_read = read(fd1, filebuf, CHUNK_SIZE)) > 0) {
			if(write(fd2, filebuf, byte_read) < 0) {
				perror("write to file error");
				exit(EXIT_FAILURE);
			}				
		}

		close(fd2);
	}	
	
	return 0;
}
