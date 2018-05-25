#include<iostream>
#include<algorithm>
#include<vector>
#include "mpi.h"
#define N 1003

using namespace std;

struct edges {
  int to;
  int dist;
} tmp;

struct mst_s {
  int first;
  int second;
  int dist;
} tmp_mst;

vector<edges> adj[N]; //Adj list
vector<mst_s> mst; //mst
int mark[N]; //To mark vertices in mst

int main(int argc, char **argv){
  MPI_Comm new_comm;
  MPI_Status status;
  int sum = 0,rank, size, i, j, a, b, c, e, v, fake_v, x, y, z;
  int chunk, rem;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  if(rank==0){ //Master
    cin>>v>>e; chunk = v/(size-1); rem = v%(size-1);//Chunk_Size
    //fake_v = v;
    //while(fake_v%(size-1))
    for(i=0; i<e; i++){ //Add to adj list
      cin>>a>>b>>tmp.dist;
      a++;
      b++;
      tmp.to = b; adj[a].push_back(tmp);
      tmp.to = a; adj[b].push_back(tmp);
    }
    mark[0] = 0; for(i=1; i<=v; i++)mark[i] = adj[i].size();
    MPI_Bcast(&v, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&chunk, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rem, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mark, N, MPI_INT, 0, MPI_COMM_WORLD);
    for(i=1; i<=v; i++)MPI_Bcast(&adj[i].front(), mark[i]*2, MPI_INT, 0, MPI_COMM_WORLD);

    for(int k=0; k<v-1; k++){
      MPI_Bcast(&mark, N, MPI_INT, 0, MPI_COMM_WORLD);
      int minDist = 99999;
      for(i=1; i<size; i++){
        MPI_Recv( &x, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status );
        MPI_Recv( &y, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status );
        MPI_Recv( &z, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status );
        //cout<<"Received candidate "<<x<<" "<<y<<" "<<z<<endl;
        if(z<minDist)tmp_mst.first=x,tmp_mst.second=y,tmp_mst.dist=z,minDist=z;
      }
      mst.push_back(tmp_mst);
      mark[tmp_mst.first] = mark[tmp_mst.second] = -1; //Mark added vertices
    }
  for(int k=0; k<v-1; k++) {
  cout<<mst[k].first<<" "<<mst[k].second<<" "<<mst[k].dist<<endl;
  sum += mst[k].dist;
  }
  cout<<sum<<endl;
  }
  else{ //Slave
    MPI_Bcast(&v, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&chunk, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rem, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mark, N, MPI_INT, 0, MPI_COMM_WORLD);
    for(i=1; i<=v; i++)adj[i].resize(mark[i]),MPI_Bcast(&adj[i].front(), mark[i]*2, MPI_INT, 0, MPI_COMM_WORLD);

    for(int k=0; k<v-1; k++){
      MPI_Bcast(&mark, N, MPI_INT, 0, MPI_COMM_WORLD);
      //Send a minimum edge
      int minDist = 99999;
      x = y = z = -1; z = 99999;
      for(i=(rank-1)*chunk+1; i<=rank*chunk+(rank==size-1?rem:0); i++)
      //cout<<"Considering for rank "<<rank<<" vertex "<<i<<endl;
      for(i=(rank-1)*chunk+1; i<=rank*chunk+(rank==size-1?rem:0); i++)
      for(j=0; j<mark[i]; j++)
      if(adj[i][j].dist < minDist)
      if(k==0 || mark[adj[i][j].to]==-1){
        //First step, send two vertices, then on, any connected to the MST
        edges curr = adj[i][j];
        minDist = curr.dist; x = i; y = curr.to; z = curr.dist;
      }
      //cout<<"Success! Sending "<<x<<" "<<y<<" with distance "<<z<<endl;
      MPI_Send(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&y, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&z, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  }

  MPI_Finalize();
}
