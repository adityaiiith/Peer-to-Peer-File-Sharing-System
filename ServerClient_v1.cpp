#include<sys/types.h>
#include<sys/socket.h>
#include<bits/stdc++.h>
#include <netinet/in.h>  //for inet_addr()
#include <arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<cstdio>
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<map>
#include<vector>
#include<openssl/sha.h>
#define BufferSize 524288
#define QUE 5
using namespace std;
map<int,int> requestlist; // first:requestport, second: gid to which request arrived
struct chunk_info
{
     int chunk_no;
     char fname[50];
};
struct part_info
{   
    FILE *file_p;
    struct chunk_info chunkinfo;
    int portno;
};
int serverport;
int trackerportno;
void* recieve(void* arg)
{
     
     struct part_info* t1=(struct part_info*)malloc(sizeof(struct part_info));
     t1=(struct part_info*)arg;
     struct part_info t=*t1;
     int port=t.portno;
     int chunkno=t.chunkinfo.chunk_no;
     FILE* file_p_o=t.file_p;
     char filename[50];
     strcpy(filename,t.chunkinfo.fname);
    int sp=socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("error in connecting");
          exit(0);
        }
        else
          cout<<"connected\n";
        char ackr='D';
        while(1)
        {
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
          else
            break;
        }
          struct chunk_info* Toserver=(struct chunk_info*)malloc(sizeof(struct chunk_info));
          Toserver->chunk_no=chunkno;
          strcpy(Toserver->fname,filename);
       if(send(sp,Toserver,sizeof(struct chunk_info),0)<0)
           perror("chunkinfo not send\n");
          int offset=chunkno*BufferSize;
           fseek(file_p_o,offset,SEEK_SET);
          char Buffer[BufferSize] ; 
          int n;
           n=recv(sp,Buffer,BufferSize,0);
                  fwrite(Buffer,sizeof(char),n,file_p_o);
	          memset(Buffer,'\0',BufferSize);
         cout<<"chunkno recieved: "<<chunkno<<"\n";
        fclose(file_p_o); 
        close(sp);
        pthread_exit(0);
}
void* Download_file(void* arg)
{
     int* trackerport=(int*)malloc(sizeof(int));
     trackerport=(int*)arg;
     int trackerPortRetrieve=*trackerport;
     int sp=socket(AF_INET,SOCK_STREAM,0);
     struct sockaddr_in addr;
     memset(&addr,'\0',sizeof(addr));
     addr.sin_family=AF_INET;
     addr.sin_port=htons(trackerPortRetrieve);
     addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int n;
     int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("errorrrrrrr");
          exit(0);
        }
        else
          cout<<"connected with tracker\n";
            char ackr='D';
            if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
            int gid;
            cout<<"enter group_id: \n";
            cin>>gid;
            int n_gid=gid;
            int n_serverport=serverport;
            if(send(sp,&n_gid,sizeof(int),0)<0)
                perror("gid not sent");
            if(send(sp,&n_serverport,sizeof(int),0)<0)
                perror("friendport not sent");
            char ack;
            recv(sp,&ack,sizeof(ack),0);
            if(ack=='$')
            {
                cout<<"first join respective group\n";
            }
            else
            {
                 cout<<"you are part of group\n";  
                string filename;
                cout<<"enter filename to download\n"; 
                cin>>filename;
                char fname[100];
                strcpy(fname,filename.c_str());
                if(send(sp,&fname,sizeof(fname),0)<0)
                    perror("fname not sent");
                //char file_sz[100];
                long long file_size;
                long long n;
                if(n=recv(sp,&file_size,sizeof(file_size),0)<0)
                    perror("username recieving failed\n");
                
               // string file_sz_c(file_sz);
               // stringstream geek(file_sz_c);
               // geek >> file_size;
                struct part_info* chunk=(struct part_info*)malloc(sizeof(struct part_info)*4);
                pthread_t tids[4];
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                int portlist[10];//={1234,1235,1236,1237};
                if(n=recv(sp,portlist,sizeof(portlist),0)<0)
                    perror("username recieving failed\n");
                FILE *fp = fopen (fname,"wb");
                char Buffer[BufferSize]; 
                memset(Buffer,'a',BufferSize);
          
                int n1=50;
                while(file_size > 0)
                {      
                     fwrite(Buffer,sizeof(char),n1,fp);
	             memset(Buffer,'a',BufferSize);
                     file_size =file_size-n1;
                } 
                fclose(fp);
                for(int i=0;i<4;i++)
                {  
                        FILE *fp1 = fopen (filename.c_str(),"wb");
                        chunk[i].file_p=fp1;
                        chunk[i].portno=portlist[i];
                        chunk[i].chunkinfo.chunk_no=i;
                        strcpy(chunk[i].chunkinfo.fname,filename.c_str());
                        if(pthread_create(&tids[i],&attr,recieve,&chunk[i])<0)
                        perror("creation error");
                }
                for(int i=0;i<4;i++)
                {
                        pthread_join(tids[i],NULL);
                        cout<<"success thread "<<i<<"\n";
                 }
             }
    
}
void* creat_group(void* arg)
{
    int* trackerport=(int*)malloc(sizeof(int));
    trackerport=(int*)arg;
    int trackerPortRetrieve=*trackerport;
    int sp=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(trackerPortRetrieve);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    int n;
    int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("errorrrrrrr");
          exit(0);
        }
        else
          cout<<"connected with tracker\n";
        char ackr='G';
        while(1)
        {
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
          else
            break;
        }
        cout<<"enter group id\n";
        int gid;
        cin>>gid;
         if((n=send(sp,&gid,sizeof(int),0))<0)
              perror("sending failed");
         if((n=send(sp,&serverport,sizeof(int),0))<0)
              perror("sending failed");
         char ack;
         recv(sp,&ack,sizeof(ack),0);
         if(ack=='#')
         cout<<"successfully created group\n";
         else
         cout<<"group creation failed\n";
    close(sp);
    pthread_exit(0);
    
}
void* join_group(void* arg)
{
    int* trackerport=(int*)malloc(sizeof(int));
    trackerport=(int*)arg;
    int trackerPortRetrieve=*trackerport;
    int sp=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(trackerPortRetrieve);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    int n;
    int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("errorrrrrrr");
          exit(0);
        }
        else
          cout<<"connected with tracker\n";
        char ackr='J';
       
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
        cout<<"enter group id\n";
        int gid;
        cin>>gid;
         if((n=send(sp,&gid,sizeof(int),0))<0)
              perror("sending failed");
         if((n=send(sp,&serverport,sizeof(int),0))<0)
              perror("sending failed");
         char ack;
         recv(sp,&ack,sizeof(ack),0);
         if(ack=='#')
         cout<<"request sent\n";
         else
         cout<<"request denied\n";
    close(sp);
    pthread_exit(0);
    
}
void* accept_request(void* arg)
{
      int* trackerport=(int*)malloc(sizeof(int));
      trackerport=(int*)arg;
      int trackerPortRetrieve=*trackerport;
      int friendport;
      string mode;
      cout<<"enter the mode for accepting\n";
      cin>>mode;
      int sp;
      if(mode=="single")
        {
           cout<<"enter request\n";
           cin>>friendport;
          ///////////////////////////////////////////add user in the group
           sp=socket(AF_INET,SOCK_STREAM,0);
           struct sockaddr_in addr;
           memset(&addr,'\0',sizeof(addr));
           addr.sin_family=AF_INET;
           addr.sin_port=htons(trackerPortRetrieve);
           addr.sin_addr.s_addr=inet_addr("127.0.0.1");
           int n;
           int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
           if(res<0)
           {
             perror("errorrrrrrr");
             exit(0);
           }
           else
           cout<<"connected with tracker\n";
            char ackr='A';
            if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
            int gid=requestlist[friendport];
            if(send(sp,&gid,sizeof(gid),0)<0)
                perror("gid not sent");
            if(send(sp,&friendport,sizeof(int),0)<0)
                perror("friendport not sent");
            char ack;
          recv(sp,&ack,sizeof(ack),0);
          if(ack=='#')
             {
               cout<<"new user added in your group\n";
               requestlist.erase(friendport);
             }
          else
            cout<<"failed to add new user\n";
            close(sp);
          //////////////////////////////////////////send ack to friendport for their request
           sp=socket(AF_INET,SOCK_STREAM,0);
           memset(&addr,'\0',sizeof(addr));
           addr.sin_family=AF_INET;
           addr.sin_port=htons(friendport);
           addr.sin_addr.s_addr=inet_addr("127.0.0.1");
           res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
           if(res<0)
           {
             perror("errorrrrrrr");
             exit(0);
           }
           else
           cout<<"connected with friendport\n";
           ackr='A';
            if(send(sp,&ackr,sizeof(ackr),0)<0)
               perror("ack not sent");
        }
      close(sp);
    pthread_exit(0); 
}
string getsha(FILE* fp)
{
     unsigned char buffer[BufferSize],outputBuf[20];
	int n;
	char temp[40];
	string hash="",finalHash="";
	while((n=fread(buffer,sizeof(buffer),1,fp))>0){
		hash="";
		buffer[n]='\0';
		SHA1(buffer,n,outputBuf);
		for(int i=0; i<20; i++){
			sprintf(temp,"%02x",outputBuf[i]);
			hash+=temp;
		}
		finalHash+=hash.substr(0,20);
	}
	string s=finalHash.substr(0,20);
	return s;
}
void* upload_files(void* arg)
{
      int* trackerport=(int*)malloc(sizeof(int));
      trackerport=(int*)arg;
      int trackerPortRetrieve=*trackerport;
      int sp=socket(AF_INET,SOCK_STREAM,0);
           struct sockaddr_in addr;
           memset(&addr,'\0',sizeof(addr));
           addr.sin_family=AF_INET;
           addr.sin_port=htons(trackerPortRetrieve);
           addr.sin_addr.s_addr=inet_addr("127.0.0.1");
           int n;
           int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
           if(res<0)
           {
             perror("errorrrrrrr");
             exit(0);
           }
           else
           cout<<"connected with tracker\n";
            char ackr='U';
            if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
            int gid;
            cout<<"enter group_id: \n";
            cin>>gid;
            int n_gid=gid;
            int n_serverport=serverport;
            if(send(sp,&n_gid,sizeof(int),0)<0)
                perror("gid not sent");
            if(send(sp,&n_serverport,sizeof(int),0)<0)
                perror("friendport not sent");
            char ack;
            recv(sp,&ack,sizeof(ack),0);
            if(ack=='$')
            {
                cout<<"first join respective group\n";
            }
            else
            {
            cout<<"you are part of group\n";
            string filename,fileLocation;
            cout<<"enter filename:\n ";
            cin>>filename;
            cout<<"enter FileLocation:\n ";
            cin>>fileLocation;
            char fname[100],floc[100],shaarray[100];
            strcpy(fname,filename.c_str());
            strcpy(floc,fileLocation.c_str());
            if(send(sp,&fname,sizeof(fname),0)<0)
                perror("friendport not sent");
            if(send(sp,&floc,sizeof(floc),0)<0)
                perror("friendport not sent");
            FILE* fp=fopen(fname,"rb");
            string hash=getsha(fp);
            strcpy(shaarray,hash.c_str());
            if(send(sp,&shaarray,sizeof(shaarray),0)<0)
                perror("friendport not sent");
            cout<<"hash value: "<<hash<<"\n";
            fseek(fp,0,SEEK_END);
  	    long long filesize=ftell(fp);
  	    fclose(fp);
  	    if(send(sp,&filesize,sizeof(filesize),0)<0)
                perror("friendport not sent");
            //char ack;
            recv(sp,&ack,sizeof(ack),0);
            if(ack=='#')
             {
               cout<<"uploaded\n";
             }
            else
            cout<<"not uploaded\n";
            close(sp);
          }
            pthread_exit(0);
 }
