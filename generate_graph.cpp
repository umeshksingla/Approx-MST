#include <bits/stdc++.h>

#define f(i,a,n) for(int i=a;i<n;i++)
#define S second
#define F first
#define Sc(n) scanf("%lld",&n)
#define scc(a,b,c) scanf("%lld %lld %lld",&a,&b,&c)
#define sp(a) scanf("%lld %lld",&a.first,&a.second)
#define pb push_back
#define mp make_pair
#define lb lower_bound
#define ub upper_bound
#define all(a) a.begin(),a.end()
#define sc(n) scanf("%d",&n)
#define It iterator
#define SET(a,b) memset(a,b,sizeof(a))
#define DRT()  int t; cin>>t; while(t--)

// inbuilt functions
// __gcd,  __builtin_ffs,     (returns least significant 1-bit, __builtin_ffsll(1)=1)
// __builtin_clz,             (returns number of leading zeroes in 
// __builtin_popcount,

using namespace std;

typedef long long LL;
typedef pair<int,int> PII;
typedef vector<int> vi;
#define tr(container, it) for(__typeof(container.begin()) it = container.begin(); it != container.end(); it++)
#define trv(s,it) for(auto it:s)

int main()
{
	ios_base::sync_with_stdio(0);cin.tie(NULL);cout.tie(NULL);
	int n,m;
	cin>>n>>m;
	cout<<n<<" "<<m<<endl;
	map< pair<int,int> , bool> M;
	
	srand(time(NULL));
	f(i,0,n-1)
	{
		int w=(rand()*101)%10;
		w=abs(w)+1;
		cout<<i<<" "<<i+1<<" "<<w<<endl;
		M[mp(i,i+1)]=M[mp(i+1,i)]=true;
	}

	f(i,0,m-n+1)
	{
		int x=(rand()*1009)%n;
		int y=(rand()*1009)%n;
		int w=(rand()*101)%10;
		x=abs(x);y=abs(y);w=abs(w)+1;
		if(M[mp(x,y)]==false && M[mp(y,x)]==false && x!=y)
		{
			cout<<x<<" "<<y<<" "<<w<<endl;
			M[mp(x,y)]=M[mp(y,x)]=true;	
		}
		else
			i--;
	}
	return 0;
}
