#include<stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
extern int errno;

struct user{
	int type;
	char name[50],pswd[20],name2[50];
	long amount;
	int account_no;
	bool status;
}; 

struct transaction{
	int account_no;
	int amount;
	struct tm time;
	bool debited;
	bool credited;
};

void lock(int fd,short ltype){
	struct flock lock;
	lock.l_type=ltype;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	fcntl(fd,F_SETLKW,&lock);
}

void unlock(int fd){
	struct flock lock;
	lock.l_type=F_UNLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	fcntl(fd,F_SETLK,&lock);
}

struct administrator{
		char ad_id[20];
		char pswd[20];
	};

int administrator_login(int nsd){
	struct administrator administrator_db;
	char ad_id[20],pswd[20];
	int fd;
	int nbytes;
	int ret;
	read(nsd,ad_id,sizeof(ad_id));
	read(nsd,pswd,sizeof(pswd));
	fd=open("administrator_db.txt",O_RDWR);
	lock(fd,F_RDLCK);
	while(nbytes=read(fd,&administrator_db,sizeof(administrator_db))){
		if(!strcmp(administrator_db.ad_id,ad_id) && !strcmp(administrator_db.pswd,pswd)){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			break;
		}
	}
	unlock(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
	}
	return ret;
}

int Add(int nsd){
	struct user u;
	int ret;
	read(nsd,&u.type,sizeof(u.type));
	if(u.type!=1 && u.type!=2){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	read(nsd,u.name,sizeof(u.name));
	if(u.type==2){
		read(nsd,u.name2,sizeof(u.name2));
	}
	read(nsd,&u.amount,sizeof(u.amount));
	read(nsd,u.pswd,sizeof(u.pswd));
	if(u.type==2){
		int fd=open("joint_account_user_db.txt",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,(-1)*sizeof(u),SEEK_END);
		struct user tmp;
		if(read(fd,&tmp,sizeof(tmp))){
			u.account_no=tmp.account_no+2;
		}
		else{
			u.account_no=10000;
		}
		lseek(fd,0,SEEK_END);
		u.status=true;
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("normal_user_db.txt",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,(-1)*sizeof(u),SEEK_END);
		struct user tmp;
		if(read(fd,&tmp,sizeof(tmp))){
			u.account_no=tmp.account_no+2;
		}
		else{
			u.account_no=10001;
		}
		lseek(fd,0,SEEK_END);
		u.status=true;
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	int fd=open("transactions_db.txt",O_RDWR);
	lock(fd,F_WRLCK);
	lseek(fd,0,SEEK_END);
	struct transaction tr;
	tr.account_no=u.account_no;
	tr.amount=u.amount;
	tr.debited=false;
	tr.credited=true;
	time_t t=time(NULL);
	struct tm *tm=localtime(&t);
	tr.time.tm_sec=tm->tm_sec;
	tr.time.tm_min=tm->tm_min;
	tr.time.tm_hour=tm->tm_hour;
	tr.time.tm_year=tm->tm_year;
	tr.time.tm_mon=tm->tm_mon;
	tr.time.tm_mday=tm->tm_mday;
	write(fd,&tr,sizeof(tr));
	unlock(fd);
	close(fd);
	ret=u.account_no;
	write(nsd,&ret,sizeof(ret));
	return ret;
}

int Search(int nsd){
	struct user u;
	int ret;
	int account_no;
	read(nsd,&account_no,sizeof(account_no));
	if(account_no & 1){
		int fd=open("normal_user_db.txt",O_RDONLY);
		lock(fd,F_RDLCK);
		while(read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				unlock(fd);
				close(fd);
				fd=open("transactions_db.txt",O_RDONLY);
				lock(fd,F_RDLCK);
				struct transaction tr;
				while(read(fd,&tr,sizeof(tr))){
					if(tr.account_no==account_no){
						write(nsd,&tr,sizeof(tr));	
					}
				}
				tr.account_no=0;
				write(nsd,&tr,sizeof(tr));	
				unlock(fd);
				close(fd);
				return 1;
			}
		}
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db.txt",O_RDONLY);
		lock(fd,F_RDLCK);
		while(read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				unlock(fd);
				close(fd);
				fd=open("transactions_db.txt",O_RDONLY);
				lock(fd,F_RDLCK);
				struct transaction tr;
				while(read(fd,&tr,sizeof(tr))){
					if(tr.account_no==account_no){
						write(nsd,&tr,sizeof(tr));	
					}
				}
				tr.account_no=0;
				write(nsd,&tr,sizeof(tr));	
				unlock(fd);
				close(fd);
				return 1;	
			}
		}
		unlock(fd);
		close(fd);
	}
	ret=0;
	write(nsd,&ret,sizeof(ret));
	return 0;
}

