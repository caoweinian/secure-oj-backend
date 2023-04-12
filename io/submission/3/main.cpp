// INVALID OPERATION

#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
using namespace std;
int main() {
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
    chroot(".");
}