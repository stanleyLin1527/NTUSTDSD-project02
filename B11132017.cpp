#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include "Product.h"
using namespace std;

#define char_zero '0'
#define not_care '-'

void read_PLA_file(ifstream &in_PLA_file, int &var_amount, int &pro_amount, vector<string> &var_labels, vector<Product> &products);

void write_PLA_file(ofstream& out_PLA_file, vector<string> &var_labels, vector<Product> &new_products);

int main()
{
    int var_amount;
    int pro_amount = 0;
    vector<string> var_labels;
    vector<Product> products;
    ifstream in_PLA_file("Pro_1.pla");
    ofstream out_PLA_file("Pro_1_out.pla");

    read_PLA_file(in_PLA_file, var_amount, pro_amount, var_labels, products);
    in_PLA_file.close();

    // Unit test to check the pla. file is read correctly
    // cout << var_amount << " " << pro_amount << endl;
    // for (string label : var_labels)
    //     cout << label << " ";
    // cout << endl;
    // for (Product x : products)
    //     cout << x << endl;
    // return 0;

    return 0;
}

void read_PLA_file(ifstream &in_PLA_file, int &var_amount, int &pro_amount, vector<string> &var_labels, vector<Product> &products)
{
    string input;
    stringstream line;
    while (getline(in_PLA_file, input))
    {
        line.clear(); // To clear string stream
        line.str(""); // To clear string stream
        if (input == ".e")
            break;

        string command;
        line << input;
        line >> command;
        if (command == ".i")
            line >> var_amount;
        else if (command == ".ilb")
        {
            string label;
            while (line >> label)
                var_labels.push_back(label);
        }
        else if (command == ".p")
        {
            line >> pro_amount;
            // Read products
            for (int i = 0; i < pro_amount; i++)
            {
                line.clear(); // To clear string stream
                line.str(""); // To clear string stream
                string product;
                Product tmp; // A product which would be push into vector later
                char type;
                getline(in_PLA_file, input);
                line << input;
                line >> product;

                for (int i = 0; i < var_amount; i++)
                {
                    if (product[i] == not_care)
                        tmp.literals.push_back(-1);
                    else
                        tmp.literals.push_back(product[i] - char_zero);
                }
                line >> type;
                if (type == not_care)
                    tmp.type = -1;
                else
                    tmp.type = type - char_zero;
                products.push_back(tmp); // Push product
            }
        }
        else
        {
            // If there's no .p command, then when the first char is not '.',
            // that line is the beginning of products and start counting
            if (command[0] != '.')
            {
                Product tmp; // A product which would be push into vector later
                char type;
                for (int i = 0; i < var_amount; i++)
                {
                    if (command[i] == not_care)
                        tmp.literals.push_back(-1);
                    else
                        tmp.literals.push_back(command[i] - char_zero);
                }
                line >> type;
                if (type == not_care)
                    tmp.type = -1;
                else
                    tmp.type = type - char_zero;
                products.push_back(tmp); // Push product
                pro_amount++;
            }
            else
                continue; // Discard useless line of operation
        }
    }
}