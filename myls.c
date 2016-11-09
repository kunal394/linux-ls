//#define _GNU_SOURCE
#include <dirent.h> /* Defines DT_* constants */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include<errno.h>
#define BUF_SIZE 2*1024*1024

typedef struct linux_dirent 
{
	unsigned long d_ino;
	unsigned long d_off;
	unsigned short d_reclen;
	char d_name[];
}ld;

typedef struct file_stats
{
	char permissions[10],name[50],user[50],group[50];
	int count,size,date;
}file_stats;


int mystrlen(char arr[])
{
	int i=0;
	while(arr[i])
		i++;
	return i;	
}

void mystrcpy(char arr1[],char arr2[])
{
	int i=0;
	while(arr2[i])
	{
		arr1[i]=arr2[i];
		i++;
	}
	arr1[i]='\0';	
}

void mystrcat(char *s1,char *s2)
{
	while(*s1)
		s1++;
	while(*s2)
	{
		*s1=*s2;
		s1++;
		s2++;
	}
	s1='\0';
}

void printfloat(long long int num,int count)
{
	char arr[100],temp[2];
	int size=0,j;
	char sizes[7]={'B','K','M','G','T','P','\0'};
	temp[1]='\0';
	while(num)
	{
		arr[size]=num%10+'0';
		num=num/10;
		size++;
	}
	for(j=size-1;j>=0;j--)
	{
		if(j==1)
		{
			if(arr[1]=='0' && arr[0]=='0') break;
			else write(1,".",1);
		}
		temp[0]=arr[j];
		write(1,temp,2);
	}
	arr[size]='\0';
	temp[0]=sizes[count];
	write(1,temp,2);
	write(1,"      ",6);
}
void hrsize(long long int size)
{
	int i=0;
	long double s;
	s=size;
	while(s>=1000)
	{
		i++;
		s/=1024;
	}
	s=(int)(s*100);
	long long int  s_rounded=s;
	printfloat(s_rounded,i);
}

void printinteger(long long int num,int spaces)
{
	char arr[100],temp[2];
	int i,size=0,j;
	temp[1]='\0';
	while(num)
	{
		arr[size]=num%10+'0';
		num=num/10;
		size++;
	}
	for(j=size-1;j>=0;j--)
	{
		temp[0]=arr[j];
		write(1,temp,2);
	}
	arr[size]='\0';
	for(i=0;i<spaces;i++)
		write(1,"  ",1);
}

int compuid(char arr[],int uid,int size)
{
        int i,suid=0,cuid;
        cuid=uid;
	if(uid==0) suid=1;
        else
	{
		while(cuid)
		{
			cuid=cuid/10;
			suid++;
		}
	}
        if(suid!=size+1)
                return 0;
        for(i=size;i>=0;i--)
        {
                if((arr[i]-'0')!=uid%10)
                        return 0;
                uid=uid/10;    
        }
        return 1;
}

file_stats idtoname(file_stats f,int id,int type)
{
	int fp,rd,name_pos=0,uid=id,cur_uid,uid_count=0;
        char buf[BUF_SIZE],name[40],uidarr[100];
	if(type==1)//user
        	fp=open("/etc/passwd",O_RDONLY);
	else//group
        	fp=open("/etc/group",O_RDONLY);
        if(fp<1)
                write(1,"cant open\n",11);
	rd=read(fp,buf,BUF_SIZE);
	int i=0;
	while(buf[i])
	{
		if(buf[i]==':')
		{
			name[name_pos]='\0';
			i++;
			while(buf[i]!=':')
			{
	//			printf("buf[i]:%c\n",buf[i]);
				i++;
			}
			i++;
			while(buf[i]!=':')
			{
	//			printf("building uidarr buf[i]:%c\n",buf[i]);
				uidarr[uid_count]=buf[i];
				uid_count++;
				i++;
			}
			uidarr[uid_count]='\0';
	//		printf("uidarr being sent to com:%s uid:%d\n",uidarr,uid);
			cur_uid=compuid(uidarr,uid,uid_count-1);
			if(cur_uid==1)
			{
				if(type==1)
				{
	//				printf("user comp:%s\n",name);
					write(1,name,mystrlen(name));
					mystrcpy(f.user,name);
				}
				else if(type==2)
				{
					write(1,name,mystrlen(name));
	//				printf("group comp:%s\n",name);
					mystrcpy(f.group,name);
				}
				return f;
			}
			else
			{
				uid_count=name_pos=0;
				while(buf[i])
				{
					if(buf[i]=='\n') break;
					i++;
				}
			}
		}
		else
		{
			name[name_pos]=buf[i];
			name_pos++;
		}
		i++;
	}
	return f;
}


void colorprint(char fname[],int type)
{
	char red_color[]="\e[1;31m",blue_color[]="\e[1;34m",green_color[]="\e[1;32m",color_end[]="\e[0m";
	char color[200]="";
	if(type==1)
		mystrcpy(color,blue_color);
	else if(type==2)	
		mystrcpy(color,green_color);
	else if(type==3)	
		mystrcpy(color,red_color);
	mystrcat(color,fname);
	mystrcat(color,color_end);
	write(1,color,mystrlen(color));//printf("%s  ",f1.name);
}

