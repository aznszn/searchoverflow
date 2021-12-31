#include <filesystem>

void buildInverted() {
    using namespace std::filesystem;

    for(const auto& entry : directory_iterator(current_path().string() + "/../data_structures/barrels")) {
        wcout << entry.path() << endl;
        wifstream curr(entry.path());

        vector<vector<wstring>> f_index_file = fetch_table(curr);

        auto sorted = vector<vector<wstring>>(f_index_file.size());
        vector<int> elem_array(1000, 0);

        for(auto& x : f_index_file)
            elem_array[stoi(x[1])]++;

        for(int i = 1; i < 1000; ++i)
            elem_array[i] += elem_array[i-1];

        for(auto & i : f_index_file)
            sorted.at(elem_array[stoi(i[1])]-- - 1) = i;

        wofstream currOut(entry.path(), ios::out);
        currOut << setw(199) << " ";
        currOut << "\n";
        for(auto& row: sorted){
            for(auto& column : row)
                currOut << column << ",";
            currOut.seekp(-1, ios::cur);
            currOut << "\n";
        }
        currOut.close();
        resize_file(entry.path(), file_size(entry.path()) - 2);
    }
}