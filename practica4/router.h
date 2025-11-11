#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <string>

using namespace std;
class router
{
private:
    char ID;
    vector<string> vecinos;
    vector<int> costos;

public:
    router();
};

#endif // ROUTER_H
