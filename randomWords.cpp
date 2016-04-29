#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>


using namespace std;

vector<string> words;


const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}


//Function to find unique position in list
int randomPosition(vector<int> arr){
    
    int randomNum;
    bool found = false;
    while (found==false) {
        randomNum = rand()%(words.size());
        if (find(arr.begin(), arr.end(), randomNum) == arr.end()) {
            arr.push_back(randomNum);
            found = true;
        }
    }
    return randomNum;
    
}

//Thread function to send words
void sendWords(){
    vector<int> arr;
    int pos = randomPosition(arr);
    string word = words[pos];
    cout<<word<<endl;
}

int main()
{
    fstream file;
    file.open("words.txt");
    time_t now = time(0);
    cout<<"Reading from file..."+currentDateTime()<<endl;
    string line;
    while(getline(file, line))
    {
        words.push_back(line);
        line.clear();
    }
    file.close();
    now = time(0);
    cout<<"Read done..."+currentDateTime()<<endl;
    
    if (words.empty()) {
        cerr << "No Words Found\n";
    }
    int i = 20;
    while(i>0){
        thread t1(sendWords);
        t1.join();
        i--;
    }
    
    return 0;
}
