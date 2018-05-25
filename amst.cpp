#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <map>
#include <time.h>
#include <mpi.h>
#include <climits>

#define F first
#define S second
#define pb push_back
#define mp make_pair
#define SZ 7
#define TIMEOUT_TIME 1
using namespace std;


int main(int argc, char** argv) 
{	
	int mrank,rank;
	int size;
	int ps=INT_MIN;
	vector< pair<int,int> > v;
	int provided;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	/*if (provided == MPI_THREAD_MULTIPLE) {
		cout << "mpi didn't provided multiple\n";
		//return 0;
	}*/
	MPI_Comm_rank(MPI_COMM_WORLD, &mrank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;
	MPI_Request request;
	rank=mrank;
	int len;
	int *graph, *weight;
	int n=-1,m,x,y,w;
	int adj[1000][1000];
	//vector<int> graph;
	if(mrank==0)
	{

		//taking INPUT
		cin>>n>>m;
		vector<int> g[n];
		vector<int> W[n];

		for(int i=0;i<m;i++)
		{
			cin>>x>>y>>w;
			g[x].push_back(y);
			g[y].push_back(x);
			W[x].pb(w);
			W[y].pb(w);
			adj[x][y]=w;
			adj[y][x]=w;
		}
		len=g[0].size();
		graph=new int [len];weight = new int [len];
		copy(g[0].begin(),g[0].end(),graph);
		copy(W[0].begin(),W[0].end(),weight);
		// need n MPI processes for n nodes

		for(int i=1;i<size;i++)
		{
			int L=g[i].size();
			int temp[L],wtemp[L];
			copy(g[i].begin(),g[i].end(),temp);
			copy(W[i].begin(),W[i].end(),wtemp);
			MPI_Send(&L,1,MPI_INT,i,1,MPI_COMM_WORLD);
			MPI_Send(temp,g[i].size(),MPI_INT,i,2,MPI_COMM_WORLD);
			MPI_Send(wtemp,g[i].size(),MPI_INT,i,2,MPI_COMM_WORLD);
		}

		//int *neighbors = (int *)malloc(graph.size()*sizeof(int));
		//MPI_Graph_neighbors(MPI_COMM_WORLD, rank, graph.size(), neighbors);

		// Starting to find NNT

		ps=1e8+7; // bigger than the number of nodes + 1
		int buff[2],buf[2];
		buff[0]=ps;buff[1]=rank;
		for(int i=0;i<len;i++)
		{
			int flag;
			// send message with ps to all neighbors
			MPI_Isend(buff,2,MPI_INT,graph[i],1,MPI_COMM_WORLD,&request);
			MPI_Irecv(buf,2,MPI_INT,graph[i],1,MPI_COMM_WORLD,&request);
			MPI_Wait(&request,&status);
			v.push_back(make_pair(buf[0],buf[1]));
		}

	}
	else
	{
		MPI_Recv(&len,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		graph=new int [len];weight = new int [len];

		MPI_Recv(graph,len,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		MPI_Recv(weight,len,MPI_INT,0,2,MPI_COMM_WORLD,&status);

		//int *neighbors = (int *)malloc(graph.size()*sizeof(int));
		//MPI_Graph_neighbors(MPI_COMM_WORLD, rank, graph.size(), neighbors);
		for(int i=0;i<len;i++)
		{
			int buff[2],buf[2],flag=1;
			MPI_Irecv(buf,2,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&request);
			MPI_Wait(&request,&status);
			if(ps==INT_MIN)
			{
				ps=buf[0]-1;
				buff[0]=ps;buff[1]=rank;
				for(int j=0;j<len;j++)
					MPI_Isend(buff,2,MPI_INT,graph[j],1,MPI_COMM_WORLD,&request);
			}
			v.push_back(make_pair(buf[0],buf[1]));
		}

		// Testing output

		/*if(mrank==2)
		{
			cout<<len<<endl;
			for(int i=0;i<len;i++)
				cout<<graph[i]<<endl;
			for(int i=0;i<v.size();i++)
				cout<<v[i].first<<" "<<v[i].second<<endl;
		}*/

	}
	// cout<<"Process "<<rank<<" "<<ps<<endl;
	MPI_Barrier(MPI_COMM_WORLD);

	//cout<<rank<<" "<<ps+rank<<endl;
	//return 0;
	
	// Initiating exploration
	int phase=1,d=1,pd=1,l=0;
	int ex[SZ],tmp[SZ];

	map< pair<int, pair<int,int> > , int > path;
	map< pair<int, pair<int,int> > , bool > counted;
	map<int,int> T;
	map<int,int> sent;
	map<int,int> wait,count,found;
	int spd=INT_MAX,sl=INT_MAX;
	bool yes=false;
	int totalrecv=0,totalsent=0;
	int pcount=0,ct=0,replies=0,round=0;
	map< pair<int,int> , int> pre;
	bool connect=false;
	map<int,bool> fis;
	T[rank]=rank;
	vector< pair<int,int> > edges;
	int connection=-1;int leave=0;bool updated=false;
	bool SS=true;

	map< pair<int,int> , int> NUL;
	time_t t1;
	while(1)
	{
		//cout<<"Process starting "<<mrank<<endl;
		if(mrank!=0 && connect==false)
		{	
			for(int i=0;i<len;i++)
			{
				// asynchronous send of explore messages to all neighbors
				pd=weight[i];
				//p=(log(pd)/log(2)) + 1;
				ex[0]=rank;ex[1]=phase;ex[2]=d;ex[3]=pd;ex[4]=l;ex[5]=0;ex[6]=ps;
				MPI_Isend(ex,SZ,MPI_INT,graph[i],mrank,MPI_COMM_WORLD,&request);
			}
			// cout<<"Process exploring "<<mrank<<endl;
			t1=time(0);
		}

		if(mrank!=0 && connect && connection==-1)
		{
			int tor[SZ];
			tor[0]=rank;tor[1]=phase;tor[2]=d;tor[3]=pd;tor[4]=l;tor[5]=5;tor[6]=ps;
			MPI_Isend(tor,SZ,MPI_INT,pre[mp(mrank,T[mrank])],mrank,MPI_COMM_WORLD,&request);
			// cout<<"Process "<<mrank<<" sending connect message to "<<pre[mp(mrank,T[mrank])]<<endl;
			connection=pre[mp(mrank,T[mrank])];
		}

		// explore - 0 ; wait - 1 ; count - 2 ; found - 3; Done - 4; Connect - 5; rank-update - 6; BREAK - 7;
		//int found=0,wait=0,count=0;
		//A:
		//int timeout=0;
		while(1)		// break condition? 
		{	int flag;
			if(mrank==1){
				// cout<<"HERE"<<endl;
			}
			MPI_Irecv(tmp,SZ,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&request);
			// time_t t2=time(0);
			// MPI_Test(&request,&flag,&status);

			// while(!flag)
			// {
			// 	if(difftime(time(0),t2)>1.5)
			// 	{
			// 		found[mrank]=1;updated=true;
			// 		connect=true;
			// 		connection=0;
			// 		replies=len;
			// 		//leave=1;
			// 		break;
			// 	}
			// 	MPI_Test(&request,&flag,&status);
			// }
			MPI_Wait(&request,&status);
			if(mrank==1){
				// cout<<"THERE"<<endl;
			}
			/*if(difftime(time(0),t2)>TIMEOUT_TIME)
				break;*/
			//time_t t2=time(0);
			/*if((t2-t1)>=0.01)
			{
				timeout=1;
				cout<<"Process timeout "<<mrank<<endl; 
    			break;
    		}*/
			//MPI_Test(&request,&flag,&status);
			//if(flag){
				totalrecv++;
				int source=status.MPI_SOURCE;
				int tag=status.MPI_TAG;
			//if(tag==2)
				// cout<<"Received a message process "<<mrank<<" of type "<<tmp[5]<<" pd: "<<tmp[3]<<" l: "<<tmp[4]<<" from "<<source<<endl;
			if(NUL[mp(tag,tmp[1])]!=tmp[2])
			{
				count[tag]=found[tag]=wait[tag]=0;
				NUL[mp(tag,tmp[1])]=tmp[2];
			}

			if(sent[tag]>0)
				yes=true;
			if(tag==mrank && tmp[5]!=0)
				replies++;
			if(tmp[5]==0) // explore
			{
				int result=path[mp(tmp[0],mp(tmp[1],tmp[2]))];
				//if(!result)
				//	result=INT_MAX;
				if(result!=0 || tmp[3]>tmp[1] || tag==mrank)
				{
					int co[SZ]={0};
					for(int j=0;j<SZ;j++)
						co[j]=tmp[j];
					co[0]=tmp[0];co[1]=tmp[1];co[2]=tmp[2];
					co[5]=2;co[4]=0;
					MPI_Isend(co,SZ,MPI_INT,source,tag,MPI_COMM_WORLD,&request);
					totalsent++;
				}
				else
				{
					path[mp(tmp[0],mp(tmp[1],tmp[2]))]=tmp[3];
					if(ps>tmp[6] || (ps==tmp[6] && rank<tmp[0]))
					{
						int fo[SZ]={0};
						for(int j=0;j<SZ;j++)
							fo[j]=tmp[j];
						fo[5]=3;fo[4]=tmp[4];fo[3]=tmp[3];
						fis[tag]=true;
						MPI_Isend(fo,SZ,MPI_INT,source,tag,MPI_COMM_WORLD,&request);

					}
					else
					{
						int f=0;
						/*if((mrank==4 || mrank==5)&& tag==2)
							cout<<"L: "<<tmp[4]<<" "<<tmp[3]<<endl;*/
						for(int i=0;i<len;i++)
						{
							if(tmp[4]>0 && graph[i]!=source && graph[i]!=tag)
							{
								yes=true;
								// f=1;
								int fow[SZ];
								for(int j=0;j<SZ;j++)
									fow[j]=tmp[j];
								fow[4]-=1;fow[3]+=weight[i];
								//if((mrank==4 || mrank==5)&& tag==2)
								if(fow[3]<=fow[1])
								{
									f = 1;
									// cout<<"Process "<<mrank<<" forwarding to process "<<graph[i]<<" initiated by "<<tag<<endl;
									T[tag]=source;
									MPI_Isend(fow,SZ,MPI_INT,graph[i],tag,MPI_COMM_WORLD,&request);
									sent[tag]++;
								} else {
									//cout << "mrank: " << mrank << " pd > phase  neigh: " << graph[i] << endl;
								}
							} else {
								//cout << "mrank: " << mrank <<" l <=0 neigh: " << graph[i] << endl;
							}
						}
						if(!f || tmp[4]<=0)
						{
							int fow[SZ]={0};
							for(int j=0;j<SZ;j++)
									fow[j]=tmp[j];
							fow[0]=tmp[0];fow[1]=tmp[1];fow[2]=tmp[2];
							fow[5]=2;
							if(counted[mp(tag,mp(tmp[1],tmp[2]))]==false)
							{
								/*if(mrank==5 && tag==2)
									cout<<"Count : 1"<<endl;*/

								fow[4]=1;
								counted[mp(tag,mp(tmp[1],tmp[2]))]=true;
							}
							else
								fow[4]=0;
							/*if(tag==2 && mrank==4)
								cout<<"THERE: "<<fow[4]<<endl;*/
							//cout<<"Process "<<mrank<<" sending count reply to "<<source<<endl;
							MPI_Isend(fow,SZ,MPI_INT,source,tag,MPI_COMM_WORLD,&request);
						} else {
							//cout << "mrank: "<<mrank<<" here in explore\n";
						}

					}
				}
			}
			else if(tmp[5]==1) // wait
			{	
				sent[tag]--;
				wait[tag]++;
			}
			else if(tmp[5]==2)
			{
				sent[tag]--;
				count[tag]+=tmp[4];
			}
			else if(tmp[5]==3)
			{
				sent[tag]--;
				found[tag]++;
				//cout<<"Process "<<mrank<<" Tag "<<tag<<" found: "<<found[tag]<<" by "<<source<<endl;
				//spd=min(spd,tmp[3]);
				//sl=min(sl,tmp[4]);
				if((spd>tmp[3]) || (spd==tmp[3] && sl<tmp[4]))
				{
					spd=tmp[3];
					sl=tmp[4];
					pre[mp(tag,T[tag])]=source;
				}
				else{
					//cout<<"Process "<<mrank<<" spd: "<<spd<<" sl: "<<sl<<endl;
				}
			}
			else if(tmp[5]==4)
			{
				edges.pb(mp(source,tmp[4]));
				ct++;
				//cout<<"Received done message from "<<tag<<" "<<ct<<endl;
			}
			else if(tmp[5]==5)
			{
				if(fis[tag]==true || (ps>tmp[6] || (ps==tmp[6] && rank<tmp[0])))
				{
					// send back rank update message
					//cout<<"Process "<<mrank<<" sent rank update to "<<source<<endl;
					int ru[SZ]={0};
					for(int j=0;j<SZ;j++)
						ru[j]=tmp[j];
					ru[5]=6;ru[0]=tmp[0];ru[6]=tmp[6];ru[4]=rank;
					MPI_Isend(ru,SZ,MPI_INT,source,tag,MPI_COMM_WORLD,&request);
				}
				else
				{
					T[tag]=source;
					int torr[SZ]={0};
					for(int j=0;j<SZ;j++)
						torr[j]=tmp[j];
					connect=true;
					torr[5]=5;torr[0]=tmp[0];torr[6]=tmp[6];
					MPI_Isend(torr,SZ,MPI_INT,pre[mp(tag,T[tag])],tag,MPI_COMM_WORLD,&request);
					connection=pre[mp(tag,T[tag])];
				}
			}
			else if(tmp[5]==6)
			{
				rank=tmp[4];
				if(mrank!=tag)
				{
					int ru[SZ]={0};
					for(int j=0;j<SZ;j++)
						ru[j]=tmp[j];
					ru[5]=6;ru[0]=tmp[0];ru[6]=tmp[6];ru[4]=tmp[4];
					MPI_Isend(ru,SZ,MPI_INT,T[tag],tag,MPI_COMM_WORLD,&request);
				}
				else{
					// cout<<"Process "<<mrank<<" received rank update from "<<source<<endl;
				}
				updated=true;
			}
			else if(tmp[5]==7)
			{
				leave=1;
				//cout<<"Received break message: "<<mrank<<endl;
				break;
			}
			else {
				//cout << "message type invalid\n";
				break;
			}
			if((replies==len || (updated && connect))&& mrank!=0 && SS)
				break;
			if(sent[tag]==0 && yes)
			{
				int rep[SZ]={0};
				for(int j=0;j<SZ;j++)
					rep[j]=tmp[j];
				rep[0]=tmp[0];rep[1]=tmp[1];rep[2]=tmp[2];
				if(found[tag]>0)
				{
					rep[5]=3;
					rep[4]=sl;rep[3]=spd;

					MPI_Isend(rep,SZ,MPI_INT,T[tag],tag,MPI_COMM_WORLD,&request);
				}
				else if(wait[tag]>0)
				{
					rep[5]=1;
					//rep[4]=sl;rep[3]=spd;
					MPI_Isend(rep,SZ,MPI_INT,T[tag],tag,MPI_COMM_WORLD,&request);	
				}
				else
				{
					rep[5]=2;
					rep[4]=count[tag]+(counted[mp(tag,mp(tmp[1],tmp[2]))]==false?1:0);
					/*if(tag==2 && mrank==4)
						cout<<"Sending "<<count[tag]<<" "<<counted[mp(tag,mp(tmp[1],tmp[2]))]<<endl;*/
					MPI_Isend(rep,SZ,MPI_INT,T[tag],tag,MPI_COMM_WORLD,&request);
					counted[mp(tag,mp(tmp[1],tmp[2]))]=true;
				}
				//cout<<"Process sending explore replies: "<<mrank<<endl;
			}
			//flag=-1;
			//}
			yes=false;

			/*if(mrank==4)
				cout<<"value print: "<<counted[mp(2,mp(2,2))]<<endl;*/
			if(mrank==0 && (ct==n-1 || (ct==n-4 && n>=50) || (ct==n-6 && n>100)))
			{
				for(int i=1;i<size;i++)
				{
					int BR[SZ]={0};
					for(int j=0;j<SZ;j++)
									BR[j]=tmp[j];
					BR[5]=7;
					MPI_Isend(BR,SZ,MPI_INT,i,mrank,MPI_COMM_WORLD,&request);
				}
				break;
			}
			//cout<<"Process "<<mrank<<" has got replies= "<<replies<<endl;

		}
		//cout<<"First OUT "<<mrank<<" "<<leave<<endl;
		//if(timeout)
		//	goto A;
		round++;
		if(!updated){
			//cout<<"Phase: "<<phase<<" and Round: "<<d<<" for Process: "<<mrank<<endl;
		}
		else {
			//cout<<"Process "<<mrank<<" is out of the loop"<<endl;
		}
		//if(phase==0)
		//	return 0;
		if(leave==1 || mrank==0) 
			break;
		if(leave==1 || mrank==0) 
			break;
		//cout<<"Process "<<mrank<<" found: "<<found[mrank]<<endl;
		if(found[mrank]>0 || connection!=-1)
			connect=true;
		else
		{
			//if(mrank==2)
			// cout<<"Process count of "<<mrank<<" = "<<count[mrank]<<endl;

			if(d==1)
				d=2;
			else if(d>1 && count[mrank]>pcount)
				d*=2;
			else
			{
				d=1;
				phase*=2;
			}
			pcount=count[mrank];
			count[mrank]=wait[mrank]=found[mrank]=0;
			l=d-1;
			replies=0;
		}
		if(mrank!=0 && connect && updated && SS)
		{
			int REP[SZ]={0};

			REP[5]=4;REP[4]=connection;
			MPI_Isend(REP,SZ,MPI_INT,0,mrank,MPI_COMM_WORLD,&request);
			SS=false;
			// cout<<"Process done: "<<mrank<<endl;
		}
		//replies=0;
	}
	//cout<<"Process ending "<<mrank<<endl;
	
	// How to test if every node has found some node?

	// on to connecting but do we have enough information?
	MPI_Barrier(MPI_COMM_WORLD);


	if(mrank==0)
	{
		int sum = 0;
		//cout<<"Edges in the MST are:"<<endl;
		for(int i=0;i<edges.size();i++){
			cout<<edges[i].F<<" "<<edges[i].S<<endl;
			sum += adj[edges[i].F][edges[i].S];
		}
		cout<<"Total weight: "<<sum<<endl;
	}
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Finalize();
	
}
