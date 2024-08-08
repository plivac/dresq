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

struct writer{
	const char *fname;
	char *mem;
	int fd;
	int in;
	int intot;
	int toflush;
	int flushcap;
	inline void open(char *_fname,int pages,int _flushcap){
		fname=_fname;
		fd=open(fname,O_RDONLY);
		if(fd<0){
			printf("error while opening fd for \"%s\"\n",fname);
			exit(1);
		}
		if(allocpages(&mem,pages)<0){
			printf("error while allocating pages for \"%s\"\n",fname);
			exit(1);
		}
		intot=4096*pages;
		flushcap=_flushcap;
	}
	inline void _sw(int L){
		int offset=0;
		while(L>0){
			int wt=write(fd,mem+offset,L);
			if(wt<0){
				if(errno==EINTR)continue;
				printf("error while writing to \"%s\"\n",fname);
				exit(1);
			}
			offset+=wt;
			L-=wt;
			toflush+=wt;
			if(toflush>=flushcap){
				toflush=0;
				if(fdatasync(fd)<0){
					printf("error while syncing data for \"%s\"\n",fname);
					exit(1);
				}
			}
		}
	}
	void write(char *bf,int len){
		while(in+len>=intot){
			memcpy(mem+in,bf,intot-in);
			_sw(intot);
			bf+=intot-in;
			len-=intot-in;
			in=0;
		}
		if(len>0)
			memcpy(mem+in,bf,len);
	}
	inline void writenumber(int64_t number,char symbol=0){
		char d[24]={symbol};
		sprintf(d+(symbol!=0),"%ld.",number);
		write(d,strlen(d));
	}
	inline void finish(){
		_sw(in);
		close(fd);
	}
};

char page[4096];
int main(int argc,char **args){
	if(argc!=3){
		printf("invalid number of arguments\n");
		exit(1);
	}
	writer fw;
	fw.open(args[2],1,1);
	int t=atoi(args[1]);
	switch(t){
		case 1:{
			for(int i=0;i<2000;++i)
				page[i]='a';
			for(int i=2000;i<3870;i+=11){
				page[i]='b';
				page[i+1]='b';
				page[i+2]='b';
				page[i+3]='b';
				page[i+4]=200;
				page[i+5]='b';
				page[i+6]='b';
				page[i+7]='b';
				page[i+8]='b';
				page[i+9]='b';
				page[i+10]=200;
			}
			for(int i=3870;i<4096;++i)
				page[i]=0;
			fw.write(page,4096);
			for(int i=0;i<1000;++i)
				page[i]=200;
			for(int i=1000;i<2000;i+=5){
				page[i]='b';
				page[i+1]='b';
				page[i+2]='b';
				page[i+3]='b';
				page[i+4]=200;
			}
			for(int i=2000;i<3500;i+=6){
				page[i]='b';
				page[i+1]='b';
				page[i+2]='b';
				page[i+3]='b';
				page[i+4]=200;
				page[i+5]=200;
			}
			for(int i=3500;i<4096;++i)
				page[i]='a';
			fw.write(page,4096);
			for(int i=0;i<1000;++i)
				page[i]=201;
			for(int i=1000;i<3000;++i)
				page[i]='c';
			for(int i=3000;i<4096;++i)
				page[i]=202;
			fw.write(page,4096);
			for(int i=0;i<4096;++i)
				page[i]='d';
			fw.write(page,4096);
		}break;
		case 2:{
			printf("this version is not implemented yet\n");
		}break;
		default:break;
	}
	fw.close();
	return 0;
}
