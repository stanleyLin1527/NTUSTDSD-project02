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

void find_minterm_recursion(int &var_amount, int &ordinal, string &minterm, vector<Product> &list);

vector<Product> make_minterm_list(int &var_amount);

vector<Product> products_to_minterms(int &var_amount, vector<Product> &products, vector<Product> &minterms_list);

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
    // cout << var_amount << " " << pro_amount << endl;
    // for (string label : var_labels)
    //     cout << label << " ";
    // cout << endl;
    // cout << out_label << endl;
    // for (Product x : products)
    //     cout << x.literals << " " << x.type << endl;
    // return 0;

    vector<Product> list = make_minterm_list(var_amount);
    // Unit test to check the list of minterms is correct
    // for (Product x : list)
    //     cout << x.literals << endl;
    // return 0;

    vector<Product> minterms = products_to_minterms(var_amount, products, list);
    // Unit test to check all products are convert into necessary minterms correctly
    for (Product x : minterms)
    {
        cout << x.literals << " " << x.type;
        for (int y : x.added_minterms)
            cout << " " << y;
        cout << endl;
    }
    return 0;

    vector<Product> PI = QA_algorithm(minterms);
    // Unit test to check all PIs are correct
    for (Product x : PI)
    {
        cout << x.literals << " " << x.type;
        for (int y : x.added_minterms)
            cout << " " << y;
        cout << endl;
    }
    return 0;

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

void find_minterm_recursion(int &var_amount, int &ordinal, string &minterm, vector<Product> &list)
{
    if (minterm.size() == var_amount)
    {
        Product tmp;
        tmp.literals = minterm;
        tmp.added_minterms.push_back(ordinal);
        list.push_back(tmp);
        ordinal++;
        minterm.pop_back();
        return;
    }

    minterm.push_back('0');
    find_minterm_recursion(var_amount, ordinal, minterm, list);
    minterm.push_back('1');
    find_minterm_recursion(var_amount, ordinal, minterm, list);
    minterm.pop_back();
    return;
}

vector<Product> make_minterm_list(int &var_amount)
{
    int start_num = 0;
    vector<Product> list;
    string minterm;

    find_minterm_recursion(var_amount, start_num, minterm, list);

    return list;
}

// FIXME: Complete it, each minterm can appear only once, need to check
vector<Product> products_to_minterms(int &var_amount, vector<Product> &products, vector<Product> &minterms_list)
{
    vector<Product> minterms; // A vector stores all necessary component minterms 

    // Use list to extract the necessary minterms from all products that are read from pla
    for (Product check : minterms_list)
    {
        bool is_component = true;
        for (Product be_checked : products)
        {
            for (int i = 0; i < var_amount; i++)
            {
                if (be_checked.literals[i] != not_care)
                {
                    if (be_checked.literals[i] != check.literals[i])
                    {
                        is_component = false;
                        break;
                    }
                }
            }

            // If current minterm is the component of that product, it must be new to the previous minterms to push into vector
            if (is_component)
            {
                bool is_new = true;
                for (Product already_in : minterms)
                {
                    if (check.added_minterms == already_in.added_minterms)
                    {
                        is_new = false;
                        break;
                    }
                }
                if (is_new)
                {
                    Product tmp;
                    tmp.literals = check.literals;
                    tmp.added_minterms = check.added_minterms;
                    tmp.type = be_checked.type;
                    minterms.push_back(tmp);
                }
                break;
            }
        }
    }

    return minterms;
}

// TODO: Complete it: mapping minterms according to the num of 1, and simplify them until can't do the simplification anymore
vector<Product> QA_algorithm(vector<Product> &minterms)
{
    vector<Product> PI;

    return PI;
}

// TODO: Complete it
void Patrick_method(vector<Product> &PI)
{
}

void write_PLA_file(ofstream &out_PLA_file, string &out_label, vector<string> &var_labels, vector<Product> &EPI)
{
    cout << "Total number of terms: " << EPI.size() << endl;
    int literals_num = 0;
    for (Product x : EPI)
    {
        for (int i = 0; i < x.literals.size(); i++)
        {
            if (x.literals[i] != not_care)
                literals_num++;
        }
    }
    cout << "Total number of literals: " << literals_num << endl;

    out_PLA_file << ".p " << var_labels.size() << endl;
    out_PLA_file << ".o 1" << endl;
    out_PLA_file << ".ilb";
    for (string x : var_labels)
        out_PLA_file << " " << x;
    out_PLA_file << endl;
    out_PLA_file << ".ob " << out_label << endl;
    out_PLA_file << ".p " << EPI.size() << endl;
    for (Product x : EPI)
        out_PLA_file << x.literals << " " << x.type << endl;
    out_PLA_file << ".e";
}
