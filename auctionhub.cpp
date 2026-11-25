#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <random>

using namespace std;
using namespace std::chrono;

struct Bid {
    string bidderId;
    double amount;       // bid price
    long long timestamp; // Unix ms
    int auctionId;
};

//  Utilities 
void printBids(const vector<Bid>& v, size_t limit = 10) {
    for (size_t i = 0; i < min(v.size(), limit); ++i)
        cout << "[" << v[i].bidderId << "] $" << v[i].amount
             << " @ " << v[i].timestamp << "\n";
    if (v.size() > limit) cout << "... (" << v.size() << " total)\n";
}

bool loadBids(const string& path, vector<Bid>& out) {
    ifstream f(path);
    if (!f) return false;
    out.clear();
    Bid b;
    while (f >> b.bidderId >> b.amount >> b.timestamp >> b.auctionId)
        out.push_back(b);
    return true;
}

// So sanh tang dan: uu tien amount, neu bang nhau thi xet timestamp tang dan
bool isLess(const Bid& a, const Bid& b) {
    if (a.amount != b.amount) return a.amount < b.amount;
    return a.timestamp < b.timestamp;
}

// Kiem tra mang da sap xep tang dan chua
bool isSortedAsc(const vector<Bid>& v) {
    for (size_t i = 1; i < v.size(); ++i) {
        if (isLess(v[i], v[i - 1])) return false;
    }
    return true;
}

// Sinh file 100,000 bids ngau nhien cho Task E
void generateLargeData(const string& path, int count = 100000) {
    ofstream out(path);
    mt19937 rng(42);
    uniform_real_distribution<double> priceDist(10.0, 5000.0);
    uniform_int_distribution<long long> timeDist(1700000000000LL, 1700086400000LL);
    uniform_int_distribution<int> idDist(1, 500);

    for (int i = 0; i < count; ++i) {
        out << "user" << i << " "
            << priceDist(rng) << " "
            << timeDist(rng) << " "
            << idDist(rng) << "\n";
    }
}

//  Task A: Insertion Sort (Ascending) 
void insertionSort(vector<Bid>& v) {
    int n = static_cast<int>(v.size());
    for (int i = 1; i < n; ++i) {
        Bid key = v[i];
        int j = i - 1;
        while (j >= 0 && isLess(key, v[j])) {
            v[j + 1] = v[j];
            --j;
        }
        v[j + 1] = key;
    }
}

//  Task B: Selection Sort (Descending, In-place) 
void selectionSort(vector<Bid>& v) {
    int n = static_cast<int>(v.size());
    for (int i = 0; i < n - 1; ++i) {
        int maxIdx = i;
        for (int j = i + 1; j < n; ++j) {
            if (v[j].amount > v[maxIdx].amount ||
               (v[j].amount == v[maxIdx].amount && v[j].timestamp < v[maxIdx].timestamp)) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            swap(v[i], v[maxIdx]);
        }
    }
}

//  Task C: Interchange Sort (Ascending + Count Swaps) 
void interchangeSort(vector<Bid>& v) {
    int n = static_cast<int>(v.size());
    long long swapCount = 0;
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (isLess(v[j], v[i])) {
                swap(v[i], v[j]);
                ++swapCount;
            }
        }
    }
    cout << "Total swaps performed: " << swapCount << "\n";
}

//  Task D: Bubble Sort with Early Stop 
bool bubbleSortEarlyStop(vector<Bid>& v) {
    int n = static_cast<int>(v.size());
    if (n <= 1) return true;

    bool firstPassNoSwaps = true;
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; ++j) {
            if (isLess(v[j + 1], v[j])) {
                swap(v[j], v[j + 1]);
                swapped = true;
            }
        }
        // Ghi nhan trang thai cua luot duyet dau tien (i == 0)
        if (i == 0 && swapped) {
            firstPassNoSwaps = false;
        }
        // Dung som neu luot hien tai khong co hoan doi nao
        if (!swapped) {
            break;
        }
    }
    return firstPassNoSwaps;
}