int check_leap_year(int year)
{
        if(year%4==0 && year%100!=0)
                return 1;
        else if(year%4==0 && year%100==0)
        {
                if(year%400==0)
                        return 1;
        }
        return 0;
}

void print_time(long long int t)
{
        long long int days;
        int sec_today,i,cur_year,lycount=0,sec_cur_year,days_cur_year;
        cur_year=t/31556926+1970;//31556926 is achieved by assuming 1 year=365.24 days in epoch format
        for(i=1970;i<cur_year;i++)
                lycount+=check_leap_year(i);
        days=365*(cur_year-1970)+lycount;
        sec_cur_year=t-(days*24*60*60);
        days_cur_year=(sec_cur_year)/(60*60*24);
        if(check_leap_year(cur_year) && days_cur_year>59)
                days_cur_year+=1;
        int months_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
        if(check_leap_year(cur_year))
                months_days[1]=29;
        int d=0,month_count,exact_date;
        for(i=0;i<12;i++)
        {
                d+=months_days[i];
                if(!(days_cur_year/d))
                {
                        month_count=i;
                        exact_date=days_cur_year-d+months_days[i]+1;
                        break;
                }
        }
        char months[12][5]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
        int cur_hour,cur_min,cur_sec;
        sec_today=sec_cur_year-days_cur_year*24*60*60;
//      printf("cur_year_days:%d curmonth:%s curday:%d sec_today:%d\n",days_cur_year,months[month_count],exact_date,sec_today);
        cur_hour=sec_today/(3600);
        sec_today-=cur_hour*3600;
        cur_min=sec_today/60;
        cur_sec=sec_today-cur_min*60;
//      printf("cur_sec:%d cur_min:%d cur_hour:%d\n",cur_sec,cur_min,cur_hour);
	printinteger(cur_year,1);
	write(1,months[month_count],mystrlen(months[month_count]));
	write(1," ",1);
	printinteger(exact_date,1);
	if(cur_hour==0)
		write(1,"00",2);
	else	
		printinteger(cur_hour,0);
	write(1,":",1);
	if(cur_min==0)
		write(1,"00  ",4);
	else	
		printinteger(cur_min,1);
	write(1,"(GMT)  ",7);	
//	printf("%s  %d  %d:%d\n",months[month_count],exact_date,cur_hour,cur_min);
}

void printl(char abspath[],struct stat fileStat,int h,char name[],int link)
{
//	printf("link:%d\n",link);
	file_stats f1;
//	printf("sending uid:%d for file:%s\n",fileStat.st_uid,name);
	mystrcpy(f1.name,name);
	f1.size=fileStat.st_size;
	f1.count=fileStat.st_nlink;
	if(link==1)
		write(1,"lrwxrwxrwx  ",12);
	else
	{
		if(S_ISDIR(fileStat.st_mode))
		{
			write(1,"d",1);
			//	write(1,(S_ISDIR(fileStat.st_mode) ? "d" : "-"),1);
			//	write(1,(S_ISLNK(fileStat.st_mode) ? "d" : "-"),1);
		}
		else
			write(1,"-",1);
		write(1,((fileStat.st_mode & S_IRUSR) ? "r" : "-"),1);
		write(1,((fileStat.st_mode & S_IWUSR) ? "w" : "-"),1);
		write(1,((fileStat.st_mode & S_IXUSR) ? "x" : "-"),1);
		write(1,((fileStat.st_mode & S_IRGRP) ? "r" : "-"),1);
		write(1,((fileStat.st_mode & S_IWGRP) ? "w" : "-"),1);
		write(1,((fileStat.st_mode & S_IXGRP) ? "x" : "-"),1);
		write(1,((fileStat.st_mode & S_IROTH) ? "r" : "-"),1);
		write(1,((fileStat.st_mode & S_IWOTH) ? "w" : "-"),1);
		write(1,((fileStat.st_mode & S_IXOTH) ? "x  " : "-  "),3);
	}
	printinteger(f1.count,2);//printf("%d  ",f1.count);
	//	write(1,f1.user,mystrlen(f1.user));//printf("%s  ",f1.user);
	f1=idtoname(f1,fileStat.st_uid,1);
	write(1,"  ",2);
	//	write(1,f1.group,mystrlen(f1.group));//printf("%s  ",f1.group);
	f1=idtoname(f1,fileStat.st_gid,2);
	write(1,"  ",2);
	if(f1.size==0)
		write(1,"0B  ",4);
	else
	{
		if(h==1)
			hrsize(f1.size);
		else
			printinteger(f1.size,2);//printf("%d  ",f1.size);
	}
	print_time(fileStat.st_mtime);	
	write(1,"   ",3);
	if(link==1)
	{
		char link_name[100]="";
		int r=readlink(abspath,link_name,100);
		colorprint(f1.name,3);
		write(1,"-->",3);
		write(1,link_name,mystrlen(link_name));
//		printf("abspath:%s link_name:%s\n",abspath,link_name);
	}
	else if(S_ISDIR(fileStat.st_mode))
		colorprint(f1.name,1);
	else if(fileStat.st_mode & S_IXUSR)
		colorprint(f1.name,2);
	else
		write(1,name,mystrlen(name));
	write(1,"\n",1);	
}