int Modify(int nsd){
	int type,ret,nbytes,nr=0;
	struct user u;
	int account_no;
	read(nsd,&account_no,sizeof(account_no));
	if(account_no & 1){
		int fd=open("normal_user_db.txt",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db.txt",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;	
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	read(nsd,&u,sizeof(u));
	if(account_no & 1){
		int fd=open("normal_user_db.txt",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		struct user u1;
		read(fd,&u1,sizeof(u1));
		lseek(fd,(-1)*sizeof(u),SEEK_CUR);
		if(strlen(u.name)==0){
			strcpy(u.name,u1.name);
		}
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db.txt",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		struct user u1;
		read(fd,&u1,sizeof(u1));
		lseek(fd,(-1)*sizeof(u),SEEK_CUR);
		if(strlen(u.name)==0){
			strcpy(u.name,u1.name);
		}
		if(strlen(u.name2)==0){
			strcpy(u.name2,u1.name2);
		}
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return 1;
}

int Delete(int nsd){
	int type,ret,nbytes,nr=0;
	struct user u;
	int account_no;
	read(nsd,&account_no,sizeof(account_no));
	if(account_no & 1){
		int fd=open("normal_user_db.txt",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db.txt",O_RDONLY);
		lock(fd,F_RDLCK);
		while(nbytes=read(fd,&u,sizeof(u))){
			if(u.account_no==account_no && u.status==true){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				write(nsd,&u,sizeof(u));
				break;	
			}
			nr++;
		}
		unlock(fd);
		close(fd);
	}
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	if(account_no & 1){
		int fd=open("normal_user_db.txt",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		read(fd,&u,sizeof(u));
		u.status=false;
		lseek(fd,nr*sizeof(u),SEEK_SET);
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	else{
		int fd=open("joint_account_user_db.txt",O_RDWR);
		lock(fd,F_WRLCK);
		lseek(fd,nr*sizeof(u),SEEK_SET);
		read(fd,&u,sizeof(u));
		u.status=false;
		lseek(fd,nr*sizeof(u),SEEK_SET);
		write(fd,&u,sizeof(u));
		unlock(fd);
		close(fd);
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return 1;
}

int login_user(int nsd){
	struct user u;
	int account_no;
	int fd;
	int nbytes;
	int ret,type;
	char pswd[20];
	read(nsd,&type,sizeof(type));
	read(nsd,&account_no,sizeof(account_no));
	read(nsd,pswd,sizeof(pswd));
	if(type==1)
		fd=open("normal_user_db.txt",O_RDONLY);
	else
		fd=open("joint_account_user_db.txt",O_RDONLY);
	lock(fd,F_RDLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && !strcmp(u.pswd,pswd) && u.status==true){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			break;
		}
	}
	unlock(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	return account_no;	
}

int Deposit(int nsd,int account_no){
	int amount,nbytes,ret;
	struct user u;
	struct transaction tr;
	read(nsd,&amount,sizeof(amount));
	int fd;
	if(account_no & 1)
		fd=open("normal_user_db.txt",O_RDWR);
	else
		fd=open("joint_account_user_db.txt",O_RDWR);
	lock(fd,F_WRLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true){
			int fd1=open("transactions_db.txt",O_RDWR);
			lock(fd1,F_WRLCK);
			lseek(fd1,0,SEEK_END);
			tr.account_no=u.account_no;
			tr.amount=amount;
			tr.debited=false;
			tr.credited=true;
			time_t t=time(NULL);
			struct tm *tm=localtime(&t);
			tr.time.tm_sec=tm->tm_sec;
			tr.time.tm_min=tm->tm_min;
			tr.time.tm_hour=tm->tm_hour;
			tr.time.tm_year=tm->tm_year;
			tr.time.tm_mon=tm->tm_mon;
			tr.time.tm_mday=tm->tm_mday;
			write(fd1,&tr,sizeof(tr));
			unlock(fd1);
			close(fd1);
			u.amount=u.amount+amount;
			lseek(fd,(-1)*sizeof(u),SEEK_CUR);
			write(fd,&u,sizeof(u));
			break;	
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return ret;
}

int Withdraw(int nsd,int account_no){
	int amount,nbytes,ret;
	struct user u;
	struct transaction tr;
	read(nsd,&amount,sizeof(amount));
	int fd;
	if(account_no & 1)
		fd=open("normal_user_db.txt",O_RDWR);
	else
		fd=open("joint_account_user_db.txt",O_RDWR);
	lock(fd,F_WRLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true && amount<=u.amount){
			int fd1=open("transactions_db.txt",O_RDWR);
			lock(fd1,F_WRLCK);
			lseek(fd1,0,SEEK_END);
			tr.account_no=u.account_no;
			tr.amount=amount;
			tr.debited=true;
			tr.credited=false;
			time_t t=time(NULL);
			struct tm *tm=localtime(&t);
			tr.time.tm_sec=tm->tm_sec;
			tr.time.tm_min=tm->tm_min;
			tr.time.tm_hour=tm->tm_hour;
			tr.time.tm_year=tm->tm_year;
			tr.time.tm_mon=tm->tm_mon;
			tr.time.tm_mday=tm->tm_mday;
			write(fd1,&tr,sizeof(tr));
			unlock(fd1);
			close(fd1);
			u.amount=u.amount-amount;
			lseek(fd,(-1)*sizeof(u),SEEK_CUR);
			write(fd,&u,sizeof(u));
			break;	
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return ret;
	}
	ret=1;
	write(nsd,&ret,sizeof(ret));
	return ret;
}

int BalanceEnquiry(int nsd,int account_no){
	int fd,nbytes,ret;
	struct user u;
	if(account_no & 1)
		fd=open("normal_user_db.txt",O_RDONLY);
	else
		fd=open("joint_account_user_db.txt",O_RDONLY);
	lock(fd,F_RDLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			write(nsd,&u.amount,sizeof(u.amount));
			break;		
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	return 1;
}

int PasswordChange(int nsd,int account_no){
	int fd,nbytes,ret;
	char pswd[20];
	struct user u;
	if(account_no & 1)
		fd=open("normal_user_db.txt",O_RDWR);
	else
		fd=open("joint_account_user_db.txt",O_RDWR);
	lock(fd,F_WRLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			read(nsd,pswd,sizeof(pswd));
			if(!strcmp(pswd,u.pswd)){
				ret=1;
				write(nsd,&ret,sizeof(ret));
				read(nsd,u.pswd,sizeof(u.pswd));
				lseek(fd,(-1)*sizeof(u),SEEK_CUR);
				write(fd,&u,sizeof(u));
				break;		
			}
			else{
				ret=0;
				write(nsd,&ret,sizeof(ret));
				break;
			}
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	return ret;	
}

int ViewDetails(int nsd,int account_no){
	struct user u;
	struct transaction tr;
	int ret,fd,fd1,nbytes;
	if(account_no & 1)
		fd=open("normal_user_db.txt",O_RDONLY);
	else
		fd=open("joint_account_user_db.txt",O_RDONLY);
	lock(fd,F_RDLCK);
	while(nbytes=read(fd,&u,sizeof(u))){
		if(u.account_no==account_no && u.status==true){
			ret=1;
			write(nsd,&ret,sizeof(ret));
			write(nsd,&u,sizeof(u));
			fd1=open("transactions_db.txt",O_RDONLY);
			lock(fd1,F_RDLCK);
			struct transaction tr;
			while(read(fd1,&tr,sizeof(tr))){
				if(tr.account_no==account_no){
					write(nsd,&tr,sizeof(tr));	
				}
			}
			tr.account_no=0;
			write(nsd,&tr,sizeof(tr));	
			unlock(fd1);
			close(fd1);
			break;
		}
	}
	unlock(fd);
	close(fd);
	if(nbytes==0){
		ret=0;
		write(nsd,&ret,sizeof(ret));
		return 0;
	}
	else
		return 1;
}

int main(){
	int fd=open("normal_user_db.txt",O_RDWR|O_CREAT|O_EXCL,0764);
	close(fd);
	fd=open("joint_account_user_db.txt",O_RDWR|O_CREAT|O_EXCL,0764);
	close(fd);
	fd=open("transactions_db.txt",O_RDWR|O_CREAT|O_EXCL,0764);
	close(fd);
	fd=open("administrator_db.txt",O_RDWR|O_CREAT|O_EXCL,0764);
	if(fd!=-1){
		struct administrator administrator_struct;
		printf("Enter Your Administrator ID:\n");
		scanf("%s",administrator_struct.ad_id);
		printf("Enter The Password:\n");
		scanf("%s",administrator_struct.pswd);
		write(fd,&administrator_struct,sizeof(administrator_struct));
	}
	close(fd);
	
	struct sockaddr_in serv,cli;
	int sd=socket(AF_INET,SOCK_STREAM,0);
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=INADDR_ANY;
	int portno;
	printf("Enter port number:\n");
	scanf("%d",&portno);
	serv.sin_port=htons(portno);
	bind(sd,(struct sockaddr*)&serv,(socklen_t)sizeof(serv));
	listen(sd,5);
	int size=sizeof(cli);
	while(1){
		int nsd=accept(sd,(struct sockaddr *)&cli,(socklen_t*)&size);
		if(!fork()){
			close(sd);
			int i,ret;
			read(nsd,&i,sizeof(i));
			if(i==1){
				ret=administrator_login(nsd);
				if(ret==1){
					while(1){
						read(nsd,&i,sizeof(i));
						switch(i){
						case 1: ret=Add(nsd);
						        break;
						case 2: ret=Delete(nsd);
						        break;
						case 3: ret=Modify(nsd);
						        break;
						case 4: ret=Search(nsd);
						        break;
						default: break;
						
						}
					}
				}
			}
			else if(i==2){
				ret=login_user(nsd);
				if(ret!=0){
					int account_no=ret;
					while(1){
						read(nsd,&i,sizeof(i));
						switch(i){
						case 1: ret=Deposit(nsd,account_no);
						        break;
						case 2: ret=Withdraw(nsd,account_no);
						        break;
						case 3: ret=BalanceEnquiry(nsd,account_no);
						        break;
						case 4: ret=PasswordChange(nsd,account_no);
						        break;
						case 5: ret=ViewDetails(nsd,account_no);
						        break;
						default:
						        break;
						}					
					}
				}
			}
			exit(0);
		}
		else{
			close(nsd);
		}
	}
}
