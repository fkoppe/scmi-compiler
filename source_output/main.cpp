#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

string readFile(const string &path);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    string input_file_path = argv[1];
    string content = readFile(input_file_path);

    stringstream ss(content);
    string line;
    vector<std::string> lines;

    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    vector<int> ints;

    for (int i = 0; i < lines.size(); i++) {
        auto line = lines[i];

        if (line.find(", R12") != string::npos) {
            string numLine = lines[i+1];

            size_t pos = numLine.find("R12:");
            if (pos != string::npos) {
                // Check if there's an arrow (->) in the line but specifically skip if it's not related to R12
                string r12string = numLine.substr(pos,numLine.size()-pos);
                size_t semiPos = r12string.find(";");
                if (semiPos != string::npos) {
                    string sub = r12string.substr(5, semiPos-5);
                    //
                    size_t arrowPos = sub.find("->");
                    if (arrowPos != string::npos) {
                        string o = sub.substr(arrowPos+3, sub.size()-arrowPos-3);
                        ints.push_back(stoi(o));
                    }
                    else {
                        ints.push_back(stoi(sub));
                    }
                }
            }

        }
    }



    string o = "";
    string o1 = "";
    for (int i = 0; i < ints.size(); i++) {
        o1+= to_string(ints[i])+" ";
        o += static_cast<char>(ints[i]);
    }
    cout << "nums: " << o1 << endl;
    cout << o << endl;
    return 0;
}

string readFile(const string& path) {
    ifstream file(path, ios::ate);
    if (!file) {
        throw runtime_error("Could not open file: " + path);
    }
    const streamsize size = file.tellg();
    file.seekg(0);
    string content = string(size, '\0');
    file.read(content.data(), size);
    return content;
}