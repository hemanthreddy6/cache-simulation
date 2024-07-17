#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

ll SIZE_OF_CACHE,BLOCK_SIZE,ASSOCIATIVITY;
string REPLACEMENT_POLICY,WRITEBACK_POLICY;
ll LINE_SIZE,SET_SIZE,OFFSET_SIZE;
ll SET_BITS,TAG_BITS,OFFSET_BITS,SET_SHIFT,TAG_SHIFT,OFFSET_SHIFT;
ll SET_MASK,TAG_MASK,OFFSET_MASK;
vector<string>input,output;

// Function to initialize all variables
void init()
{
    ifstream in("cache.config");
    string s;
    getline(in,s);
    SIZE_OF_CACHE=stoll(s);
    getline(in,s);
    BLOCK_SIZE=stoll(s);
    getline(in,s);
    ASSOCIATIVITY=stoll(s);
    getline(in,s);
    REPLACEMENT_POLICY=s;
    getline(in,s);
    WRITEBACK_POLICY=s;

    if(ASSOCIATIVITY!=0)
    {
        LINE_SIZE=BLOCK_SIZE*ASSOCIATIVITY;
        SET_SIZE=SIZE_OF_CACHE/LINE_SIZE;
    }
    else
    {
        LINE_SIZE=SIZE_OF_CACHE;
        SET_SIZE=1;
        ASSOCIATIVITY=SIZE_OF_CACHE/BLOCK_SIZE;
    }

    OFFSET_BITS=log2(BLOCK_SIZE);
    OFFSET_SHIFT=0;
    OFFSET_MASK=BLOCK_SIZE-1;
    SET_BITS=log2(SET_SIZE);
    SET_SHIFT=OFFSET_BITS;
    SET_MASK=SET_SIZE-1;
    TAG_BITS=32-OFFSET_BITS-SET_BITS;
    TAG_SHIFT=OFFSET_BITS+SET_BITS;
    TAG_MASK=(1<<(32-OFFSET_BITS-SET_BITS))-1;
}

// Returns the binary string of a hexadecimal instruction
string to_binary(string s)
{
    bitset<32>b(stoul(s,nullptr,16));
    return (b.to_string());
}

// Returns the hexadecimal string of a binary nubmer
string to_hexa(string s)
{
    reverse(s.begin(),s.end());
    ll x=s.length()%4;
    if(x!=0)
    {
        for(ll i=0;i<4-x;i++)
        {
            s+='0';
        }
    }
    string answer;
    for(ll i=0;i<s.length();i+=4)
    {
        x=(s[i]-'0')+2*(s[i+1]-'0')+4*(s[i+2]-'0')+8*(s[i+3]-'0');
        if(x<=9)
        answer.push_back(x+'0');
        else
        answer.push_back(x-10+'a');
    }
    answer+="x0";
    reverse(answer.begin(),answer.end());
    return answer;
}

// Returns the hexadecimal string of a decimal number
string to_hex(string s)
{
    bitset<32>b(stoul(s,nullptr,10));
    return to_hexa(b.to_string());
}

// Function to get offset
ll get_offset(string s)
{
    bitset<64>b(s);
    ll n=b.to_ullong();
    n>>=OFFSET_SHIFT;
    n&=OFFSET_MASK;
    return n;
}

// Function to get set index
ll get_set_index(string s)
{
    bitset<64>b(s);
    ll n=b.to_ullong();
    n>>=SET_SHIFT;
    n&=SET_MASK;
    return n;
}

// Function to get tag
ll get_tag(string s)
{
    bitset<64>b(s);
    ll n=b.to_ullong();
    n>>=TAG_SHIFT;
    n&=TAG_MASK;
    return n;
}

// Function to build the output from the parameters
string build(string address,ll set_index,string verdict,ll tag)
{
    string s="Address: ";
    s+=address.substr(3,address.length()-3);
    s+=", Set: ";
    s+=to_hex(to_string(set_index));
    s+=", ";
    s+=verdict;
    s+=", Tag: ";
    s+=to_hex(to_string(tag));
    return s;
}