//  Task E: Quick Sort (Median-of-Three + Lomuto) 
int medianOfThree(const vector<Bid>& v, int lo, int hi) {
    int mid = lo + (hi - lo) / 2;
    if ((isLess(v[lo], v[mid]) && isLess(v[mid], v[hi])) ||
        (isLess(v[hi], v[mid]) && isLess(v[mid], v[lo]))) {
        return mid;
    }
    if ((isLess(v[mid], v[lo]) && isLess(v[lo], v[hi])) ||
        (isLess(v[hi], v[lo]) && isLess(v[lo], v[mid]))) {
        return lo;
    }
    return hi;
}

int partitionLomuto(vector<Bid>& v, int lo, int hi) {
    int medIdx = medianOfThree(v, lo, hi);
    swap(v[medIdx], v[hi]); // Dua pivot trung vi ve cuoi doan

    Bid pivot = v[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; ++j) {
        if (isLess(v[j], pivot)) {
            ++i;
            swap(v[i], v[j]);
        }
    }
    swap(v[i + 1], v[hi]);
    return i + 1;
}

void quickSort(vector<Bid>& v, int lo, int hi) {
    if (lo < hi) {
        int p = partitionLomuto(v, lo, hi);
        quickSort(v, lo, p - 1);
        quickSort(v, p + 1, hi);
    }
}

void quickSortWrapper(vector<Bid>& v) {
    if (v.empty()) return;
    quickSort(v, 0, static_cast<int>(v.size()) - 1);
}

//  Driver 
int main() {
    vector<Bid> smallBids;
    if (!loadBids("test_data/small.txt", smallBids)) {
        cerr << "Khong mo duoc file test_data/small.txt\n";
        return 1;
    }

    // --- Test Task A ---
    vector<Bid> taskA = smallBids;
    insertionSort(taskA);
    cout << "=== Task A: Insertion Sort (Ascending) ===\n";
    printBids(taskA);

    // --- Test Task B ---
    vector<Bid> taskB = smallBids;
    selectionSort(taskB);
    cout << "\n=== Task B: Selection Sort (Top 3 Highest Bids) ===\n";
    printBids(taskB, 3);

    // --- Test Task C ---
    vector<Bid> taskC = smallBids;
    cout << "\n=== Task C: Interchange Sort ===\n";
    interchangeSort(taskC);
    printBids(taskC);

    // --- Test Task D ---
    cout << "\n=== Task D: Bubble Sort Early Stop ===\n";
    vector<Bid> sortedCase = taskA; // Da sap xep tang dan
    bool res1 = bubbleSortEarlyStop(sortedCase);
    cout << "Case 1 (Already-sorted): " << (res1 ? "STABLE" : "UNSTABLE") << "\n";

    vector<Bid> reversedCase = taskB; // Dang giam dan (dao nguoc)
    bool res2 = bubbleSortEarlyStop(reversedCase);
    cout << "Case 2 (Reversed input): " << (res2 ? "STABLE" : "UNSTABLE") << "\n";

    // --- Test Task E (100,000 bids) ---
    cout << "\n=== Task E: Quick Sort (100,000 records) ===\n";
    generateLargeData("test_data/large.txt", 100000);

    vector<Bid> largeBids;
    if (!loadBids("test_data/large.txt", largeBids)) {
        cerr << "Khong mo duoc file test_data/large.txt\n";
        return 1;
    }

    auto start = high_resolution_clock::now();
    quickSortWrapper(largeBids);
    auto stop = high_resolution_clock::now();

    double elapsedMs = duration<double, milli>(stop - start).count();
    cout << "Loaded records: " << largeBids.size() << "\n";
    cout << "Sorted verified: " << (isSortedAsc(largeBids) ? "YES" : "NO") << "\n";
    cout << "Quick Sort runtime: " << elapsedMs << " ms\n";
    cout << "First 5 sorted bids:\n";
    printBids(largeBids, 5);

    return 0;
}