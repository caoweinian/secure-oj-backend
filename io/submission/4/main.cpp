// SPACE LIMIT EXCEEDED (stack overflow)


#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
using namespace std;

void f(int i){f(i+1);}
int main() {
    f(4);
    // constexpr size_t sz = 10*1024*1536;
    // char *s = (char*)malloc(sz);
    // cout<<(s==nullptr)<<endl;
    // for(size_t i=0;i!=sz;++i)
    //     s[i]='c';
    
    // vector<int> v;
    // int n;
    // cin >> n;
    // int temp;
    // for (int i = 0; i < n; ++i) {
    //     cin >> temp;
    //     v.push_back(temp);
    // }
    // sort(rbegin(v), rend(v));
    // cout << v[0];
    // int sz = size(v);
    // for (int i = 1; i < sz; ++i)
    //     cout << " " << v[i];
}