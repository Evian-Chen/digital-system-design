#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <tuple>

using namespace std;

class State {
public:
    int varNum = 0;
    int inputNumLen = 0;
    int outputLen = 0;
    bool hasUpdated = false;

    vector<string> varName;

    // { curState: {tuple of next states and the outputs, index is the input value (need to be changed to binary when outputting)}, ... }
    // ex:{ "S1": {("S2", "0"),("S3", "0"), ("S4", "1")}, "S3": {...}, ... } 
    map<string, vector<tuple<string, string>>> infoMap;
    
    // {'a': {'b': {(t, t), (t, t), ...}, 'c': {(t, t), (t, t), ...}, ...}, 'b': {'c': {(t, t), (t, t), ...}, 'd': {(t, t), (t, t), ...}, ...}, ...}
    // {cur: { next: { (next when in=0), (next when in=1), (next when in=2), ...),  } }}
    map<string, map<string, vector<tuple<string, string>>>> implicationMap;

    // for making dot file
    // {("S1", "S2"): { (input, output), (input, output), ... }}
    map<string, vector<string>> dotMap;

    bool checkOutput(string state1, string state2) {
        // compare all output, all the same  then return true
        for (size_t i=0; i<infoMap[state1].size(); i++) {
            if (get<1>(infoMap[state1][i]) != get<1>(infoMap[state2][i])) {
                return false;
            }
        }
        return true;
    }

    void initImpliMap() {
        tuple<string, string> temp;

        for (size_t i=0; i<varName.size()-1; i++) {  
            // initialize for every key char(state)
            implicationMap[varName[i]] = map<string, vector<tuple<string, string>>>();

            for (size_t j=i+1; j<varName.size(); j++) {
                if (i != j) {  // the states are different
                    string stateX = varName[i];
                    string stateY = varName[j]; 
                    vector<tuple<string, string>> tempV;

                    // compare all the output of the block
                    if (checkOutput(stateX, stateY)) {  // if the output is valid
                        for (int i=0; i<infoMap[stateX].size(); i++) {
                            tempV.push_back({ get<0>(infoMap[stateX][i]), get<0>(infoMap[stateY][i]) });
                        }
                    }
                    else {  // if the output are not the same(can not be substitude)
                        for (int _=0; _<infoMap[varName[i]].size(); _++) {
                            tempV.push_back({ "-", "-" });
                        }
                    }
                    
                    implicationMap[varName[i]][varName[j]] = tempV;
                }
            }
        }
    }

    // after this excuting this function, grouping done
    void readFile(string inputFile) {
        fstream f(inputFile);
        string line;
        vector<string> elements;
        while (getline(f, line)) {
            if (line.find(".p") != string::npos && varNum == 0) inputNumLen = stoi(line.substr(2));  
            
            // in, cur, next, out
            if (line.find(".") == string::npos) {
                elements = split(line, ' ');  // [in, cur, next, out], ex: ['1', 'a', 'b', '0'] 
                updateInfo(elements);
            }
        }
    }

    void updateInfo(vector<string> elements) {
        // if this current state hasn't been seen
        if (infoMap.find(elements[1]) == infoMap.end()) {
            vector<tuple<string, string>> temp;
            tuple<string, string> output = { elements[2], elements[3] };

            varName.push_back(elements[1]);
            temp.push_back(output);
            infoMap[elements[1]] = temp;
        }
        else {  // only the state tuple needs to be altered
            infoMap[elements[1]].push_back({ elements[2], elements[3] });
        }
    }

