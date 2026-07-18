#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include "types.h"

using namespace std;

struct Compare {
    bool operator()(const pair<Nombre,TI>& a,
                    const pair<Nombre,TI>& b) const {
        return a.second > b.second;   // Min Heap
    }
};

struct Student{
    Nombre name;
    vector<TI> grades;

    double average() const{
        return accumulate(grades.begin(),
                          grades.end(),
                          0.0) / grades.size();
    }
};

void demo1() {

    vector<Nombre> words = {
        "java","cpp","python","cpp",
        "java","cpp","go","rust",
        "python","cpp","java"
    };

    unordered_map<Nombre,TI> freq;

    for(const auto& word : words)
        freq[word]++;

    priority_queue<
        pair<Nombre,TI>,
        vector<pair<Nombre,TI>>,
        Compare
    > pq;

    size_t K = 4;

    for(const auto& item : freq){
        pq.push(item);

        if(pq.size() > K)
            pq.pop();
    }

    vector<pair<Nombre,TI>> result;

    while(!pq.empty()){
        result.push_back(pq.top());
        pq.pop();
    }

    reverse(result.begin(), result.end());

    cout << "Top " << K << " palabras:\n";

    for(const auto& e : result)
        cout << e.first << " -> " << e.second << endl;
}

void demo2(){

    vector<Student> students = {
        {"Carlos",{18,20,19}},
        {"Ana",{15,16,17}},
        {"Luis",{20,19,20}},
        {"María",{18,17,18}},
        {"Pedro",{14,15,16}}
    };

    sort(students.begin(),
         students.end(),
         [](const Student& a,
            const Student& b){
                return a.average() > b.average();
         });

    cout << "Ranking\n\n";

    size_t pos = 1;

    for(const auto& s : students){
        cout << pos++
             << ". "
             << s.name
             << " promedio = "
             << s.average()
             << endl;
    }
}

void DemoStl() {
    cout << "########## DEMO STL ##########\n";
    cout << "1) Top K palabras mas frecuentes\n";
    demo1();
    cout << "\n2) Ranking de estudiantes por promedio\n";
    demo2();
}