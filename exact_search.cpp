#include <algorithm>
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

// find the first occurrence of substr in str within str[left, right], both inclusive
int find_substr(const string &str, const string &pattern, const string &pattern_flip,
                const unsigned &left, const unsigned &right) {
    if (left > right) return -1;

    // load values before hand
    const unsigned substr_size = pattern.size(), str_size = str.size();
    if (str_size == 0 or substr_size == 0) return -1;

    // optimization, load first character
    const char first_sub_char = pattern[0];
    const char first_sub_char_flip = pattern_flip[0];

    // find the substr starting from i in str
    // check pattern and pattern_flip (for case insensitive search)
    for (unsigned i = left; i <= right; i++) {
        // use optimization, check first character match before initializing loops etc
        if (str[i] != first_sub_char && str[i] != first_sub_char_flip) continue;

        // if not a match, go to next i
        bool match = true;
        // check proceeding characters
        for (unsigned j = 1; j < substr_size; j++) {
            // check match, if case_insensitive, check lower and upper, else both are same
            if (i + j < str_size && str[i + j] != pattern[j] && str[i + j] != pattern_flip[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return -1;
}

vector<unsigned> exact_search(const string &str, const string &pattern, const string &pattern_flip,
                              const unsigned left, const unsigned right) {
    if (left > right or str.empty() or pattern.empty()) return {};

    // since there is a lot of overhead to make new threads,
    // use single thread for small strings
    if (right - left >= 1'000'000) {
        const unsigned mid = (left + right) / 2;
        // search left and right halves parellely using only software threads
        auto future_left = async(launch::deferred, exact_search, ref(str),
                                 ref(pattern), ref(pattern_flip), left, mid);
        auto future_right = async(launch::deferred, exact_search, ref(str),
                                  ref(pattern), ref(pattern_flip), mid + 1, right);
        vector<unsigned> left_indices = future_left.get();
        vector<unsigned> right_indices = future_right.get();
        left_indices.insert(left_indices.end(), right_indices.begin(), right_indices.end());
        return left_indices;
    }

    vector<unsigned> indices;
    // reserve a few indices
    indices.reserve(64);
    while (true) {
        int ind;
        // use str.find for in built find, is faster, but doesn't support case_insensitive
        if (!indices.empty())
            // ind = str.find(pattern, indices.back() + 1);
            ind = find_substr(str, pattern, pattern_flip, indices.back() + 1, right);
        else
            // ind = str.find(pattern, left);
            ind = find_substr(str, pattern, pattern_flip, left, right);
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
    string pattern = argv[2];
    string pattern_flip;

    // if case_insensitive, have 2 patterns, with lower and upper only respectively
    // search for pattern in an OR fashion, either lower or upper character should be present
    if (case_insensitive) {
        transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);
        transform(pattern.begin(), pattern.end(), pattern_flip.begin(), ::toupper);
    } else {
        pattern_flip = pattern;
    }

    // time the function
    const auto start = chrono::steady_clock::now();
    const vector<unsigned> indices = exact_search(str, pattern, pattern_flip, 0, str.size() - 1);
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
