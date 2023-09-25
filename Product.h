#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

class Product
{
public:
    string literals;
    vector<int> added_minterms;
    char type;
    bool has_checked = false;
};