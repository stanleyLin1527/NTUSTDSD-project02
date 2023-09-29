#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "Product.h"
using namespace std;

#define not_care '-'

void read_PLA_file(ifstream &in_PLA_file, int &var_amount, int &pro_amount, string &out_label,
                   vector<string> &var_labels, vector<Product> &products);

void find_minterm_recursion(int &var_amount, int &ordinal, string &minterm, vector<Product> &list);

vector<Product> make_minterm_list(int &var_amount);

// This function is to extract all possible minterms from read-in pla file
vector<Product> products_to_minterms(int &var_amount, vector<Product> &products, vector<Product> &minterms_list);

map<int, vector<Product>> sort_by_1_amount(int &var_amount, vector<Product> &minterms);

// Merge map by recursion
void merging_map(map<int, vector<Product>> &unmerged, int max_of_1, bool do_again, int &digit_amount, vector<Product> &PI);

vector<Product> QA_algorithm(int &var_amount, vector<Product> &minterms);

void Patrick_method(vector<Product> &PI, vector<Product> &minterms, vector<Product> &EPI);

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
    // for (Product x : minterms)
    // {
    //     cout << x.literals << " " << x.type;
    //     for (int y : x.merged_minterms)
    //         cout << " " << y;
    //     cout << endl;
    // }
    // return 0;

    vector<Product> PI = QA_algorithm(var_amount, minterms);
    // return 0;

    // Unit test to check all PIs are correct
    // for (const Product &x : PI)
    // {
    //     cout << x.literals;
    //     for (int y : x.merged_minterms)
    //         cout << " " << y;
    //     cout << endl;
    // }
    // return 0;

    // Remove not-care minterms from vector "minterms" because we don'y need those minterms anymore
    for (int i = 0; i < minterms.size(); i++)
    {
        if (minterms[i].type == not_care)
        {
            minterms.erase(minterms.begin() + i);
            i = -1;
        }
    }
    vector<Product> EPI;
    // FIXME: Find minterms which are wrapped by only one PI and push them into EPI first, and then remove them from PI, need check!
    vector<vector<bool>> first_patrick_form(minterms.size(), vector<bool>(PI.size(), false));
    for (int product_index = 0; product_index < PI.size(); product_index++)
    {
        for (int merged_minterm : PI[product_index].merged_minterms)
        {
            for (Product minterm : minterms)
            {
                for (int minterm_index : minterm.merged_minterms)
                {
                    if (merged_minterm == minterm_index)
                    {
                        first_patrick_form[minterm_index][merged_minterm] = true;
                    }
                }
            }
        }
    }
    for (int minterm_index; minterms.size(); minterm_index++)
    {
        int own_amount = 0;
        for (bool x : first_patrick_form[minterm_index])
        {
            if (x)
                own_amount++;
        }
        if (own_amount == 1)
        {
            for (int target_PI_idx = 0; target_PI_idx < PI.size(); target_PI_idx++)
            {
                if (first_patrick_form[minterm_index][target_PI_idx])
                {
                    EPI.push_back(PI[target_PI_idx]);
                    PI.erase(PI.begin() + target_PI_idx);
                    break;
                }
            }
        }
    }

    // FIXME: Remove minterms that are already wrapped up from vector "minterms", need check!
    for (int i = 0; i < minterms.size(); i++)
    {
        for (int minterm_idx : minterms[i].merged_minterms)
        {
            for (Product x : EPI)
            {
                for (int confirm_minterm_idx : x.merged_minterms)
                {
                    if (minterm_idx == confirm_minterm_idx)
                    {
                        minterms.erase(minterms.begin() + i);
                        i = -1;
                        goto check_next_minterm;
                    }
                }
            }
        }
    check_next_minterm:
        continue;
    }

    Patrick_method(PI, minterms, EPI); // Find required EPI from the remaining PIs

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
        tmp.merged_minterms.push_back(ordinal);
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

