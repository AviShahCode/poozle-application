#include <iostream>
#include <regex>
#include <string>

using namespace std;

int main() {
    int t;
    cin >> t;
    cin.ignore();

    int answer = 0;
    bool do_add = true, do_mul = true;

    regex pattern(R"((do_mul\(\)|do_add\(\)|don't_mul\(\)|don't_add\(\)|add\(\d+,\d+\)|mul\(\d+,\d+\)))");

    while(t--) {
        string s;
        getline(cin, s);

        auto words_begin = sregex_iterator(s.begin(), s.end(), pattern);
        auto words_end = sregex_iterator();

        for(sregex_iterator i = words_begin; i != words_end; ++i) {
            string item = i->str();

            if(item == "do_mul()") {
                do_mul = true;
            }
            else if(item == "do_add()") {
                do_add = true;
            }
            else if(item == "don't_mul()") {
                do_mul = false;
            }
            else if(item == "don't_add()") {
                do_add = false;
            }
            else if(item.substr(0, 3) == "add" && do_add) {
                string nums = item.substr(4, item.length() - 5); // remove add( and )
                size_t comma = nums.find(',');
                int a = stoi(nums.substr(0, comma));
                int b = stoi(nums.substr(comma + 1));
                answer += a + b;
            }
            else if(item.substr(0, 3) == "mul" && do_mul) {
                string nums = item.substr(4, item.length() - 5); // remove mul( and )
                size_t comma = nums.find(',');
                int a = stoi(nums.substr(0, comma));
                int b = stoi(nums.substr(comma + 1));
                answer += a * b;
            }
        }
    }

    cout << answer << endl;
    return 0;
}