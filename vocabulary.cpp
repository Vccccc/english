#include <cstring>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include <string>
using namespace std;
int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        cout << "usage: " << argv[0] << " filename" << endl;
        return 0;
    }

    FILE* fp = fopen(argv[1], "r");
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    string buffer(len, 0);
    fseek(fp, 0, SEEK_SET);
    if(fread(&buffer[0], sizeof(buffer[0]), buffer.size(), fp) < 0)
    {
        perror("failed to fread");
        return 0;
    }

    fclose(fp);

    const char* sep = "\n";
    char* start = &buffer[0];
    std::map<string, int> vocabularyCount;

    while(start && *start)
    {
        size_t spnsz = strspn(start, sep);
        start += spnsz;
        char* end = strpbrk(start, sep);

        string str;
        if(end)
        {
            str.assign(start, end);
        }
        else
        {
            if(*start == '\0')
            {
                break;
            } 
            str.assign(start, strlen(start));
        }
        start = end;

        vocabularyCount[str]++;
    }

    string toFile(argv[1]);
    toFile.append("2");
    FILE* fp2 = fopen(toFile.c_str(), "w");
    vector<pair<int, string>> sortCount;
    for(auto& word_and_count : vocabularyCount)
    {
        sortCount.push_back({word_and_count.second, word_and_count.first});
    }

    sort(begin(sortCount), end(sortCount), [](const pair<int, string>& left, const pair<int, string>& right){
            if(left.first < right.first)
            {
            return false;
            }
            else if(left.first == right.first)
            {
            return left.second < right.second;
            }
            else
            {
            return true;
            }
            });

    for(auto& elem : sortCount)
    {
        char buf[50] = {0};
        snprintf(buf, sizeof buf, "%-30s*%d\n", &elem.second.c_str()[0], elem.first);
        fwrite(&buf[0], sizeof(buf[0]), strlen(buf), fp2);

    }
    fclose(fp2);
    return 0;
}