// This function is to extract all possible minterms from read-in pla file
vector<Product> products_to_minterms(int &var_amount, vector<Product> &products, vector<Product> &minterms_list)
{
    vector<Product> minterms; // A vector stores all necessary component minterms

    // Use list to extract the necessary minterms from all products that are read from pla
    for (Product check : minterms_list)
    {
        for (Product be_checked : products)
        {
            bool is_component = true;
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
                    if (check.merged_minterms == already_in.merged_minterms)
                    {
                        is_new = false;
                        break;
                    }
                }
                if (is_new)
                {
                    Product tmp;
                    tmp.literals = check.literals;
                    tmp.merged_minterms = check.merged_minterms;
                    tmp.type = be_checked.type;
                    minterms.push_back(tmp);
                }
                break;
            }
        }
    }

    return minterms;
}

map<int, vector<Product>> sort_by_1_amount(int &max_of_1, vector<Product> &minterms)
{
    map<int, vector<Product>> sorted_map;
    for (int num_of_1 = 0; num_of_1 <= max_of_1; num_of_1++)
    {
        vector<Product> tmp;
        for (Product x : minterms)
        {
            int equal_1_num = 0;
            for (char digit : x.literals)
            {
                if (digit == '1')
                    equal_1_num++;
            }
            if (equal_1_num == num_of_1)
                tmp.push_back(x);
        }
        if (tmp.size() != 0)
            sorted_map[num_of_1] = tmp;
    }
    return sorted_map;
}

void merging_map(map<int, vector<Product>> &unmerged, int max_of_1, bool do_again, int &digit_amount, vector<Product> &PI)
{
    if (!do_again)
        return;

    bool have_changed = false;
    vector<Product> merged_result;
    for (int i = 0; i <= max_of_1; i++)
    {
        for (int j = i + 1; j <= max_of_1 + 1; j++)
        {
            for (Product &x : unmerged[i]) // Must be call-by-reference
            {
                for (Product &y : unmerged[j]) // Must be call-by-reference
                {
                    vector<int> differ_digit_index;
                    for (int i = 0; i < digit_amount; i++)
                    {
                        if (x.literals[i] != y.literals[i])
                            differ_digit_index.push_back(i);
                    }
                    if (differ_digit_index.size() == 1) // Going to merge
                    {
                        bool is_new = true;
                        have_changed = true;
                        x.has_checked = true;
                        y.has_checked = true;
                        Product merged;
                        merged.literals = x.literals;
                        merged.literals[differ_digit_index[0]] = not_care;
                        // Ignore duplicate Product when merging
                        for (Product already_in : merged_result)
                        {
                            if (merged.literals == already_in.literals)
                            {
                                is_new = false;
                                break;
                            }
                        }
                        if (is_new)
                        {
                            // Record this product is composed of what minterms
                            for (int x_merged_minterms : x.merged_minterms)
                                merged.merged_minterms.push_back(x_merged_minterms);
                            for (int y_merged_minterms : y.merged_minterms)
                                merged.merged_minterms.push_back(y_merged_minterms);
                            merged_result.push_back(merged);
                        }
                    }
                }
            }
        }
    }

    // If there are some minterms that don't merge with others, then those minterms are PI.
    for (const auto &pair : unmerged)
    {
        for (Product x : pair.second)
        {
            if (!x.has_checked)
                PI.push_back(x);
        }
    }

    unmerged.clear();
    unmerged = sort_by_1_amount(max_of_1, merged_result);

    merging_map(unmerged, max_of_1 - 1, have_changed, digit_amount, PI);
}

vector<Product> QA_algorithm(int &var_amount, vector<Product> &minterms)
{
    vector<Product> PI;
    // Init map of minterms according to the num of 1
    map<int, vector<Product>> init_map = sort_by_1_amount(var_amount, minterms);
    // Unit test to verify init map is correct
    // for (const auto &pair : init_map)
    // {
    //     cout << pair.first << endl;
    //     for (Product x : pair.second)
    //     {
    //         for (int merged_minterm : x.merged_minterms)
    //             cout << merged_minterm << " ";
    //         cout << x.literals << endl;
    //     }
    // }
    // goto check_sort_function;

    merging_map(init_map, var_amount - 1, true, var_amount, PI);

    for (const auto &pair : init_map /*Has become can't-merge-anymore map*/)
    {
        for (Product merged_product : pair.second)
            PI.push_back(merged_product);
    }

    // check_sort_function:
    return PI;
}

// TODO: Complete it
void Patrick_method(vector<Product> &PI, vector<Product> &minterms, vector<Product> &EPI)
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
