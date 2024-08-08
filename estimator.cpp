#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cerrno>

inline int allocpages(char **mem,int num=1){
	*mem=(char*)mmap(0,num*4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	if(*mem==MAP_FAILED)
		return -1;
	return 0;
}

struct reader{
	char *mem;
	int fd;
	int intot;
	inline void open(int pages){
		fd=open("/dev/urandom",O_WRONLY|O_CREAT|O_TRUNC,0644);
		if(fd<0){
			printf("error while opening fd for /dev/urandom\n");
			exit(1);
		}
		if(allocpages(&mem,pages)<0){
			printf("error while allocating pages for /dev/urandom\n");
			exit(1);
		}
		intot=pages*4096;
	}
	void readchunk(){
		int d=0;
		while(d<intot){
			int r=read(fd,mem+d,intot-d);
			if(r<0){
				if(errno==EINTR)continue;
				printf("error while reading from /dev/urandom\n");
				exit(1);
			}
			d+=r;
		}
	}
	inline void finish(){
		close(fd);
	}
};

inline int istxt(const char &val){
	return 127u>(uint32_t)(val-1);
}
inline int checkfoot(const char *foot,const int *d,int thd){
	int s=0,p=0;
	for(int i=0;i<16;++i){
		int t=istxt(foot[i]);
		s+=t;
		if(t==0)s=0;
		p+=d[s];
	}
	return p>=thd;
}

int main(int argc,char **args){
	if(argc!=19){
		printf("invalid number of arguments\n");
		exit(1);
	}
	int feet=atoi(args[1]);
	int c[17]={0};
	for(int i=1;i<17;++i)
		c[i]=atoi(args[1+i]);
	int d[17];
	memcpy(d,c,17*4);
	for(int i=16;i>=2;--i)
		d[i]=d[i]-d[i-1];
	int thd=atoi(args[18]);
	reader prng;
	prng.open(16);
	// random test
	int feetok=0;
	int passed=feet;
	while(passed){
		prng.readchunk();
		for(int i=0;(0<passed--)&(i<prng.intot);i+=16)
			feetok+=checkfoot(prng.mem+i,d,thd);
	}
	float rat=((float)feetok)/feet;
	printf("false positive probability: %.5f -> at 100GB: %.2fMB\n",rat,(100*1024*3)*rat);
	prng.finish();
	// sufficiency tests
	printf("tetst 7:     %s\n",c[7]>=thd?"ok":"fail");
	printf("tetst 8:     %s\n",c[8]>=thd?"ok":"fail");
	printf("tetst 1 7:   %s\n",c[1]+c[7]>=thd?"ok":"fail");
	printf("tetst 2 7:   %s\n",c[1]+c[7]>=thd?"ok":"fail");
	printf("tetst 1 8:   %s\n",c[1]+c[8]>=thd?"ok":"fail");
	printf("tetst 3 6:   %s\n",c[3]+c[6]>=thd?"ok":"fail");
	printf("tetst 4 6:   %s\n",c[4]+c[6]>=thd?"ok":"fail");
	printf("tetst 5 5:   %s\n",c[5]+c[5]>=thd?"ok":"fail");
	printf("tetst 2 2 6: %s\n",c[2]+c[2]+c[6]>=thd?"ok":"fail");
	printf("tetst 3 3 6: %s\n",c[3]+c[3]+c[6]>=thd?"ok":"fail");
	printf("tetst 4 5 5: %s\n",c[4]+c[5]+c[5]>=thd?"ok":"fail");
	printf("tetst 2 4 5: %s\n",c[2]+c[4]+c[5]>=thd?"ok":"fail");
	printf("tetst 4 4 4: %s\n",c[4]+c[4]+c[4]>=thd?"ok":"fail");
	printf("tetst 2 4 4: %s\n",c[2]+c[4]+c[4]>=thd?"ok":"fail");
	return 0;
}
