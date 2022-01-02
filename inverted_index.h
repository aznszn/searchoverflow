#include <filesystem>
#include <set>

#define WORDS_IN_FILE 500

void buildInverted() {
    using namespace std::filesystem;

    //iterate over every barrel
    for (const auto &entry: directory_iterator(current_path().string() + "/../data_structures/barrels")) {
        wcout << entry.path() << endl;
        wifstream curr(entry.path());

        //get the entire barrel in this 2D vector
        vector<vector<wstring>> f_index_file = fetch_table(curr);

        //counting sort algo
        auto sorted = vector<vector<wstring>>(f_index_file.size());
        vector<int> elem_array(WORDS_IN_FILE, 0);   //max

        for (auto &x: f_index_file)
            elem_array[stoi(x[1])]++;   //frequency

        for (int i = 1; i < WORDS_IN_FILE; ++i)
            elem_array[i] += elem_array[i - 1]; //cumulative frequency

        for (auto &i: f_index_file)
            sorted.at(elem_array[stoi(i[1])]-- - 1) = i;    //placing at appropriate index

        wofstream currOut(entry.path(), ios::out);      //writing back the inverted barrel)
        currOut << setw(199) << " ";
        currOut << "\n";
        for (auto &row: sorted) {
            for (auto &column: row)
                currOut << column << ",";
            currOut.seekp(-1, ios::cur);
            currOut << "\n";
        }
        currOut.close();
        resize_file(entry.path(), file_size(entry.path()) - 2); //remove trailing CRLF
    }
}

void updateInverted(set<int, greater<>> barrelsToUpdate) {
    using namespace std::filesystem;
    set<int, greater<> >::iterator itr;

    for (itr = barrelsToUpdate.begin(); itr != barrelsToUpdate.end(); itr++) {
        cout << "../data_structures/barrels/" + to_string(*itr) << endl;
        wifstream curr("../data_structures/barrels/" + to_string(*itr) + ".txt");

        vector<vector<wstring>> f_index_file = fetch_table(curr);

        auto sorted = vector<vector<wstring>>(f_index_file.size());
        vector<int> elem_array(WORDS_IN_FILE, 0);

        for (auto &x: f_index_file)
            elem_array[stoi(x[1])]++;

        for (int i = 1; i < WORDS_IN_FILE; ++i)
            elem_array[i] += elem_array[i - 1];

        for (auto &i: f_index_file)
            sorted.at(elem_array[stoi(i[1])]-- - 1) = i;

        wofstream currOut("../data_structures/barrels/" + to_string(*itr) + ".txt", ios::out);
        currOut << setw(199) << " ";
        currOut << "\n";
        for (auto &row: sorted) {
            for (auto &column: row)
                currOut << column << ",";
            currOut.seekp(-1, ios::cur);
            currOut << "\n";
        }
        currOut.close();
        resize_file(current_path().string() + "/../data_structures/barrels/" + to_string(*itr) + ".txt", file_size(current_path().string() + "/../data_structures/barrels/" + to_string(*itr) + ".txt") - 2);
    }
}