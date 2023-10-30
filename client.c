#include<stdbool.h>
#include <arpa/inet.h>
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

int login_admin(int sd){
	printf("---------------------------Welcome to Admin Management portal-------------------------------\n");
	printf("Enter Admin ID:\n");
	char ad_id[20];
	scanf("%s",ad_id);
	printf("Enter password:\n");
	char pswd[20];
	scanf("%s",pswd);
	write(sd,ad_id,sizeof(ad_id));
	write(sd,pswd,sizeof(pswd));
	int ret;
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("login failed\n");
	}
	else{
		printf("login success\n");
	}
	return ret;
}

int Add(int sd){
	printf("Enter account type:\n1:Normal user\n2:Joint account\n");
	int type;
	int ret;
	scanf("%d",&type);
	getchar();
	write(sd,&type,sizeof(type));
	if(type!=1 && type!=2){
		read(sd,&ret,sizeof(ret));
		printf("Adding failed\n");
		return ret;
	}
	printf("Enter account holder name:\n");
	char name[50];
	fgets(name,50,stdin);
	name[strlen(name)-1]='\0';
	write(sd,name,sizeof(name));
	if(type==2){
		printf("Enter account holder 2 name:\n");
		char name2[50];
		fgets(name2,50,stdin);
		name2[strlen(name2)-1]='\0';
		write(sd,name2,sizeof(name2));
	}
	printf("Enter amount:\n");
	long amount;
	scanf("%ld",&amount);
	getchar();
	write(sd,&amount,sizeof(amount));
	printf("Enter password:\n");
	char pswd[20];
	fgets(pswd,20,stdin);
	pswd[strlen(pswd)-1]='\0';
	write(sd,pswd,sizeof(pswd));
	read(sd,&ret,sizeof(ret));
	printf("Account added successfully\n");
	printf("Account number:%d\n",ret);
	return ret;
} 

int Search(int sd){
	struct user u;
	struct transaction tr;
	int ret;
	printf("Enter account number:\n");
	int account_no;
	scanf("%d",&account_no);
	write(sd,&account_no,sizeof(account_no));
	read(sd,&ret,sizeof(ret));
	if(ret==1){
		read(sd,&u,sizeof(u));
		printf("Name:%s\n",u.name);
		if(!(account_no & 1)){
			printf("Name2:%s\n",u.name2);
		}
		printf("Amount:%ld\n",u.amount);
		printf("Account number:%d\n",u.account_no);
		printf("Transactions Details:\n");
		while(1){
			read(sd,&tr,sizeof(tr));
			if(tr.account_no==0) 
				break;	
			if(tr.debited==true)
				printf("Debited\n");
			if(tr.credited==true)
				printf("Credited\n");
			printf("Amount:%d\n",tr.amount);
			printf("Time:%d:%d:%d\n",tr.time.tm_hour,tr.time.tm_min,tr.time.tm_sec);
			printf("Date:%d:%d:%d\n",tr.time.tm_year+1900,tr.time.tm_mon+1,tr.time.tm_mday);
		}
		return ret;
	}
	printf("Search failed\n");
	return ret;
}

int Modify(int sd){
	struct user u;
	int ret,i;
	printf("Enter account number\n");
	int account_no;
	scanf("%d",&account_no);
	write(sd,&account_no,sizeof(account_no));
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Search failed\n");
		return ret;
	}
	read(sd,&u,sizeof(u));
	printf("Name:%s\n",u.name);
	if(!(account_no & 1)){
		printf("Name2:%s\n",u.name2);
	}
	printf("Amount:%ld\n",u.amount);
	printf("Account number:%d\n",u.account_no);
	printf("Change username\n1:Yes\n2:No\n");
	scanf("%d",&i);
	if(i==1){
		printf("Enter Name:\n");
		getchar();
		fgets(u.name,50,stdin);
		u.name[strlen(u.name)-1]='\0';
	}
	else{
		u.name[0]='\0';
	}
	if(!(account_no & 1)){
		printf("Change username2\n1:Yes\n2:No\n");
		scanf("%d",&i);
		if(i==1){
			printf("Enter Name:\n");
			getchar();
			fgets(u.name2,50,stdin);
			u.name2[strlen(u.name2)-1]='\0';
		}
		else{
			u.name2[0]='\0';
		}
	}
	else{
		u.name2[0]='\0';
	}
	write(sd,&u,sizeof(u));
	read(sd,&ret,sizeof(ret));
	printf("Modified successfully\n");
	return ret;
}

int Delete(int sd){
	struct user u;
	int ret,i;
	printf("Enter account number\n");
	int account_no;
	scanf("%d",&account_no);
	write(sd,&account_no,sizeof(account_no));
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Search failed\n");
		return ret;
	}
	read(sd,&u,sizeof(u));
	printf("Name:%s\n",u.name);
	if(!(account_no & 1)){
		printf("Name2:%s\n",u.name2);
	}
	printf("Amount:%ld\n",u.amount);
	printf("Account number:%d\n",u.account_no);
	read(sd,&ret,sizeof(ret));
	if(ret==1){
		printf("Deleted Successfully\n");
	}
	return ret;
}