    bool simplify() {
        // add states that need to be deleted
        bool hasChanged = false;

        // {cur: { next: { (next when in=0), (next when in=1), (next when in=2), ...),  } }}
        // {'a': {'b': {(t, t), (t, t), ...}, 'c': {(t, t), (t, t), ...}, ...}, 'b': {'c': {(t, t), (t, t), ...}, 'd': {(t, t), (t, t), ...}, ...}, ...}
        for (auto outerIt = implicationMap.begin(); outerIt != implicationMap.end(); ++outerIt) {
            string outerKey = outerIt->first;
            auto& innerMap = outerIt->second;

            for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt) {
                string innerKey = innerIt->first;
                vector<tuple<string, string>> statesTuples = innerIt->second;

                for (tuple<string, string> statesTuple: statesTuples) {
                    string stateX = get<0>(statesTuple);
                    string stateY = get<1>(statesTuple);
                    string toCheck;

                    // check which state is bigger
                    auto itX = find(varName.begin(), varName.end(), stateX);
                    auto itY = find(varName.begin(), varName.end(), stateY);
                    int inX = distance(varName.begin(), itX);
                    int inY = distance(varName.begin(), itY);

                    if (inX != inY) {
                        if (inX < inY) {  // legal
                            toCheck = get<0>(implicationMap[stateX][stateY][0]);
                        }
                        else {
                            toCheck = get<0>(implicationMap[stateY][stateX][0]);
                        }

                        if (toCheck == "-") {
                            for (size_t i=0; i<implicationMap[outerKey][innerKey].size(); i++) {
                                get<0>(implicationMap[outerKey][innerKey][i]) = "-";
                                get<1>(implicationMap[outerKey][innerKey][i]) = "-";
                            }

                            hasChanged = true;
                        }
                    }
                }
            }
        }

        return hasChanged;
    }

    void replace() {
        set<string> keyToDel;
        // when start doing the replacement, alter the infoMap
        // loop through the implicationMap
        for (auto outerIt = implicationMap.begin(); outerIt != implicationMap.end(); ++outerIt) {
            string keyX = outerIt->first;
            auto& innerMap = outerIt->second;

            for (auto innerIt =  innerMap.begin(); innerIt != innerMap.end(); ++innerIt) {
                string keyY = innerIt->first;
                vector<tuple<string, string>> statesTuples = innerIt->second;

                if (get<0>(statesTuples[0]) != "-") {  // only use the clock that hasn't been removed
                    // keyY is replaced by keyX 
                    for (auto infoIt = infoMap.begin(); infoIt != infoMap.end(); ++infoIt) {
                        string infoKay = infoIt->first;

                        for (int i=0; i<infoMap[keyX].size(); i++) {
                            if (get<0>(infoMap[infoKay][i]) == keyY) {
                                get<0>(infoMap[infoKay][i]) = keyX;
                            }
                            get<1>(infoMap[keyY][i]) = "-";
                        }
                    }
                    keyToDel.insert(keyY);
                }
            }
        }

        // clean the infoMap
        for (string key: keyToDel) {
            infoMap.erase(key);
        }
    }

    void writeKiss(string fileName) {
        ofstream  out(fileName);
        out << ".start_kiss\n.i 1\n.o 1\n.p " << outputLen << '\n';
        out << ".s " << infoMap.size() << "\n.r a\n";

        for (auto it = infoMap.begin(); it != infoMap.end(); ++it) {
            string cur = it->first;
            vector<tuple<string, string>> tuples = it->second;
            for (int i=0; i<tuples.size(); i++) {
                out << deToBi(i) << " " << cur << " " << get<0>(tuples[i]) << " " << get<1>(tuples[i]) << endl;
            }
        }

        out << ".end_kiss";
        out.close();
    }

    void makeDot() {
        // {("S1", "S2"): { "input/output", "input/output", ... }} dotMap
        // {cur: { next: { (next when in=0), (next when in=1), (next when in=2), ...),  } }}
        int sum = 0;
        for (auto outerIt=infoMap.begin(); outerIt!=infoMap.end(); ++outerIt) {
            string states = outerIt->first + " -> ";
            int count = 0;
            for (auto tuple: outerIt->second) {
                if (dotMap.find(states + get<0>(tuple)) == dotMap.end()) {
                    dotMap[states + get<0>(tuple)] = vector<string>();
                }
                string result = deToBi(count) + "/" + get<1>(tuple);
                dotMap[states + get<0>(tuple)].push_back(result);
                count++;
                sum++;
            }
        }
        outputLen = sum;
    }

    void writeDot(string fileName) {
        ofstream  out(fileName);
        out << "digraph STG {\n   rankdir=LR;\n\n   INIT [shape=point];\n";
        for (auto it=infoMap.begin(); it != infoMap.end(); ++it) {
            string key = it->first;
            out << "   " << key << " [label=\"" << key << "\"];\n";
        }

        out << "\n   INIT -> " << varName[0] << ";\n";

        for (auto it=dotMap.begin(); it != dotMap.end(); ++it) {
            out << "   " << it->first << " [label=\"";
            out << it->second[0];
            for (size_t i=1; i<it->second.size(); i++) {
                out << "," << it->second[i];
            }
            out << "\"];\n";
        }
        out << "}";
    }

    void printInfoMap() {
        for (auto it = infoMap.begin(); it != infoMap.end(); ++it) {
            cout << "Key: " << it->first << endl;

            int count = 0;

            for (const auto& tuple : it->second) {
                cout << "  input = " << count++ << " :(" << get<0>(tuple) << ", " << get<1>(tuple) << ")" << endl;
            }
        }
    }


    // map<string, map<string, vector<tuple<string, string>>>> implicationMap;
    void printImplicationMap() {
        for (auto it=implicationMap.begin(); it != implicationMap.end(); ++it) {
            cout << "X-axis: " << it->first << endl;
            map<string, vector<tuple<string, string>>> innerMap = it->second;
            for (auto innerIt=innerMap.begin(); innerIt != innerMap.end(); ++innerIt) {
                cout << "  Y-axis: " << innerIt->first << endl;
                for (tuple<string, string> tuples: innerIt->second) {
                    cout << "    states: (" << get<0>(tuples) << ", " << get<1>(tuples) << ")\n";
                }
            }

            cout << endl;
        }
    }

