# needle-in-a-haystack

# Source code and compilation
The core of the solution can be found in the file "substring.cpp" and consists of approx. 50 code lines. Unit tests are in "tests.cpp", together with a naive implementation of the solution. "main.cpp" provides a simple wrapper for the algorithm, which feeds it with input data from the files in the current directory and prints the common substrings found. The code is in C++14 and compiles with Visual Studio 2015 RC. I would also expect it to work with GCC 4.9+ and with CLang. "tests.cpp" could be excluded from the build.

# Running the program
Invoke it like this:

exe-file-name 100 haystack.txt needle.txt result.txt

In this example, the threshold is 100 bytes and the substrings will be placed in "result.txt". When the last argument is omitted, the program prints results to stdout. Invoking the program without arguments is equivalent to the following command line:

exe-file-name 50 haystack.txt needle.txt.

# Sample results
In addition to the source code files, sample input and output files are attached, "haystack.txt" (2Mb), "needle.txt" (1Mb) and "result.txt". The contents of the first two files are two different English translations of "Don Quixote", with line breaks, commas and extra spaces removed. The output file contains all the common substrings of these two versions of the text greater than 50 characters. The processing time was about 10 seconds.

# Details of the algorithm
A variant of Rabin-Karp method is implemented, based on the rolling hash idea. Adler-32 rolling hash was chosen for its simplicity.

During the initialization, the hashes of all the needle substrings with length threshold are generated and stored in the hash table as hash-substring index pairs. During the main phase, all the new input sequences from haystack, each of length threshold, are looked up in the hash table, using their Adler fingerprint. If match is found, the candidate substrings will be compared byte to byte to work around possible hash collisions.

 If match is confirmed, the verified substrings will grow with each new matching byte from the haystack, until the needle ends, or until the new haystack byte mismatches the corresponding needle byte.

The execution time is constant per byte input from haystack, i.e. the algorithm has O(|haystack|) time average complexity. Memory requirements are O(|needle|) in average, and O(|needle|^2) in the worst case due to the storage of |needle|^2 substrings. A trivial worst case is needle "aaaa" matched vs infinite haystack "aaaaa..."

The method is "online": haystack is never fully stored in memory and can be infinitely long. Preparation phase, when the hash table of needle substrings is built, also uses the rolling hash and therefore takes no more than 1-10 sec per 1Mb of needle.

The growing common substrings are stored in another dedicated hash table (STL unordered_set). When a new common substring emerges, it is matched against the strings in the set, thus avoiding the case when the new substring is a part of another one, found earlier. This comparison takes into account the positions of the substrings in haystack and needle, not just their contents.

"Growing" the found common substrings takes additional O(|Nsubstrings|) operations per each byte from haystack, to check if some of the substrings are complete.