int main(int argc, char *argv[])
{
	int i,nf=-2,bpos,l=0,a=0,h=0,j,mark;
	char d_type;
	struct stat fileStat;
	char buf[BUF_SIZE];
	ld *d;
	for(i=1;i<argc;i++)
	{
		if(argv[i][0]=='-')
		{
			for(j=1;j<mystrlen(argv[i]);j++)
			{
				if(argv[i][j]=='l') l=1;
				else if(argv[i][j]=='a') a=1;
				else if(argv[i][j]=='h') h=1;
			}
		}
		else
		{
			mark=i;
			nf = open(argv[i],O_RDONLY | O_DIRECTORY);
		}
	}
	if(nf==-2)
	{
		mark=4;
		argv[mark]=".";
		nf = open(".", O_RDONLY | O_DIRECTORY);
	}
	if (nf == -1)
	{
		if(errno==ENOTDIR)
		{
			int check;
			check=lstat(argv[mark],&fileStat);
			if(check==-1)
			{
				printf("can't read %s\n",argv[mark]);
				return 0;
			}
			if(l==1)
				printl(argv[mark],fileStat,h,argv[mark],S_ISLNK(fileStat.st_mode));
			else
			{
				if(S_ISLNK(fileStat.st_mode))
					colorprint(argv[mark],3);
				else if(fileStat.st_mode & S_IXUSR)
					colorprint(argv[mark],2);
				else
					write(1,argv[mark],mystrlen(argv[mark]));
				write(1,"\n",1);	
			}
		}
		else
			perror(argv[mark]);
		return 0;
	}
	while(1)
	{
		bpos=0;
		//	printf("nf:%d bpos:%d\n",nf,bpos);
		nf = syscall(SYS_getdents, nf, buf, BUF_SIZE);//syscall is used for invoking a system call which has no wrapper function
		if (nf == -1)
		{
//			perror("Error");
			return 0;
		}
		if (nf == 0)
			break;
		if(l==1)
		{
			while(bpos < nf)
			{
				d = (ld *) (buf + bpos);
				d_type = *(buf + bpos + d->d_reclen - 1);
				int check;
				char curfile[200]="";
				if(argv[mark]!=".")
				{	
		//			printf("\ncurfile:%s\n",curfile);
					mystrcpy(curfile,argv[mark]);
					if(curfile[mystrlen(curfile)-1]!='/')
						mystrcat(curfile,"/");
					mystrcat(curfile,d->d_name);
				//	printf("\nargv[mark]:%s curfile:%s,d->d_name:%s\n",argv[mark],curfile,d->d_name);
				}
				else 
					mystrcpy(curfile,d->d_name);
				check=stat(curfile,&fileStat);
				if(check==-1)
				{
					printf("can't read %s\n",curfile);
					bpos += d->d_reclen;
					continue;
				}
				if(d->d_name[0]=='.'&&a==0)
				{
					bpos += d->d_reclen;
					continue;
				}
				printl(curfile,fileStat,h,d->d_name,d_type==DT_LNK);
				bpos += d->d_reclen;
			}
		}
		else
		{
			while(bpos < nf)
			{
				d = (ld *) (buf + bpos);
				d_type = *(buf + bpos + d->d_reclen - 1);
				if(a==0&&d->d_name[0]=='.')
				{
					bpos += d->d_reclen;
					continue;
				}
				char curfile[200]="";
				if(argv[mark]!=".")
				{	
					mystrcpy(curfile,argv[mark]);
					if(curfile[mystrlen(curfile)-1]!='/')
						mystrcat(curfile,"/");
					mystrcat(curfile,d->d_name);
				}
				else 
					mystrcpy(curfile,d->d_name);
				stat(curfile,&fileStat);
				if(d_type==DT_LNK)
				{
					colorprint(d->d_name,3);
					char link_name[100]="";
					int r=readlink(curfile,link_name,100);
					colorprint(d->d_name,3);
					write(1,"-->",3);
					write(1,link_name,mystrlen(link_name));
				}
				else if(S_ISDIR(fileStat.st_mode))
					colorprint(d->d_name,1);
				else if(fileStat.st_mode & S_IXUSR)
					colorprint(d->d_name,2);
				else
					write(1,d->d_name,mystrlen(d->d_name));//printf("%s ",d->d_name);	
				write(1," \n",2);
				bpos += d->d_reclen;
			}
		}
	}
	return 0;
}
