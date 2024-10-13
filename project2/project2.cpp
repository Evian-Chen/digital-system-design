#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>

using namespace std;

class Robdd {
public:
    string inputFile, outputFile;
    int varNum;                         // number of variables
    vector<string> varVec;              // vector storing all the variablesL ['a', 'b', 'c'...]
    map<string, set<int>> mintermMap;   // { "0001": {1}, "0011": {3}, ... }
    map<string, int> dontcareMap;       // { "0001": 1, "0011": 3, ... }
    map<int, set<string>> groupMap;     // group minterm: {1:{"0001", "0010"}, ...}
    set<string> epi = set<string>();    // store essential prime implicant
    set<set<string>> disMap = set<set<string>>();  // used to implement distributive law
    map<int, set<string>> implicantsMap = map<int, set<string>>();  // {minterm: {all implicants containing the key minterm}}
    string outputLine;  // store the basic information of the input pla file

    // constructor
    Robdd(string in, string out) {
        inputFile = in;
        outputFile = out;
        varNum = 0;
    }


    // this function put minterm into groups with different number of 1 
    void grouping(string minterm) {
        int num = count(minterm.begin(), minterm.end(), '1');
        if (groupMap.find(num) == groupMap.end()) {
            groupMap[num] = {minterm};
        }
        else {
            groupMap[num].insert(minterm);
        }
    }

    // this function reads in a literal term and return a set of all minterms of the input literal
    // don't care will not be the input, ex. 00-1
    void findMinterm(string term, string ans, int index) {
        if (index == varNum) {  // all literals is added
            mintermMap[ans] = { addInBinary(ans) };
            grouping(ans);
        }
        else { 
            if (term[index] == '-') {
                findMinterm(term, ans+"0", index+1);
                findMinterm(term, ans+"1", index+1);
            }
            else {
                findMinterm(term, ans+term[index], index+1);
            }
        }
    }

    // this function reads in a string of binary representaion and return the decimal representation of term
    int addInBinary(string binary) {
        reverse(binary.begin(), binary.end());
        int ans = 0;
        for (int i=0; i<varNum; i++) {
            if (binary[i] == '1') ans += pow(2, i);
        }

        return ans;
    }

    // after this excuting this function, grouping done
    void readFile() {
        fstream f(inputFile);
        string line;
        bool start = false;
        while (getline(f, line)) {
            if (!start) outputLine += (line + '\n');
            if (line.find(".i") != string::npos && varNum == 0) varNum = stoi(line.substr(2));  // get varNum
            if (line.find(".ilb") != string::npos) varVec = split(line.substr(4), ' ');   // get varVec
            if (line.find(".e") != string::npos) start = false;
            
            if (start) {
                // get term
                string term = line.substr(0, varNum);

                // don't care literal, add to mintermMap directly, pass term to findMinterm otherwise
                if (line[line.size() - 1] == '-') {
                    findMinterm(term, "", 0);
                    dontcareMap[term] = addInBinary(term);
                    grouping(term);
                } 
                else {
                    findMinterm(term, "", 0);
                }
            }

            if (line.find(".p") != std::string::npos) start = true;
        }
    }

    // this function simplpified two implicants
    string simplfyProcess(string t1, string t2) {
        string ans = "";
        int diffCount = 0;
        for (int i=0; i<t1.size(); i++) {
            if (t1[i] != t2[i]) {
                diffCount++;
                if (diffCount > 1) {
                    return "";
                }
                else {
                    ans += '-';
                }
            }
            else {  // same literal
                ans += t1[i];
            }
        }
        return ans;
    }

    // this function should ne excuted mutiple times until return value is false
    // first simplfy with each other, but the simplfied one need to be group again
    bool simplfy() {
        bool isSimplfied = false;  // if there's at least one term is simplfed, then true
        set<string> abandondStr = set<string>();
        string simplifedStr;
        int group = 0;
        map<string, set<int>> simplifiedMap = map<string, set<int>>();
        
        for (int i=0; i<varNum; i++) {
            for (auto& term1: groupMap[i]) {

                for (auto& term2: groupMap[i+1]) {  // term is from term set
                    simplifedStr = simplfyProcess(term1, term2);
                    
                    if (simplifedStr != "") {
                        simplifiedMap[simplifedStr].insert(mintermMap[term1].begin(), mintermMap[term1].end());
                        simplifiedMap[simplifedStr].insert(mintermMap[term2].begin(), mintermMap[term2].end());

                        isSimplfied = true;
                        
                        // term1 can be simplified, need to be remove from mintermMap and groupMap
                        abandondStr.insert(term1);
                        abandondStr.insert(term2);
                    }
                }
            }
        }

        // remove all unneccessary terms from mintermMap & groupMap
        for (string term: abandondStr) {
            mintermMap.erase(term);

            group = count(term.begin(), term.end(), '1');
            groupMap[group].erase(term);
        }

        // update mintermMap & groupMap
        for (auto& pair: simplifiedMap) {
            if (mintermMap.find(pair.first) == mintermMap.end()) {
                mintermMap[pair.first] = pair.second;
            }
            else {
                mintermMap[pair.first].insert(pair.second.begin(), pair.second.end());
            }

            grouping(pair.first);
        }

        return isSimplfied;
    }

