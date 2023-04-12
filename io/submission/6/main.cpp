#include <iostream>
#include <vector>
#include <unistd.h>
using namespace std;

int main(){
    vector<int> v;

    while(true){
        v.push_back(3);
        v.pop_back();
    }
}