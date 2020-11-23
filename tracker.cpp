#include<sys/types.h>
#include<sys/socket.h>
#include<bits/stdc++.h>
#include <netinet/in.h>  //for inet_addr()
#include <arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<cstdio>
#include<stdio.h>
#include<map>
#include<vector>
#include<stdlib.h>
#include<string.h>
using namespace std;
map<string,string> mp; // map for user validation and new registrations
map<int,int> groupadmin;// map used for keeping admin info first: gid
map<int,vector<int> > groupusers;// map used for tracking all users in a group
map<int,vector<string> > lof;    //list of files in a group
map<string,vector<string> > fileinfo; //file information fn:size,loc,sha
map<string,vector<int> > fileusers;// list of users having that file
map<string,long long> sizeoffile; //keeping file size info
class RequestInfo
{
  public:
  int gid;
  int RequestPort;  
};
void* rgstr(void* arg)
{
          // pthread_detach(pthread_self());
    
           int *t=(int*)arg;
           int connfd=*t;
           int n;
           char user[100],pass[100];
           if(n=recv(connfd, &user,sizeof(user),0)<0)
                perror("username recieving failed\n");
           string username(user);
           if(n=recv(connfd, &pass,sizeof(pass),0)<0)
                perror("passwd recieving failed");
           string passwd(pass);
           mp[username]=passwd;
           char ack='#';
           send(connfd,&ack,sizeof(ack),0);
           cout<<"registered\n";
    pthread_exit(0);
}
void* verify(void* arg)
{
           int *t=(int*)arg;
           int connfd=*t;
           int n;
           char user[100],pass[100];
           if(n=recv(connfd, &user,sizeof(user),0)<0)
                perror("username recieving failed\n");
           string username(user);
           if(n=recv(connfd, &pass,sizeof(pass),0)<0)
                perror("passwd recieving failed");
           string passwd(pass);
           char ack;
           if(mp.find(username)!=mp.end())
            {      if(mp[username]==passwd)
                      {
                        ack='#';
                        send(connfd,&ack,sizeof(ack),0);
                        cout<<"verified\n";
                       }
             }
             else
             {  
                  ack='$';
                  send(connfd,&ack,sizeof(ack),0);
                  cout<<"verified\n";
             }
    pthread_exit(0);
}
void* groupCreation(void* arg)
{
           int *t=(int*)arg;
           int connfd=*t;
           int n,gid,adminport;
           if(n=recv(connfd, &gid,sizeof(int),0)<0)
                perror("gid recieving failed\n");
           if(n=recv(connfd, &adminport,sizeof(int),0)<0)
                perror("adminport recieving failed");
           char ack='#';
           
           groupadmin[gid]=adminport;
          // cout<<"adminport "<<groupadmin[gid]<<"\n";
           groupusers[gid].push_back(adminport);
           send(connfd,&ack,sizeof(ack),0);
           cout<<"group-created\n";
            fflush(stdout);
           pthread_exit(0);
}
void* SendToAdmin(void* arg)
{     
      //struct RequestInfo* info1=(struct RequestInfo*)malloc(sizeof(struct RequestInfo));
      RequestInfo* info1=new RequestInfo();
      //info1=(struct RequestInfo*)arg;
     info1=(RequestInfo*)arg;
      //struct RequestInfo info=*info1;
      int AdminGroup=info1->gid;
      int requestport=info1->RequestPort;
      //cout<<"RequestPort "<<info1->RequestPort<<" \n";
     int adminport=groupadmin[AdminGroup];
     //cout<<"adminport "<<adminport<<"\n";
    int sp=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(adminport);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int n;
    int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("errorrrrrrr");
          exit(0);
        }
        else
          cout<<"connected with admin\n";
          char ackr='J';
           while(1)
           {
            if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
            else
            break;
           }
          if(send(sp,&requestport,sizeof(int),0)<0)
               perror("requestport sending failed\n");
         if(send(sp,&AdminGroup,sizeof(int),0)<0)
               perror("gid of admin sending failed\n");
          char ack;
          recv(sp,&ack,sizeof(ack),0);
         if(ack=='#')
         cout<<"request report to admin\n";
         else
         cout<<"not able to transfer request\n";
    close(sp);
    pthread_exit(0);
}
void* downloadfile(void* arg)
{
           int *t=(int*)arg;
           int connfd=*t;
           int n,gid,friendport;
           if(n=recv(connfd, &gid,sizeof(int),0)<0)
                perror("gid recieving failed\n");
           if(n=recv(connfd, &friendport,sizeof(int),0)<0)
                perror("adminport recieving failed");
          int flag=0;
        char ackr;
        map<int,vector<int> >:: iterator it=groupusers.find(gid);
        if(it!=groupusers.end())
        {
             vector<int> v(it->second);
             vector<int>::iterator i;
             for(i=v.begin();i<v.end();i++)
             {
                //cout<<*i<<" ";
                if(*i==friendport)
                {
                     flag=1;
                     break;
                }
             }
        }
        if(flag!=1)
        {
                ackr='$';
                if(send(connfd,&ackr,sizeof(ackr),0)<0)
                perror("ack not sent"); 
        }
        else
        {
                ackr='#';
                if(send(connfd,&ackr,sizeof(ackr),0)<0)
                        perror("ack not sent");
                        
                char fname[100];
                if(n=recv(connfd,&fname,sizeof(fname),0)<0)
                   perror("gid recieving failed\n");
                   
                map<string,vector<string> >:: iterator itr=fileinfo.find(fname);
                
                if(itr!=fileinfo.end())
                        {  
                           vector<string> v(itr->second);
                           vector<string>::iterator i;
                           //string filesize=*i;
                           long long fsize=sizeoffile[fname];
                           //char fsize[100];
                           //strcpy(fsize,filesize.c_str());
                           
                            if(send(connfd,&fsize,sizeof(fsize),0)<0)
                                 perror("ack not sent"); 
                            cout<<"success6\n";
                        }
                map<string,vector<int> >::iterator itr1=fileusers.find(fname);
                if(itr1!=fileusers.end())
                        {
                           int lop[10],j=0;
                           vector<int> v(itr1->second);
                           vector<int>::iterator i;
                           for(i=v.begin();i<v.end();i++)
                               {
                                   lop[j++]=*i;
                                   cout<<"in download section "<<*i<<" ";
                                   
                               }
                              /* for(int k=0;k<4;k++)
                               {
                                    cout<<lop[k]<<" ";
                               }
                               cout<<"\n";*/
                           if(send(connfd,lop,sizeof(lop),0)<0)
                                 perror("ack not sent"); 
                          }
        }
      fflush(stdout);
      pthread_exit(0);
}
int main(int argc,char* argv[])
{
     int port=atoi(argv[1]);
     pthread_t tid_r;
     pthread_t tid_s;
     pthread_attr_t attr;
     pthread_attr_init(&attr);
     int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //serv_addr.sin_addr.s_addr = htonl("127.0.0.1");
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(port); 

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0)
        perror("binding error\n"); 

    if(listen(listenfd, 10)<0)
        perror("listening error"); 
    else
    cout<<"listening...\n";
   
     while(1){
     connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
      //struct RequestInfo* info=(struct RequestInfo*)malloc(sizeof(struct RequestInfo));
      RequestInfo info;//=new RequestInfo();
     char ack;
     recv(connfd,&ack,sizeof(ack),0);
     if(ack=='R')       //for registration
     {
        pthread_create(&tid_r,&attr,rgstr,&connfd);
        pthread_join(tid_r,NULL);
     }
     else if(ack=='V')     //to verify
     {
        pthread_create(&tid_r,&attr,verify,&connfd);
        pthread_join(tid_r,NULL);
     }
     else if(ack=='G')      //for group creation
     {
        pthread_create(&tid_r,&attr,groupCreation,&connfd);
        pthread_join(tid_r,NULL);
     }
     else if(ack=='J')        // join request from client
     {  
        //cout<<"in J\n";
        ack='#';
        int gid_i,requestport;
        recv(connfd,&gid_i,sizeof(int),0);
        recv(connfd,&requestport,sizeof(int),0);
        send(connfd,&ack,sizeof(ack),0);
        info.gid=gid_i;
        info.RequestPort=requestport;
        if(pthread_create(&tid_r,&attr,SendToAdmin,&info)<0)
           perror("sendToAdmin thread failed\n");
        pthread_join(tid_r,NULL);
     }
     else if(ack=='A')          //accept-request after work for the tracker
     {
        int gid,friendport;
        recv(connfd,&gid,sizeof(int),0);
        recv(connfd,&friendport,sizeof(int),0);
        //cout<<gid<<" "<<friendport;
        groupusers[gid].push_back(friendport);
        char ackr='#';
        if(send(connfd,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
            fflush(stdout);
     } 
     else if(ack=='L')             // list groups
     {
              map<int,int>::iterator it ;
              int gid;
              int size=groupadmin.size();
              if(send(connfd,&size,sizeof(int),0)<0)
                      perror("size not sent");
             for(it =groupadmin.begin();it!=groupadmin.end();++it)
             {
                 gid=it->first;
                 if(send(connfd,&gid,sizeof(int),0)<0)
                      perror("gid not sent");
             }
     } 
     else if(ack=='U')          //accept-request after work for the tracker
     {
        int gid,friendport;
        recv(connfd,&gid,sizeof(int),0);
        recv(connfd,&friendport,sizeof(int),0);
        //cout<<gid<<" "<<friendport;
        //groupusers[gid].push_back(friendport);
        int flag=0;
        char ackr;
        map<int,vector<int> >:: iterator it=groupusers.find(gid);
        if(it!=groupusers.end())
        {
             vector<int> v(it->second);
             vector<int>::iterator i;
             for(i=v.begin();i<v.end();i++)
             {
                //cout<<*i<<" ";
                if(*i==friendport)
                {
                     flag=1;
                     break;
                }
             }
        }
        if(flag!=1)
        {
                ackr='$';
                if(send(connfd,&ackr,sizeof(ackr),0)<0)
                perror("ack not sent"); 
        }
        else
        {
                ackr='#';
                if(send(connfd,&ackr,sizeof(ackr),0)<0)
                    perror("ack not sent"); 
                 //cout<<"\n";
                long long n;
                char fname[100],floc[100],sharray[100];
                if(n=recv(connfd, &fname,sizeof(fname),0)<0)
                perror("fname recieving failed\n");
                string filename(fname);
                if(n=recv(connfd, &floc,sizeof(floc),0)<0)
                perror("floc recieving failed");
                string fileLocation(floc);
                long long filesize;
                if(n=recv(connfd, &filesize,sizeof(filesize),0)<0)
                        perror("filesize recieving failed");
                 if(n=recv(connfd, &sharray,sizeof(sharray),0)<0)
                perror("fname recieving failed\n");
                string finalsha(sharray);      
               // cout<<"fname: "<<filename<<"floc :"<<fileLocation<<"filesize: "<<filesize<<"\n";
                char filesz[100];
                snprintf(filesz, sizeof(filesz), "%lld", filesize); 
                string filesizestr(filesz);
                fileusers[filename].push_back(friendport);//updated fileusers
                sizeoffile[filename]=filesize;        //fileinfo added in the map
                fileinfo[filename].push_back(fileLocation);  //fileinfo updated
                fileinfo[filename].push_back(finalsha);
                lof[gid].push_back(filename);//list of files in a group
                ackr='#';
                if(send(connfd,&ackr,sizeof(ackr),0)<0)
                        perror("ack not sent");
                fflush(stdout);
        } 
     }
     else if(ack=='D')      //To download files
     {
        pthread_create(&tid_r,&attr,downloadfile,&connfd);
        pthread_join(tid_r,NULL);
     }   
      else if(ack=='F')      //To download files
     {
        int gid,n;
        if(n=recv(connfd, &gid,sizeof(int),0)<0)
                perror("gid recieving failed\n");
        map<int,vector<string> >::iterator it=lof.find(gid);
        if(it!=lof.end())
                        {
                          
                           vector<string> v(it->second);
                           vector<string>::iterator i;
                           int fileno=v.size();
                           if(send(connfd,&fileno,sizeof(fileno),0)<0)
                                 perror("ack not sent"); 
                           for(i=v.begin();i<v.end();i++)
                               {
                                   string temp=*i;
                                   char temp_c[100];
                                   strcpy(temp_c,temp.c_str());
                                   if(send(connfd,&temp_c,sizeof(temp_c),0)<0)
                                     perror("ack not sent"); 
                                   
                               }
                            
                           
                          }
     }   
     else      
     cout<<"not coded\n";
     close(connfd);
     }
     close(listenfd);
}
