// space limit exceeded

#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std;

#define MB (1024ULL*1024ULL)

int main(){
    //constexpr size_t sz = MB;
    vector<int> vvv;
    vvv[234314143]=23;


    vector<int> v;
    int n;
    cin >> n;
    int temp;
    for (int i = 0; i < n; ++i) {
        cin >> temp;
        v.push_back(temp);
    }
    sort(rbegin(v), rend(v));
    cout << v[0];
    int sz = size(v);
    for (int i = 1; i < sz; ++i)
        cout << " " << v[i];
}