int login_user(int sd,int type){
	printf("---------------------Welcome to user management portal------------------------\n");
	printf("Enter account number:\n");
	int account_no;
	scanf("%d",&account_no);
	printf("Enter password:\n");
	char pswd[20];
	scanf("%s",pswd);
	write(sd,&type,sizeof(type));
	write(sd,&account_no,sizeof(account_no));
	write(sd,pswd,sizeof(pswd));
	int ret;
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("login failed\n");
		return 0;
	}
	else{
		printf("login success\n");
		return account_no;
	}
}

int Deposit(int sd,int account_no){
	int amount,ret;
	printf("Enter amount:\n");
	scanf("%d",&amount);
	write(sd,&amount,sizeof(amount));
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Operation failed\n");
		return 0;
	}
	else{
		printf("Operation success\n");
		return 1;
	}
}

int Withdraw(int sd,int account_no){
	int amount,ret;
	printf("Enter amount:\n");
	scanf("%d",&amount);
	write(sd,&amount,sizeof(amount));
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Operation failed\n");
		return 0;
	}
	else{
		printf("Operation success\n");
		return 1;
	}
}

int BalanceEnquiry(int sd,int account_no){
	int ret;
	long amount;
	read(sd,&ret,sizeof(ret));
	if(ret==1){
		read(sd,&amount,sizeof(amount));
		printf("Balance:%ld\n",amount);
		return 1;
	}
	else{
		printf("Operation failed\n");
		return 0;
	}
}

int PasswordChange(int sd,int account_no){
	int ret;
	char pswd[20];
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Search failed\n");
		return 0;
	}
	printf("Enter old password:\n");
	getchar();
	fgets(pswd,sizeof(pswd),stdin);
	pswd[strlen(pswd)-1]='\0';
	write(sd,pswd,sizeof(pswd));
	read(sd,&ret,sizeof(ret));
	if(ret==0){
		printf("Password does not match\n");
		return 0;
	}
	printf("Enter new password:\n");
	fgets(pswd,sizeof(pswd),stdin);
	pswd[strlen(pswd)-1]='\0';
	write(sd,pswd,sizeof(pswd));
	printf("Operation Success\n");
	return 1;	
}

int ViewDetails(int sd,int account_no){
	struct user u;
	struct transaction tr;
	int ret;
	read(sd,&ret,sizeof(ret));
	if(ret==1){
		read(sd,&u,sizeof(u));
		printf("Name1:%s\n",u.name);
		if(!(account_no & 1)){
			printf("Name2:%s\n",u.name2);
		}
		printf("Amount:%ld\n",u.amount);
		printf("Account number:%d\n",u.account_no);
		printf("Transactions Details:\n");
		while(1){
			read(sd,&tr,sizeof(tr));
			if(tr.account_no==0) 
				break;	
			if(tr.debited==true)
				printf("Debited\n");
			if(tr.credited==true)
				printf("Credited\n");
			printf("Amount:%d\n",tr.amount);
			printf("Time:%d:%d:%d\n",tr.time.tm_hour,tr.time.tm_min,tr.time.tm_sec);
			printf("Date:%d:%d:%d\n",tr.time.tm_year+1900,tr.time.tm_mon+1,tr.time.tm_mday);
		}
		return ret;
	}
	printf("Search failed\n");
	return ret;
}

int main(){
	struct sockaddr_in serv;
	int sd=socket(AF_INET,SOCK_STREAM,0);
	serv.sin_family=AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
	int portno=8080;
	serv.sin_port=htons(portno);
	int nsd=connect(sd,(struct sockaddr *)&serv,sizeof(serv));
	if(nsd<0){
		perror("Error");
	}
	printf("Enter login option:\n1:Administrator\n2:Customer Account\n");
	int i,ret;
	scanf("%d",&i);
	write(sd,&i,sizeof(i));
	if(i==1){
		ret=login_admin(sd);
		if(ret==1){
			while(1){
				printf("Enter operation you want to perform:\n1:Add\n2:Delete\n3:Modify\n4:Search\n5:Exit\n");
				scanf("%d",&i);
				write(sd,&i,sizeof(i));
				switch(i){
				        case 1: ret=Add(sd);
				                break;
				        case 2: ret=Delete(sd);
				                break;
				        case 3: ret=Modify(sd);
				                break;
				        case 4: ret=Search(sd);
				                break;
				        default: break;
				}
				if((i!=1)&&(i!=2)&&(i!=3)&&(i!=4)) break;
			}
		}
	}
	else if(i==2){
        int type;
        printf("Enter account type:\n1:Normal user\n2:Joint account\n");	
	scanf("%d",&type);
		ret=login_user(sd,type);
		if(ret!=0){
			int account_no=ret;
			while(1){
				printf("Enter operation you want to perform:\n1:Deposit\n2:Withdraw\n3:Balance Enquiry\n4:Password Change\n5:View Details\n6:Exit\n");
				scanf("%d",&i);
				write(sd,&i,sizeof(i));
				switch (i){
				case 1: ret=Deposit(sd,account_no);
					break;
				case 2: ret=Withdraw(sd,account_no);
					break;
				case 3: ret=BalanceEnquiry(sd,account_no);
					break;
				case 4: ret=PasswordChange(sd,account_no);
					break;
				case 5: ret=ViewDetails(sd,account_no);
				        break;
				default: break;
				}
				if((i!=1)&&(i!=2)&&(i!=3)&&(i!=4)&&(i!=5)) break;
			}
		}			
	}
	
	close(nsd);
}
