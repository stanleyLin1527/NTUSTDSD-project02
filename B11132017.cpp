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

// According the amount of variables to find all minterms' form
void find_minterm_recursion(int &var_amount, int &ordinal, string &minterm, vector<Product> &list);
// Make a list of all minterms in order to compare with the read-in products easily
vector<Product> make_minterm_list(int &var_amount);
// This function is to extract all possible minterms from read-in pla file
vector<Product> products_to_minterms(int &var_amount, vector<Product> &products, vector<Product> &minterms_list);
map<int, vector<Product>> sort_by_1_amount(int &var_amount, vector<Product> &minterms);
// Merge map by recursion
void merging_map(map<int, vector<Product>> &unmerged, int max_of_1, bool do_again, int &digit_amount, vector<Product> &PI);
vector<Product> QA_algorithm(int &var_amount, vector<Product> &minterms);
// Create a 2D boolean chart according to "unwrapped minterms" and PI
vector<vector<bool>> create_patrick_chart(vector<Product> &minterms, vector<Product> &PI);
// Do polynomial multiplication by recursion
void do_poly_multi(vector<vector<string>> &un_simplify);
// Get EPI from the remaining PI
void Patrick_method(vector<Product> &PI, vector<Product> &minterms, vector<Product> &EPI);
void write_PLA_file(ofstream &out_PLA_file, string &out_label, vector<string> &var_labels, vector<Product> &EPI);

int main(int argc, char *argv[])
{
    int var_amount;
    int pro_amount = 0;
    string out_label;
    vector<string> var_labels;
    vector<Product> products;
    ifstream in_PLA_file(argv[1]);
    ofstream out_PLA_file(argv[2]);

    read_PLA_file(in_PLA_file, var_amount, pro_amount, out_label, var_labels, products);
    in_PLA_file.close();

    vector<Product> list = make_minterm_list(var_amount);
    vector<Product> minterms = products_to_minterms(var_amount, products, list);
    vector<Product> PI = QA_algorithm(var_amount, minterms); // Find PI from necessary minterms
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
    // Find minterms which are wrapped by only one PI and push them into EPI first, and then remove them from PI
    vector<vector<bool>> chart_before_patrick = create_patrick_chart(minterms, PI);
    for (int i = 0; i < chart_before_patrick.size(); i++) // Push into EPI
    {
        int have_amount = 0;
        for (int j = 0; j < chart_before_patrick[i].size(); j++)
        {
            if (chart_before_patrick[i][j])
                have_amount++;
        }
        if (have_amount == 1)
        {
            for (int j = 0; j < chart_before_patrick[i].size(); j++)
            {
                if (chart_before_patrick[i][j])
                {
                    bool is_new = true;
                    for (Product already_in : EPI) // Push if it is new to EPI
                    {
                        if (PI[j].literals == already_in.literals)
                        {
                            is_new = false;
                            break;
                        }
                    }
                    if (is_new)
                        EPI.push_back(PI[j]);
                }
            }
        }
    }
    for (Product x : EPI) // Remove them from PI
    {
        for (int i = 0; i < PI.size(); i++)
        {
            if (PI[i].literals == x.literals)
            {
                PI.erase(PI.begin() + i);
                break;
            }
        }
    }
    // Remove minterms that are already wrapped up from vector "minterms"
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
    merging_map(init_map, var_amount - 1, true, var_amount, PI);
    for (const auto &pair : init_map /*Has become can't-merge-anymore map*/)
    {
        for (Product merged_product : pair.second)
            PI.push_back(merged_product);
    }
    return PI;
}

vector<vector<bool>> create_patrick_chart(vector<Product> &minterms, vector<Product> &PI)
{
    vector<vector<bool>> chart(minterms.size(), vector<bool>(PI.size(), false));
    for (int which_minterm = 0; which_minterm < minterms.size(); which_minterm++) // Traverse all necessary minterms
    {
        for (int which_product = 0; which_product < PI.size(); which_product++) // Traverse all PI
        {
            for (int minterm_idx : minterms[which_minterm].merged_minterms)
            {
                for (int merged_idx : PI[which_product].merged_minterms)
                {
                    if (merged_idx == minterm_idx)
                        chart[which_minterm][which_product] = true;
                }
            }
        }
    }
    return chart;
}

void do_poly_multi(vector<vector<string>> &un_simplify)
{
    if (un_simplify.size() == 1)
        return;

    vector<string> new_product;
    for (int i = 0; i < un_simplify[0].size(); i++)
    {
        for (int j = 0; j < un_simplify[1].size(); j++)
        {
            if (un_simplify[0][i].find(un_simplify[1][j]) == string::npos)
            {
                string tmp = un_simplify[0][i] + " " + un_simplify[1][j];
                new_product.push_back(tmp);
            }
            else
                new_product.push_back(un_simplify[0][i]);
        }
    }
    un_simplify[0] = new_product;
    un_simplify.erase(un_simplify.begin() + 1);
    do_poly_multi(un_simplify);
}

void Patrick_method(vector<Product> &PI, vector<Product> &minterms, vector<Product> &EPI)
{
    if (minterms.size() == 0) // If there's no minterms to extract from PI, then don't do patrick method
        return;

    vector<vector<bool>> chart = create_patrick_chart(minterms, PI);
    vector<vector<string>> poly_multi_result;
    for (int i = 0; i < chart.size(); i++) // Init poly_multi_result
    {
        vector<string> tmp;
        for (int j = 0; j < chart[i].size(); j++)
        {
            if (chart[i][j])
                tmp.push_back(PI[j].literals);
        }
        poly_multi_result.push_back(tmp);
    }
    do_poly_multi(poly_multi_result);
    int least_term_amount = INT32_MAX;    // init this with the MAX of int
    for (string x : poly_multi_result[0]) // Find the amount of the least term
    {
        int space_amount = 0;
        for (char y : x)
        {
            if (y == ' ')
                space_amount++;
        }
        if (space_amount + 1 < least_term_amount)
            least_term_amount = space_amount + 1;
    }
    for (string x : poly_multi_result[0]) // Find one result whose term num = the least amount, and push it into EPI
    {
        int space_amount = 0;
        for (char y : x)
        {
            if (y == ' ')
                space_amount++;
        }
        if (space_amount + 1 == least_term_amount)
        {
            stringstream product;
            string tmp;
            product << x;
            while (product >> tmp)
            {
                Product target_PI;
                target_PI.literals = tmp;
                EPI.push_back(target_PI);
            }
            break;
        }
    }
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
    out_PLA_file << ".i " << var_labels.size() << endl;
    out_PLA_file << ".o 1" << endl;
    out_PLA_file << ".ilb";
    for (string x : var_labels)
        out_PLA_file << " " << x;
    out_PLA_file << endl;
    out_PLA_file << ".ob " << out_label << endl;
    out_PLA_file << ".p " << EPI.size() << endl;
    for (Product x : EPI)
        out_PLA_file << x.literals << " 1" << endl;
    out_PLA_file << ".e" << endl;
}
