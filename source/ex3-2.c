#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int checkFileFifo(char *str, char *splitPara);

int main(int argc, char *argv[]){
	int fd, n;
	char buf[256];
	struct stat fsbuf;
	mode_t mode;
	int kind;
	int PermCheck;
	int isThisFifo;
	char *fileStr;
	
	// 환경변수 COM_FILE의 값(파일명)을 fileStr변수에 저장
	fileStr = getenv("COM_FILE");
	
	// 실행인자를 받아 각 인자에 알맞게 모드를 설정
	if(argc == 1){ // 기본 (일반파일모드)
		if(fileStr == NULL) fileStr = "data.txt";
		isThisFifo = 0;
	}
	else if((argc == 3) && (strcmp(argv[1], "-t") == 0)){
		if(strcmp(argv[2], "f") == 0){ // fifo 모드
			if(fileStr == NULL) fileStr = "data.fifo";
			isThisFifo = 1;

		}
		else if(strcmp(argv[2], "r") == 0){ // 일반 파일 모드
			if(fileStr == NULL) fileStr = "data.txt";
			isThisFifo = 0;
		}
		else{ // 잘못된 인자 입력시 오류 반환
			perror("Wrong Argument\n");
			exit(1);
		}
	}
	else if((argc == 3) && (strcmp(argv[1], "-f") == 0)){ // 사용자 파일명 지정 모드
		fileStr = argv[2]; 
		if(access(fileStr, F_OK) == -1){	
			isThisFifo = checkFileFifo(fileStr, ".");
		}
		else{
			isThisFifo = -1;
		}
	}
	else{ // 잘못된 인자 입력시 오류 반환
		perror("Wrong Argument\n");
		exit(1);
	}
	
	// fileStr이 가리키는 파일명을 가지고 있는 파일이 없으면 그 파일을 사용자만 읽고쓸수 있도록 생성후 열기
	// fileStr이 가리키는 파일명을 가지고 있는 파일이 있으면 그 파일을 열기
	mode = S_IRUSR | S_IWUSR;	
	switch(isThisFifo){
		case 1:
			if((access(fileStr, F_OK) == -1) && (mkfifo(fileStr, mode) == -1)){
				perror("mkfifo\n");
				exit(1);
			}
		default:
			if((fd = open(fileStr, O_CREAT | O_WRONLY | O_TRUNC, mode)) == -1){
				printf("Open error\n");
				exit(1);	
			}
	}
	
	// 연 파일이 어떤 종류의 파일인지 확인하여 설정된 모드에 맞는 종류의 파일이 아닐경우 오류를 반환
	lstat(fileStr, &fsbuf);
	kind = fsbuf.st_mode & S_IFMT;
	switch(isThisFifo){
		case 1:
			if(kind != S_IFIFO){
				perror("this is not fifo file!\n");
				exit(1);
			}
			break;
		case 0:
			if(kind != S_IFREG){
				perror("this is not Reg file\n");
				exit(1);
			}
			break;
		default:
			if((kind != S_IFIFO) && (kind != S_IFREG)){
				perror("this is not fifo or Reg file\n");
				exit(1);
			}
			else{
				if(kind == S_IFIFO){
					isThisFifo = 1;
				}
				else{
					isThisFifo = 0;
				}
			}
	}
	
	// 연 파일의 권한을 확인하여 group과 other에 읽기, 쓰기 권한이 부여되어 있을경우 오류반환
	PermCheck = ((S_IREAD | S_IWRITE) >> 3) | ((S_IREAD | S_IWRITE) >> 6);
	if((fsbuf.st_mode & PermCheck) != 0){
		printf("%s must be protected\n", fileStr);
		exit(1);
	}
	
	// 파일을 이용한 통신을 위한 로직을 반복적으로 실행
	while(1){
		write(1, ">> ", 3);
		n = read(0, buf, 255);
		buf[n] = '\0';
		if(n > 0){
			if(write(fd, buf, n) != n){
				perror("Write error");
			}
		} else if (n == -1){
			perror("Read error");
		}
		if(n == 1 && buf[0] == 'q'){
			write(1, "Terminate\n", 10);
			break;
		}
		write(1, buf, n);
	}
	close(fd);

	return 0;
}

// 사용자 파일명 지정 모드에서 파일의 이름을 통해 파일의 종류를 파악하여 알맞은 숫자를 반환하는 함수
int checkFileFifo(char *str, char *splitPara){
	char *strUseCopy = (char *)malloc(strlen(str)+1);
	strcpy(strUseCopy, str);
	char *temp = (char *)malloc(strlen(str)+1);
	char *ptr2 = strtok(strUseCopy, splitPara);
	while(ptr2 != NULL){
		strcpy(temp, ptr2);
		ptr2 = strtok(NULL, splitPara);
	}
	if(strcmp(temp, "fifo") == 0){
		free(strUseCopy);
		free(temp);
		return 1;
	}
	else{
		free(strUseCopy);
		free(temp);
		return 0;
	}
}