private:

    // this funciton return a vector of character(variables): "a b c" -> ['a', 'b', 'c']
    vector<string> split(const string &line, char del) {
        vector<string> v;
        size_t start = 0, end = 0;

        while ((end = line.find(del, start)) != string::npos) {
            // Add token if it is not empty
            if (end != start) {
                v.push_back(line.substr(start, end - start));
            }
            start = end + 1;
        }

        // Add the last token if it is not empty
        if (start < line.size()) {
            v.push_back(line.substr(start));
        }

        return v;
    }

    // convert binary(string) num to decimal(int)
    int biToDe(string binary) {
        int result = 0;
        for (int i=binary.length()-1; i>=0; i--) {
            result += ((binary[i]-'0') * pow(2, binary.length()-1-i));
        }

        return result;
    }

    // convert decimal(int) number to binary (string)
    string deToBi(int num) {
        string ans = "";
        if (num == 0) {
            ans = "0";
        }
        else {
            while (num != 0) {
                ans = ((num % 2 == 1) ? "1": "0") + ans;
                num /= 2;
            }
        }

        for (int i=0; i<(inputNumLen/varName.size())/2-ans.length(); i++) {
            ans = "0" + ans;
        }
        
        return ans;
    }
};


int main(int argc, char** argv) {
    string inputFile, outputKiss, outputDot;
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
        return 1;
    }

    inputFile = argv[1];
    outputKiss = argv[2];
    outputDot = argv[3];

    State state;

    state.readFile(inputFile);
    state.initImpliMap();

    bool keepSimplfy = state.simplify();
    while (keepSimplfy) {
        keepSimplfy = state.simplify();
    }

    state.replace();

    state.printInfoMap();

    state.makeDot();

    state.writeKiss(outputKiss);
    state.writeDot(outputDot);
}