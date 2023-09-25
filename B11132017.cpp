#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include "Product.h"
using namespace std;

#define not_care '-'

void read_PLA_file(ifstream &in_PLA_file, int &var_amount, int &pro_amount, string &out_label,
                   vector<string> &var_labels, vector<Product> &products);

vector<Product> make_minterm_list(int &var_amount);

vector<Product> products_to_minterms(vector<Product> &products, vector<Product> &minterms_list);

vector<Product> QA_algorithm(vector<Product> &minterms);

void Patrick_method(vector<Product> &PI);

void write_PLA_file(ofstream &out_PLA_file, string &out_label, vector<string> &var_labels, vector<Product> &EPI);

int main()
{
    int var_amount;
    int pro_amount = 0;
    string out_label;
    vector<string> var_labels;
    vector<Product> products;
    ifstream in_PLA_file("Pro_1.pla");
    ofstream out_PLA_file("Pro_1_out.pla");

    read_PLA_file(in_PLA_file, var_amount, pro_amount, out_label, var_labels, products);
    in_PLA_file.close();

    // Unit test to check the pla. file is read correctly
    cout << var_amount << " " << pro_amount << endl;
    for (string label : var_labels)
        cout << label << " ";
    cout << endl;
    cout << out_label << endl;
    for (Product x : products)
        cout << x.literals << " " << x.type << endl;
    return 0;

    vector<Product> list = make_minterm_list(var_amount);
    vector<Product> minterms = products_to_minterms(products, list);
    // TODO: Unit test to check all products are convert into minterms correctly

    vector<Product> PI = QA_algorithm(minterms);
    // TODO: Unit test to check all PIs are correct

    vector<Product> EPI;
    // TODO: Find minterms which are wrapped by only one PI and push them into EPI first, and then remove them from PI

    Patrick_method(PI); // Find required EPI from the remaining PIs

    write_PLA_file(out_PLA_file, out_label, var_labels, EPI);
    out_PLA_file.close();

    return 0;
}

void read_PLA_file(ifstream &in_PLA_file, int &var_amount, int &pro_amount, string &out_label,
                   vector<string> &var_labels, vector<Product> &products)
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
                Product tmp;  // A product which would be push into vector later
                getline(in_PLA_file, input);
                line << input;
                line >> tmp.literals;    // Fill literals
                line >> tmp.type;        // Fill type
                products.push_back(tmp); // Push product
            }
        }
        else if (command == ".ob")
            line >> out_label;
        else
        {
            // If there's no .p command, then when the first char is not '.',
            // that line is the beginning of products and start counting
            if (command[0] != '.')
            {
                Product tmp;             // A product which would be push into vector later
                tmp.literals = command;  // Fill literals
                line >> tmp.type;        // Fill type
                products.push_back(tmp); // Push product
                pro_amount++;
            }
            else
                continue; // Discard useless line of operation
        }
    }
}

// TODO: Complete it
vector<Product> make_minterm_list(int &var_amount)
{
    vector<Product> list;

    return list;
}

// TODO: Complete it
vector<Product> products_to_minterms(vector<Product> &products, vector<Product> &minterms_list)
{
    vector<Product> minterms;

    return minterms;
}

// TODO: Complete it
vector<Product> QA_algorithm(vector<Product> &minterms)
{
    vector<Product> PI;

    return PI;
}

// TODO: Complete it
void Patrick_method(vector<Product> &PI)
{
}

// TODO: Complete it
void write_PLA_file(ofstream &out_PLA_file, string &out_label, vector<string> &var_labels, vector<Product> &EPI)
{
    cout << ".e";
}