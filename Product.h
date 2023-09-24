#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

class Product
{
public:
    vector<int> literals;
    int type;

    friend ostream &operator<<(ostream &out, Product &product)
    {
        for (int x : product.literals){
            if (x == -1)
                out << "-";
            else
                out << x;
        }
        out << " " << product.type;
        return out;
    }

    friend ofstream &operator<<(ofstream &out, Product &product)
    {
        for (int x : product.literals)
        {
            if (x == -1)
                out << "-";
            else
                out << x;
            out << " " << product.type;
        }
        return out;
    }
};