    // this funciton find essential prime implicant, update epi
    void findEPI() {
        map<int, string> minMap;  // the one with string is EPI, "" is not EPI
        for (auto& pair: mintermMap) {

            // make sure the term is not dontcare
            if (dontcareMap.find(pair.first) == dontcareMap.end()) {
                for (int min: pair.second) {
                    if (minMap.find(min) == minMap.end()) {  // if min not exist, it's first time to add min
                        minMap[min] = pair.first;
                        implicantsMap[min] = { pair.first };
                    }
                    else {  // min at least appear once, not EPI
                        minMap[min] = "";
                        implicantsMap[min].insert(pair.first);
                    }
                }
            }
        }

        // update epi and remove epi from implicantMap
        for (auto& pair: minMap) {
            if (pair.second.size() != 0) {
                epi.insert(pair.second);

                // find the rest of the implicant
                for (int t: mintermMap[pair.second]) {
                    if (implicantsMap.find(t) != implicantsMap.end()) {
                        implicantsMap.erase(t);
                    }
                }
                
            }
        } 
    }

    // this function loop through all the implicants
    void findSol() {
        for (auto& pair : implicantsMap) {
            combine(pair.second);  // pair.second is a set<string>
        }
    }

    // this function do the distributive law, which aims to get all the possible combination
    void combine(set<string> s) {
        if (disMap.empty()) {
            // if disMap is empty, initialize it with the elements from 's'
            for (string str : s) {
                disMap.insert({str});  // insert each element as a new set
            }
        }
        else {
            // temporary containers to store new sets and sets to be deleted
            set<set<string>> newDisMap;
            set<set<string>> toDelete;
            bool isRepeted = false;

            // combine the new set 's' with each set in disMap
            for (set<string> strSet : disMap) {
                for (string str : s) {
                    set<string> result = strSet;  // copy the current set from disMap
                    result.insert(str);           // add the element from 's'
                    newDisMap.insert(result);     // store the new combination
                    if (strSet == result) {
                        isRepeted = true;
                    }
                }
                if (!isRepeted) {
                    toDelete.insert(strSet);  // mark the old set for deletion
                }
                isRepeted = false;
            }

            // add the new distributed sets to disMap
            disMap.insert(newDisMap.begin(), newDisMap.end());

            // erase the old sets from disMap
            for (const auto& oldSet : toDelete) {
                disMap.erase(oldSet);
            }
        }
    }

    // this function write the implified implicants to a pla file
    void writePLA(set<string> result) {
        ofstream out;
        out.open(outputFile);
        if (out.is_open()) {
            out << outputLine.substr(0, outputLine.size()-2);
            out << result.size() << endl;
            for (string s: result) {
                out << s << " 1\n";
            }
            out << ".e";
        }
        
    }

    // this function find all the feasible implicants and return with epi
    set<string> findFinal() {
        set<string> result;
        int minimum = 100;
        for (set<string> strSet: disMap) {
            if (strSet.size() <= minimum) {
                result = strSet;
                minimum = strSet.size();
            }
        }

        for (string str: epi) {
            result.insert(str);
        }

        return result;
    }

private:

    // This funciton return a vector of character(variables): "a b c" -> ['a', 'b', 'c']
    vector<string> split(string line, char del) {
        vector<string> v;
        size_t end = 0, start = 0;
        end = line.find(del, start);
        while (end != string::npos) {
            v.push_back(line.substr(start, end-start));
            start = end + 1;
            end = line.find(del, start);
        }

        return v;
    }
};


int main(int argc, char** argv) {
    string inputFile, outputFile;
    set<string> finalAns;
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
        return 1;
    }

    inputFile = argv[1];
    outputFile = argv[2];

    Robdd robddObj(inputFile, outputFile);
    
    robddObj.readFile();

    // this loop make sure the funciton is completely simplified
    bool isSimplfied = robddObj.simplfy();
    while (isSimplfied) {
        isSimplfied = robddObj.simplfy();
    }

    robddObj.findEPI();

    robddObj.findSol();

    set<string> result;
    result = robddObj.findFinal();

    robddObj.writePLA(result);
}