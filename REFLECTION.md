Reflection - AuctionHub Real-Time Bid Analytics Engine

1. Real-World Justification for Each Sorting Algorithm

- Task A - Insertion Sort (Live Auction Monitor): Bids in a live auction stream arrive in nearly-sorted order. Insertion Sort achieves O(n) best-case time complexity on nearly-sorted data because the inner while loop terminates immediately without shifting elements when a new bid is already larger than the previous maximum, avoiding the recursion overhead of divide-and-conquer algorithms.
- Task B - Selection Sort (Top-K Bid Finder): When extracting only the Top 3 highest bids from a small dataset, Selection Sort naturally places the k-th largest element into its final position on the k-th outer pass while performing at most n - 1 swaps (O(n) data writes). It operates strictly in-place with O(1) auxiliary memory, making it clean and predictable for small leaderboard widgets.
- Task C - Interchange Sort (Price Anomaly Validator): For small QA validation batches (<= 50 records), the difference between O(n^2) and O(n log n) is negligible (under a millisecond). Interchange Sort is acceptable here because its implementation is extremely simple, easy to audit for correctness without edge-case bugs, and requires zero extra memory.
- Task D - Bubble Sort with Early Stop (Stabilization Detector): The early-stop swapped flag allows Bubble Sort to verify whether an incoming price window is already ordered in a single linear scan (O(n) best-case time). If no swaps occur during the first pass, the algorithm immediately exits and flags the stream as STABLE.
- Task E - Quick Sort (Full Historical Sorter): End-of-day reporting requires sorting hundreds of thousands or millions of historical bids where O(n^2) algorithms become unusable. Quick Sort provides O(n log n) average time complexity, strong cache locality on contiguous std::vector memory, and O(log n) auxiliary stack space.

2. Quick Sort Runtime on 100,000 Records

- Dataset size: 100,000 randomly generated Bid records (test_data/large.txt)
- Verification: Confirmed strictly ascending order (isSortedAsc == true)
- Measured Runtime: 189.105 ms (using std::chrono::high_resolution_clock)

3. Bug Encountered and Diagnosis

- Bug: Initially, the program failed to load test_data/small.txt at runtime (ifstream returned false), and in quickSortWrapper, passing v.size() - 1 directly risked unsigned integer underflow (size_t wrapping to SIZE_MAX) if an empty vector was passed.
- Diagnosis: Checking the terminal working directory revealed auctionhub.cpp was placed inside test_data/ instead of the project root directory. Moving auctionhub.cpp to the root folder (AuctionHub/) resolved the relative path lookup, and adding an explicit if (v.empty()) return; guard in quickSortWrapper prevented potential out-of-bounds memory access on empty inputs.

4. LLM Usage Log

1. Prompt 1: "Generate C++ code using std::mt19937 to produce 100,000 random Bid structs and write them to a file."
   - Rating: Green Zone - Used strictly to generate synthetic test data utility code rather than the sorting algorithms themselves.
2. Prompt 2: "Explain how the median-of-three pivot selection works with Lomuto partitioning and why it avoids worst-case splits on sorted inputs."
   - Rating: Green Zone - Asked for conceptual clarification of the pivot selection mechanism before implementing the C++ code.
3. Prompt 3: "Review my bubbleSortEarlyStop loop logic to verify whether checking swapped on the first pass accurately detects an already-sorted stream."
   - Rating: Yellow/Green Zone - Used the LLM to verify boundary conditions on my own implementation.

5. Worst-Case Complexity of Quick Sort with Median-of-Three

- Worst-case time complexity: Yes, it is still O(n^2).
- Why: Although median-of-three eliminates the O(n^2) worst case on already-sorted and reverse-sorted arrays by selecting the true middle element as the pivot, the selection rule remains deterministic (always inspecting indices lo, mid, and hi). Consequently, an adversary can construct a specific "median-of-three killer" permutation where the two smallest or two largest remaining elements repeatedly land at lo, mid, and hi at every recursive step, causing unbalanced 0 and n - 1 partitions that degrade runtime to O(n^2).