#include <algorithm>
#include <cstdio>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;


vector<int> random_vector(const unsigned size, const int min_value, const int max_value) {
    vector<int> result(size);
    for (size_t i = 0; i < size; i++) {
        result[i] = min_value + rand() % (max_value - min_value + 1);
    }
    return result;
}

void print_vector(const vector<int> &v) {
    for (const int i: v) {
        printf("%d ", i);
    }
    printf("\n");
}

// single threaded merge sort
void merge_sort_st(vector<int> &v, const unsigned left, const unsigned right) {
    if (left >= right) return;

    // recursively divide and sort subarray
    const unsigned mid = (left + right) / 2;
    merge_sort_st(v, left, mid);
    merge_sort_st(v, mid + 1, right);

    // merge subarrays
    unsigned i = left, j = mid + 1, k = 0;
    vector<int> temp(right - left + 1);
    while (i <= mid && j <= right) {
        if (v[i] <= v[j]) temp[k++] = v[i++];
        else temp[k++] = v[j++];
    }

    // fill remaining elements
    while (i <= mid) temp[k++] = v[i++];
    while (j <= right) temp[k++] = v[j++];

    // copy back to original array
    copy(temp.begin(), temp.end(), v.begin() + left);
}

// single threaded quick sort
void quick_sort_st(vector<int> &v, const unsigned left, const unsigned right) {
    if (left >= right) return;

    // pivot element
    const int pivot = v[right];

    // put all smaller elements to left side
    unsigned i = left;
    for (unsigned j = left; j < right; j++) {
        if (v[j] <= pivot) {
            swap(v[i++], v[j]);
        }
    }
    // put pivot just after smaller elements
    swap(v[i], v[right]);

    // recursively sort subarrays
    if (i > left + 1) quick_sort_st(v, left, i - 1);
    if (i + 1 < right) quick_sort_st(v, i + 1, right);
}

// multi threaded merge sort
void merge_sort_mt(vector<int> &v, const unsigned left, const unsigned right) {
    if (left >= right) return;

    // since there is a lot of overhead to make new threads,
    // use single thread for small arrays
    if (right - left <= 10000) {
        merge_sort_st(v, left, right);
        return;
    }

    const unsigned mid = (left + right) / 2;
    // create threads for recursively divide and sort subarray
    thread left_thread([&v, left, mid] {
        merge_sort_mt(v, left, mid);
    });
    thread right_thread([&v, mid, right] {
        merge_sort_mt(v, mid + 1, right);
    });

    // wait for threads to finish
    left_thread.join();
    right_thread.join();

    // merge subarrays
    unsigned i = left, j = mid + 1, k = 0;
    vector<int> temp(right - left + 1);
    while (i <= mid && j <= right) {
        if (v[i] <= v[j]) temp[k++] = v[i++];
        else temp[k++] = v[j++];
    }

    // fill remaining elements
    while (i <= mid) temp[k++] = v[i++];
    while (j <= right) temp[k++] = v[j++];

    // copy back to original array
    copy(temp.begin(), temp.end(), v.begin() + left);
}

// multi threaded quick sort
void quick_sort_mt(vector<int> &v, const unsigned left, const unsigned right) {
    if (left >= right) return;

    // since there is a lot of overhead to make new threads,
    // use single thread for small arrays
    if (right - left <= 10000) {
        quick_sort_st(v, left, right);
        return;
    }

    // Partition
    const int pivot = v[right];
    unsigned i = left;
    for (unsigned j = left; j < right; j++) {
        if (v[j] <= pivot) {
            swap(v[i++], v[j]);
        }
    }
    swap(v[i], v[right]);

    // create threads for recursively divide and sort subarray
    vector<thread> threads;
    if (i > left + 1) {
        threads.emplace_back([&v, left, i] { quick_sort_mt(v, left, i - 1); });
    }
    if (i + 1 < right) {
        threads.emplace_back([&v, i, right] { quick_sort_mt(v, i + 1, right); });
    }

    // wait for threads to finish
    for (auto &t: threads) {
        t.join();
    }
}


int main() {
    srand(0);
    vector<int> v = random_vector(5'000'000, 0, 1'000'000'000);
    // vector<int> v = {6, 5, 4, 9, 7, 8, 7};
    // vector<int> v = {3, 4, 2, 1};
    const unsigned n = v.size() - 1;

    const auto start = chrono::steady_clock::now();

    // choose one of the algos (st: single thread, mt: multithread)
    merge_sort_st(v, 0, n);
    // quick_sort_st(v, 0, n);
    // merge_sort_mt(v, 0, n);
    // quick_sort_mt(v, 0, n);

    const auto end = chrono::steady_clock::now();

    // print_vector(v);
    printf("Sorted? %s\n", is_sorted(v.begin(), v.end()) ? "YES" : "NO");
    printf("time: %lu ms\n", chrono::duration_cast<chrono::milliseconds>(end - start).count());

    return 0;
}