int main()
{
    freopen("cache.access","r",stdin);
    freopen("output.txt","w",stdout);
    init();
    string s;
    char type;
    ll set_index,tag,offset,x,y,z;
    vector<vector<ll>>cache(SET_SIZE,vector<ll>(ASSOCIATIVITY,-1));
    vector<vector<ll>>valid(SET_SIZE,vector<ll>(ASSOCIATIVITY,0));
    vector<vector<ll>>dirty(SET_SIZE,vector<ll>(ASSOCIATIVITY,0));
    vector<vector<ll>>lru(SET_SIZE,vector<ll>(ASSOCIATIVITY,1e17));
    vector<vector<ll>>fifo(SET_SIZE,vector<ll>(ASSOCIATIVITY,1e17));
    while(getline(cin,s))
    {
        type=s[0];
        input.push_back(s);
        s=s.substr(5,s.length()-5);
        s=to_binary(s);
        set_index=get_set_index(s);
        tag=get_tag(s);
        if(type=='R')
        {
            x=-1;
            // Searching if it's present already in the cache
            for(ll i=0;i<ASSOCIATIVITY;i++)
            {
                if(cache[set_index][i]==tag&&valid[set_index][i]==1)
                {
                    x=i;
                    break;
                }
            }

            // If it's already present, We just update LRU time
            if(x!=-1)
            {
                cache[set_index][x]=tag;
                lru[set_index][x]=input.size();
                output.push_back(build(input.back(),set_index,"Hit",tag));
                continue;
            }

            // If not present, We search for an empty spot
            for(ll i=0;i<ASSOCIATIVITY;i++)
            {
                if(cache[set_index][i]==-1)
                {
                    x=i;
                    break;
                }
            }

            // If an empty spot is found, We update the tag, validity, lru and fifo time
            if(x!=-1)
            {
                cache[set_index][x]=tag;
                valid[set_index][x]=1;
                lru[set_index][x]=input.size();
                if(REPLACEMENT_POLICY=="FIFO")
                {
                    fifo[set_index][x]=input.size();
                }
                output.push_back(build(input.back(),set_index,"Miss",tag));
                continue;
            }
            
            // If there's no empty spot for it, We use replacement policy
            y=1e17;
            for(ll i=0;i<ASSOCIATIVITY;i++)
            {
                if(REPLACEMENT_POLICY=="LRU"&&lru[set_index][i]<y)
                {
                    y=lru[set_index][i];
                    x=i;
                }
                else if(REPLACEMENT_POLICY=="FIFO"&&fifo[set_index][i]<y)
                {
                    y=fifo[set_index][i];
                    x=i;
                }
            }
            if(REPLACEMENT_POLICY=="RANDOM")
            {
                x=rand()%ASSOCIATIVITY;
            }
            cache[set_index][x]=tag;
            valid[set_index][x]=1;
            lru[set_index][x]=input.size();
            fifo[set_index][x]=input.size();
            dirty[set_index][x]=0;
            output.push_back(build(input.back(),set_index,"Miss",tag));
        }
        else
        {
            x=-1;
            // Searching if it's present already in the cache
            for(ll i=0;i<ASSOCIATIVITY;i++)
            {
                if(cache[set_index][i]==tag&&valid[set_index][i]==1)
                {
                    x=i;
                    break;
                }
            }

            // If it's already present, We just update LRU time
            if(x!=-1)
            {
                cache[set_index][x]=tag;
                valid[set_index][x]=1;
                lru[set_index][x]=input.size();
                if(WRITEBACK_POLICY=="WB")
                dirty[set_index][x]=1;
                output.push_back(build(input.back(),set_index,"Hit",tag));
                continue;
            }

            // If it's write through, we directly get a miss
            if(WRITEBACK_POLICY=="WT")
            {
                output.push_back(build(input.back(),set_index,"Miss",tag));
                continue;
            }
            x=-1;

            // If not present, We search for an empty spot
            for(ll i=0;i<ASSOCIATIVITY;i++)
            {
                if(cache[set_index][i]==-1)
                {
                    x=i;
                    break;
                }
            }

            // If an empty spot is found, We update the tag, validity, lru and fifo time
            if(x!=-1)
            {
                cache[set_index][x]=tag;
                valid[set_index][x]=1;
                lru[set_index][x]=input.size();
                if(WRITEBACK_POLICY=="WB")
                dirty[set_index][x]=1;
                if(REPLACEMENT_POLICY=="FIFO")
                {
                    fifo[set_index][x]=input.size();
                }
                output.push_back(build(input.back(),set_index,"Miss",tag));
                continue;
            }
            y=1e17;
            x=-1;

            // If there's no empty spot for it, We use replacement policy
            for(ll i=0;i<ASSOCIATIVITY;i++)
            {
                if(REPLACEMENT_POLICY=="LRU"&&lru[set_index][i]<y)
                {
                    y=lru[set_index][i];
                    x=i;
                }
                else if(REPLACEMENT_POLICY=="FIFO"&&fifo[set_index][i]<y)
                {
                    y=fifo[set_index][i];
                    x=i;
                }
            }
            if(REPLACEMENT_POLICY=="RANDOM")
            {
                x=rand()%ASSOCIATIVITY;
            }
            cache[set_index][x]=tag;
            valid[set_index][x]=1;
            dirty[set_index][x]=1;
            lru[set_index][x]=input.size();
            fifo[set_index][x]=input.size();
            output.push_back(build(input.back(),set_index,"Miss",tag));
            continue;
        }
    }
    for(auto i:output)
    {
        cout << i << endl;
    }
    return 0;
}