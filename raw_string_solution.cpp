#include <iostream>

using namespace std;

int find_op(const string &str, const bool add, unsigned int &idx) {
    // skip "add(" and "mul("
    idx += 4;
    // check the start character is digit
    if (!isdigit(str[idx])) return 0;
    int nums[] = {0, 0};
    int num = 0;
    while (idx < str.size()) {
        // append digit to current number
        if (isdigit(str[idx])) nums[num] = nums[num] * 10 + str[idx] - '0';
        // switch to next number
        else if (str[idx] == ',') {
            num++;
            // only 2 numbers allowed
            if (num >= 2) return 0;
            // check the next character is digit
            if (idx + 1 < str.size() && !isdigit(str[idx + 1])) return 0;
        }
        // if exactly 2 valid numbers in function call
        else if (str[idx] == ')' and num == 1) {
            // move past ')', taken care of by next i++
            // idx++;
            return add? nums[0] + nums[1] : nums[0] * nums[1];
        }
        else {
            return 0;
        }
        idx++;
    }
    return 0;
}

int process(const string &s) {
    // state of do's does not change
    static bool do_add = true, do_mul = true;
    int result = 0;

    for (unsigned int i = 0; i < s.size(); i++) {
        // check upcoming substring
        if (s.substr(i, 8) == "do_add()") {
            do_add = true;
            // skip this substring
            i += 7;
        }
        else if (s.substr(i, 11) == "don't_add()") {
            do_add = false;
            i += 10;
        }
        if (s.substr(i, 8) == "do_mul()") {
            do_mul = true;
            i += 7;
        }
        else if (s.substr(i, 11) == "don't_mul()") {
            do_mul = false;
            i += 10;
        }
        // do operation if do is enabled
        else if (s.substr(i, 4) == "add(" && do_add) {
            result += find_op(s, true, i);
        }
        else if (s.substr(i, 4) == "mul(" && do_mul) {
            result += find_op(s, false, i);
        }
    }
    return result;
}


int main() {
    unsigned int t;
    cin >> t;
    cin.ignore();
    int result = 0;
    // process each line
    while (t--) {
        string s;
        getline(cin, s);
        result += process(s);
    }
    cout << result << endl;
    return 0;
}