void* list_groups(void* arg)
{
      int* trackerport=(int*)malloc(sizeof(int));
      trackerport=(int*)arg;
      int trackerPortRetrieve=*trackerport;
      int sp=socket(AF_INET,SOCK_STREAM,0);
           struct sockaddr_in addr;
           memset(&addr,'\0',sizeof(addr));
           addr.sin_family=AF_INET;
           addr.sin_port=htons(trackerPortRetrieve);
           addr.sin_addr.s_addr=inet_addr("127.0.0.1");
           int n;
           int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
           if(res<0)
           {
             perror("errorrrrrrr");
             exit(0);
           }
           else
           cout<<"connected with tracker\n";
            char ackr='L';
            if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
           int size;
           recv(sp,&size,sizeof(int),0);
           while(size--)
           {
                 int gid;
                 recv(sp,&gid,sizeof(int),0);
                 cout<<gid<<" ";
           }
           
}
void* list_files(void* arg)
{     
      int* trackerport=(int*)malloc(sizeof(int));
      trackerport=(int*)arg;
      int trackerPortRetrieve=*trackerport;
      int sp=socket(AF_INET,SOCK_STREAM,0);
           struct sockaddr_in addr;
           memset(&addr,'\0',sizeof(addr));
           addr.sin_family=AF_INET;
           addr.sin_port=htons(trackerPortRetrieve);
           addr.sin_addr.s_addr=inet_addr("127.0.0.1");
           int n;
           int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
           if(res<0)
           {
             perror("errorrrrrrr");
             exit(0);
           }
           else
           cout<<"connected with tracker\n";
            char ackr='F';
            if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
            cout<<"enter groupid:\n";
            int gid;
            cin>>gid;
            if(send(sp,&gid,sizeof(gid),0)<0)
               perror("ack not sent");
           int size;
           recv(sp,&size,sizeof(int),0);
           while(size--)
           {
                 char fname[100];
                 recv(sp,&fname,sizeof(fname),0);
                 cout<<fname<<" ";
           }
           cout<<"\n";
           
}
///////////////////what client will do
void* client(void* arg)
{
     int* trackerport=(int*)malloc(sizeof(int));
     trackerport=(int*)arg;
     int trackerPortRetrieve=*trackerport;
     pthread_t tid_c,tid_s,tid_l;
     pthread_attr_t attr;
     pthread_attr_init(&attr);
     string what;
  while(1)
   {
     cin>>what;
     char ack;
     if(what=="download")
     {
        if(pthread_create(&tid_l,&attr,Download_file,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
    
     }
     else if(what=="creategroup")
     {
            if(pthread_create(&tid_l,&attr,creat_group,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
     }
     else if(what=="joingroup")
     {
            if(pthread_create(&tid_l,&attr,join_group,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
     }
     else if(what=="listrequest")
     {
             map<int,int>::iterator it ;
             for(it =requestlist.begin();it!=requestlist.end();++it)
             {
                 cout <<"requestport " << it->first<<"\n";
             }
     }
     else if(what=="acceptrequest")
     {
             if(pthread_create(&tid_l,&attr,accept_request,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
     }
     else if(what=="listgroups")
     {
             if(pthread_create(&tid_l,&attr,list_groups,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
     }
     else if(what=="upload")
     {
             if(pthread_create(&tid_l,&attr,upload_files,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
     }
     else if(what=="listfiles")
     {
             if(pthread_create(&tid_l,&attr,list_files,&trackerPortRetrieve)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
     }
     else
     cout<<"not coded\n";
   }
     pthread_exit(0);   
       
}

///// server code
void* handle_client(void* arg)
{   
    int *t=(int*)arg;
    int connfd=*t;
    struct chunk_info* chunk=(struct chunk_info*)malloc(sizeof(struct chunk_info));
    if(recv(connfd,chunk,sizeof(struct chunk_info),0)<0)
            perror("receiving error\n");
          int n;
          FILE *fp = fopen(chunk->fname,"rb");
          int offset=(chunk->chunk_no)*BufferSize;
           fseek(fp,offset,SEEK_SET);
            char buffer[BufferSize];
            n=fread(buffer,sizeof(char),BufferSize,fp);
            cout<<"read "<<n<<" bytes\n";
            send(connfd,buffer,n,0);
   	    memset(buffer,'\0',BufferSize); 
   	    cout<<"sent chunk "<<chunk->chunk_no<<"\n";
	    fclose(fp);
            pthread_exit(0);
    
}
void* server(void* arg)
{
     int* pt=(int*)malloc(sizeof(int*));
     pt=(int*)arg;
     int serverport=*pt;
     pthread_t tid;
     pthread_attr_t attr;
     pthread_attr_init(&attr);
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serverport); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    if(listen(listenfd, 10)<0)
    {
          perror("listening error\n");
    } 
    else
    cout<<"listening....\n";
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
          if(connfd<0)
              perror("not accepted");
          char ack;
          recv(connfd,&ack,sizeof(ack),0);
          if(ack=='D')
            {
                if(pthread_create(&tid,&attr,handle_client,&connfd)<0)
                     perror("creation error");
                pthread_join(tid,NULL);
             }
          if(ack=='J')
            {
                char ack='#';
                int RequestPort,gid;
                recv(connfd,&RequestPort,sizeof(int),0);
                recv(connfd,&gid,sizeof(int),0);
                requestlist[RequestPort]=gid;
                cout<<"request Queued\n";
                send(connfd,&ack,sizeof(ack),0);
             }
          if(ack=='A')
          {
              cout<<"Request Accepted\n";
          }
        close(connfd);
        fflush(stdout); 
     }
     close(listenfd);
}
////// register user //////////
void* creat_user(void* arg)
{       
    int* trackerport=(int*)malloc(sizeof(int));
    trackerport=(int*)arg;
    int trackerPortRetrieve=*trackerport;
    int sp=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(trackerPortRetrieve);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int n;
    int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("errorrrrrrr");
          exit(0);
        }
        else
          cout<<"connected with tracker\n";
        char ackr='R';
        while(1)
        {
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("ack not sent");
          else
            break;
        }
        string username,passwd;
        cout<<"enter username\n";
        //getline(cin,username);
        cin>>username;
        cout<<"enter password\n";
         cin>>passwd;
        char user[100],pass[100];
         strcpy(user,username.c_str());
         strcpy(pass,passwd.c_str());
         if((n=send(sp,&user,sizeof(user),0))<0)
              perror("sending failed");
         if((n=send(sp,&pass,sizeof(pass),0))<0)
              perror("sending failed");
              char ack;
         recv(sp,&ack,sizeof(ack),0);
         if(ack=='#')
         cout<<"successfully registered\n";
         else
         cout<<"registration failed\n";
    close(sp);
    pthread_exit(0);
}
//////////verify user///////////////
void* verify_user(void* arg)
{       
    int* trackerport=(int*)malloc(sizeof(int));
    trackerport=(int*)arg;
    int trackerPortRetrieve=*trackerport;
    int sp=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(trackerPortRetrieve);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
     int n;
    int res =  connect(sp,(struct sockaddr*)&addr,sizeof(addr));
        if(res<0)
        {
          perror("error in connection with tracker\n");
          exit(0);
        }
        else
          cout<<"connected with tracker\n";
        char ackr='V';
        while(1)
        {
           if(send(sp,&ackr,sizeof(ackr),0)<0)
            perror("verification ack not sent");
          else
            break;
        }
        string username,passwd;
        cout<<"enter username\n";
        //getline(cin,username);
        cin>>username;
        cout<<"enter password\n";
         cin>>passwd;
        char user[100],pass[100];
         strcpy(user,username.c_str());
         strcpy(pass,passwd.c_str());
         if((n=send(sp,&user,sizeof(user),0))<0)
              perror("sending verifying username failed");
         if((n=send(sp,&pass,sizeof(pass),0))<0)
              perror("sending verifying password failed");
              char ack;
         recv(sp,&ack,sizeof(ack),0);
         if(ack=='#')
         cout<<"successfully verified\n";
         else
         cout<<"verification failed\n";
    close(sp);
    pthread_exit(0);
}

int main(int argc,char* argv[])
{
     pthread_t tid_c,tid_s,tid_l;
     pthread_attr_t attr;
     pthread_attr_init(&attr);
     string what;
     serverport=atoi(argv[1]);
     int tracker_port=atoi(argv[2]);
     trackerportno=tracker_port;
     //cout<<port<<"\n";
     cout<<"for new user press[Y/N] ?";
     char d;
     cin>>d;
     if(d=='Y')
      {
            if(pthread_create(&tid_l,&attr,creat_user,&tracker_port)<0)
             perror("creation error");
             pthread_join(tid_l,NULL);
      }
     else
     {
        if(pthread_create(&tid_l,&attr,verify_user,&tracker_port)<0)
             perror("creation error");
          pthread_join(tid_l,NULL);
     }
     if(pthread_create(&tid_s,&attr,server,&serverport)<0)
           perror("creation error");
     if(pthread_create(&tid_c,&attr,client,&tracker_port)<0)
           perror("creation error");
        pthread_join(tid_s,NULL);
        pthread_join(tid_c,NULL);
       
     return 0;
}
