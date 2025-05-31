#include <cstring>
#include <future>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// read all contents of file
string read_file(const string &filename) {
    ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// flip lower to upper and vice versa
char flip_case(const char &c) {
    if (isupper(c)) return tolower(c);
    return toupper(c);
}

// find the first occurence of substr in str within str[left, right], both inclusive
int find_substr(const string &str, const string &substr, const unsigned &left, const unsigned &right,
                const bool case_insensitive = false) {
    if (left > right) return -1;

    // load values before hand
    const unsigned substr_size = substr.size(), str_size = str.size();
    if (str_size == 0 or substr_size == 0) return -1;

    // if case_insensitive, load lower and upper, else both are same
    const char first_sub_char1 = substr[0];
    const char first_sub_char2 = case_insensitive ? flip_case(first_sub_char1) : first_sub_char1;

    // find the substr starting from i in str
    for (unsigned i = left; i <= right; i++) {
        // optimization, check upper and lower case match
        if (str[i] != first_sub_char1 && str[i] != first_sub_char2) continue;

        // if not a match, go to next i
        bool match = true;
        // check proceeding characters
        for (unsigned j = 1; j < substr_size; j++) {
            // if case_insensitive, load lower and upper, else both are same
            const char sub_char1 = substr[j];
            const char sub_char2 = case_insensitive ? flip_case(sub_char1) : sub_char1;

            // check upper and lower case match
            if (i + j < str_size && str[i + j] != sub_char1 && str[i + j] != sub_char2) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return -1;
}

vector<unsigned> exact_search(const string &str, const string &pattern, const unsigned left, const unsigned right,
                              bool case_insensitive = false) {
    if (left > right or str.empty() or pattern.empty()) return {};
    // edge case, left should never equal right anyway
    if (left == right && pattern.size() == 1 and str[left] == pattern[0]) return {left};

    // since there is a lot of overhead to make new threads,
    // use single thread for small strings
    if (right - left >= 1'000'000) {
        const unsigned mid = (left + right) / 2;
        // search left and right halves
        auto future_left = async(launch::async, exact_search, ref(str), ref(pattern), left, mid, case_insensitive);
        auto future_right = async(launch::async, exact_search, ref(str), ref(pattern), mid + 1, right,
                                  case_insensitive);
        vector<unsigned> left_indices = future_left.get();
        vector<unsigned> right_indices = future_right.get();
        left_indices.insert(left_indices.end(), right_indices.begin(), right_indices.end());
        return left_indices;
    }

    vector<unsigned> indices;
    while (true) {
        int ind;
        // use str.find for in built find, is faster, but doesn't support case_insensitive
        if (!indices.empty())
            // ind = str.find(pattern, indices.back() + 1);
            ind = find_substr(str, pattern, indices.back() + 1, right, case_insensitive);
        else
            // ind = str.find(pattern, left);
            ind = find_substr(str, pattern, left, right, case_insensitive);
        if (ind == -1 or ind > right) break;
        indices.push_back(ind);
    }
    return indices;
}

int main(const int argc, char *argv[]) {
    // help message
    // count (number of matches) is always displayed
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        fprintf(stderr, "Usage: %s <file> <pattern> [options]\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "-h: print this help message\n");
        fprintf(stderr, "-c: case insensitive search\n");
        fprintf(stderr, "-o: print output, matched pattern is highlighted\n");
        fprintf(stderr, "-i: print indices of matches\n");
        fprintf(stderr, "-t: print time taken for search operation\n");
        return 0;
    }
    if (argc < 3) {
        fprintf(stderr, "Incorrect usage, use: %s -h\n", argv[0]);
        return 1;
    }

    bool print_output = false, print_indices = false, print_time = false, case_insensitive = false;
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            print_output = true;
        } else if (strcmp(argv[i], "-i") == 0) {
            print_indices = true;
        } else if (strcmp(argv[i], "-t") == 0) {
            print_time = true;
        } else if (strcmp(argv[i], "-c") == 0) {
            case_insensitive = true;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    const string str = read_file(argv[1]);
    const string pattern = argv[2];

    // time the function
    const auto start = chrono::steady_clock::now();
    const vector<unsigned> indices = exact_search(str, pattern, 0, str.size() - 1, case_insensitive);
    const auto end = chrono::steady_clock::now();

    // highlight results in green
    if (print_output) {
        printf("output:\n");
        unsigned i = 0;
        for (const unsigned &ind: indices) {
            printf("%s", str.substr(i, ind - i).c_str());
            printf("\033[92m%s\033[0m", str.substr(ind, pattern.size()).c_str());
            i = ind + pattern.size();
        }
        if (i < str.size()) printf("%s", str.substr(i).c_str());
        printf("\n");
    }

    if (print_indices) {
        printf("indices: { ");
        for (const unsigned &i: indices) {
            printf("%d ", i);
        }
        printf("}\n");
    }

    printf("count: %lu\n", indices.size());

    if (print_time) {
        const auto time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        printf("time: %lu ms\n", time);
    }

    return 0;
}
