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

// find the first occurence of substr in str within str[left, right], both inclusive
int find_substr(const string &str, const string &substr, const unsigned &left, const unsigned &right) {
    if (left > right) return -1;
    // load values before hand
    const unsigned substr_size = substr.size(), str_size = str.size();
    if (str_size == 0 or substr_size == 0) return -1;
    const char first_sub_char = substr[0];
    // find the substr starting from i in str
    for (unsigned i = left; i <= right; i++) {
        if (str[i] != first_sub_char) continue;
        bool match = true;
        // check proceeding characters
        for (unsigned j = 1; j < substr_size; j++) {
            // if not a match, go to next i
            if (i + j < str_size && str[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return -1;
}

vector<unsigned> exact_search(const string &str, const string &pattern, const unsigned left, const unsigned right) {
    if (left > right or str.empty() or pattern.empty()) return {};
    // edge case, left should never equal right anyway
    if (left == right && pattern.size() == 1 and str[left] == pattern[0]) return {left};

    // since there is a lot of overhead to make new threads,
    // use single thread for small strings
    if (right - left >= 1'000'000) {
        const unsigned mid = (left + right) / 2;
        // search left and right halves
        auto future_left = async(launch::async, exact_search, ref(str), ref(pattern), left, mid);
        auto future_right = async(launch::async, exact_search, ref(str), ref(pattern), mid + 1, right);
        vector<unsigned> left_indices = future_left.get();
        vector<unsigned> right_indices = future_right.get();
        left_indices.insert(left_indices.end(), right_indices.begin(), right_indices.end());
        return left_indices;
    }

    vector<unsigned> indices;
    while (true) {
        int ind;
        // use str.find for in built find, is faster
        if (!indices.empty())
            // ind = str.find(pattern, indices.back() + 1);
            ind = find_substr(str, pattern, indices.back() + 1, right);
        else
            // ind = str.find(pattern, left);
            ind = find_substr(str, pattern, left, right);
        if (ind == -1 or ind > right) break;
        indices.push_back(ind);
    }
    return indices;
}

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file> <pattern>\n", argv[0]);
        return 1;
    }

    const string str = read_file(argv[1]);
    const string pattern = argv[2];

    // time the function
    const auto start = chrono::steady_clock::now();
    const vector<unsigned> indices = exact_search(str, pattern, 0, str.size() - 1);
    const auto end = chrono::steady_clock::now();

    printf("indices: { ");
    for (const unsigned &i: indices) {
        printf("%d ", i);
    }
    printf("}\n");

    printf("count: %lu\n", indices.size());
    printf("time: %lu ms\n", chrono::duration_cast<chrono::milliseconds>(end - start).count());
    return 0;
}
