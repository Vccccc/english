#include <functional>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <sstream>
using namespace std;
struct Meta
{
    struct Hash
    {
        size_t operator()(const Meta& word) const
        {
            return hash<string>{}(word.content_);
        }
    };

    struct Compare
    {
        bool operator()(const Meta& lhs, const Meta& rhs) const
        {
            return lhs.content_ < rhs.content_;
        }
    };

    enum Type
    {
        EWORD,
        EPREFIX,
        ESUFFIX,
        EROOT,
        EINVALID,
        SIZE,
    };

    const char* Strs[Type::SIZE] = {
        "word",
        "prefix",
        "suffix",
        "root",
        "invalid",
    };
    Meta() : type_(EINVALID) {}
    Meta(Type type, const string& content)
        : type_(type), content_(content)
    {

    }

    bool operator==(const Meta& other) const
    {
        return content_ == other.content_;
    }

    string format() const
    {
        ostringstream stream;
        stream << "{ ";
        stream << "[" << content_ << "]: ";
        stream << "type: \"" << Strs[type_] << "\", ";
        if(!comments_.empty())
        {
            stream << "comment: ";
            stream << "[";
            bool first = true;
            for(auto& comment: comments_)
            {
                if(!first)
                {
                    stream << ", ";
                }
                stream << "\"" << comment << "\"";
                first = false;
            }
            stream << "] ";
        }

        if(!hint_.empty())
        {
            stream << "hint: \"" << hint_ << "\" ";
        }
        stream << "}";
        return stream.str();
    }

    void addComment(string comment)
    {
        comments_.emplace_back(std::move(comment));
    }

    void setHint(string hint)
    {
        hint_ = std::move(hint);
    }
    Type type_;
    string content_;
    vector<string> comments_;
    string hint_;
};

class Word
{
public:
    struct Hash
    {
        size_t operator()(const Word& word) const
        {
            return Meta::Hash()(word.getWord());
        }
    };

    struct Compare
    {
        bool operator()(const Word& lhs, const Word& rhs) const
        {
            return Meta::Compare()(lhs.getWord(), rhs.getWord());
        }
    };

    Word() = default;
    Word(const string& word) : word_(Meta::EWORD, word) {}

    bool operator==(const Word& other) const
    {
        return word_ == other.word_;
    }

    bool operator!=(const Word& other) const
    {
        return !(word_ == other.word_);
    }
public:
    string format() const
    {
        ostringstream stream;
        stream << "{\n";
        stream << "\tWord: " << word_.content_ << "\n";
        if(!word_.comments_.empty())
        {
            stream << "\tComment: [";
            bool first = true;
            for(auto& comment : word_.comments_)
            {
                if(!first)
                {
                    stream << ", ";
                }
                stream << "\"" << comment << "\"";
                first = false;
            }
            stream << "]\n";

        }
        if(!word_.hint_.empty())
        {
            stream << "\tHint: " << word_.hint_ << "\n";
        }
        if(!components_.empty())
        {
            stream << "\tComponent:\n";
            stream << "\t{\n";
            for(auto& component : components_)
            {
                stream << "\t\t" << component.format() << "\n";
            }
            stream << "\t}\n";
        }
        stream << "}\n";
        return stream.str(); 
    }

    void setComponent(int idx, const Meta& data)
    {
        if(idx >= components_.size())
        {
            components_.resize(idx+1);
        }
        components_[idx] = data;
    }
    void setComponent(vector<Meta> components)
    {
        components_.swap(components);
    }

    const Meta& getWord() const
    {
        return word_;
    }

    const vector<Meta>& getComponents() const
    {
        return components_;
    }
    
    void addComment(string comment)
    {
        word_.addComment(comment);
    }

    void setHint(string hint)
    {
        word_.setHint(hint);
    }
private:
    Meta word_;
    vector<Meta> components_;
};


class Server
{
public:
    const static Word Null;
    const Word& get(const string& key)
    {
        auto iter = words_.find(key);
        if(iter != words_.end())
        {
            return *iter;
        }
        return Null;
    }

    void set(const Word& word)
    {
        words_.insert(word);
        for(const auto& component : word.getComponents())
        {
            if(component.type_ == Meta::Type::EROOT)
            {
                roots_.insert(component);
            }
            else if(component.type_ == Meta::Type::EPREFIX)
            {
                prefixs_.insert(component);
            }
            else if(component.type_ == Meta::Type::ESUFFIX)
            {
                suffixs_.insert(component);
            }
            else 
            {
                assert(false);     
            }
        }
    }

    const char* skipAllSpace(const char* p)
    {
        while(*p == '\t' || *p == ' ')
        {
            p++;
        }
        return p;
    }
    
    bool parseCmd(const char* cmd)
    {
        const char* p;
        p = skipAllSpace(cmd);     
        if(p[0] == 's')
        {
            p++;
            p = skipAllSpace(p);
        }
        return true;
    }

    void printAllWord() 
    {
        cout << "Words>>>>>>>>>>>>>>>>>>:\n";
        for (auto &word : words_) 
        {
            cout << word.format() << "\n";
        }
    }

    void printAllRoot()
    {
        cout << "Roots>>>>>>>>>>>>>>>>>>:\n";
        for(auto& meta : roots_)
        {
            cout << meta.format() << "\n";
        }
    }

    void printAllPrefixs()
    {
        cout << "Prefixs>>>>>>>>>>>>>>>>>>:\n";
        for(auto& meta : prefixs_)
        {
            cout << meta.format() << "\n";
        }
    }

    void printAllSuffixs()
    {
        cout << "Suffixs>>>>>>>>>>>>>>>>>>:\n";
        for(auto& meta : suffixs_)
        {
            cout << meta.format() << "\n";
        }
    }

    void printAll()
    {
        printAllWord();
        printAllRoot();
        printAllPrefixs();
        printAllSuffixs();
    }
private:
    unordered_set<Word, Word::Hash, Word::Compare> words_;
    typedef unordered_set<Meta, Meta::Hash, Meta::Compare> MetaSet;
    MetaSet prefixs_;
    MetaSet suffixs_;
    MetaSet roots_;
};
const Word Server::Null{};

const char* skipAllSpace(const char* p)
{
    while(*p == '\t' || *p == ' ')
    {
        p++;
    }
    return p;
}

Server server;
void test();
int main()
{
    cout << ">>>>>>>>>>>>>>>>>>Cv Vocabulary<<<<<<<<<<<<<<<<<<<<<\n";

    string cmd;
    while (cmd != "q" && cmd != "quit") {
      getline(cin, cmd);
      const char *p = cmd.c_str();
      if (*p == 's') {
            p++;
            p = skipAllSpace(p);
            const char* end = p;
            while(*end != '\0')
            {
                cout << *end;
                end++;
            }
            end = p;
            while(*end != '\n')
            {
                end++;
            }

            string source(p, end);
            cout << "comment: ";
            getline(cin, cmd);
            p = skipAllSpace(cmd.c_str());
            end = p;
            vector<string> comments;
            while(*end != '\0')
            {
                while(*end != '\t' || *end != ' ' || *end != '\n')
                {
                    end++;
                }
                string comment(p, end);
                p = skipAllSpace(end);
                end = p;
                comments.emplace_back(comment);
            }

            cout << "hint:";
            getline(cin, cmd);
            p = skipAllSpace(cmd.c_str());
            end = p;
            while(*end != '\t' || *end != ' ' || *end != '\n')
            {
                end++;
            }
            string hint(p, end);
            Word word(source);
            for(auto& comment : comments)
            {
                word.addComment(comment);
            }
            word.setHint(hint); 
            server.set(word);
            cmd = "";
      }
      else if(*p == 'h')
      {
          cout << "h: 帮助\n";
          cout << "s: 设置单词\n";
          cout << "g: 获取单词\n";
      }
      else if(*p == 'S')
      {
          server.printAll();
      }
    }
    return 0;
}

void test()
{
    Word agent("agent");
    Meta ag(Meta::Type::EROOT, "ag");
    ag.addComment("do or act");
    ag.addComment("do or act2");
    Meta ent(Meta::Type::ESUFFIX, "ent");
    ent.addComment("名词，指人");
    Meta endi(Meta::Type::EROOT, "endi");
    endi.addComment("名词，指物");


    agent.setComponent(0, ag);
    agent.setComponent(1, ent);
    agent.addComment("代理人");
    server.set(agent);
    
    Word agendi("agendi");
    agent.setComponent(0, ag);
    agent.setComponent(1, endi);
    server.set(agendi);
    
    server.printAll();

    const Word& ragent = server.get("agent");
    assert(ragent != Server::Null);
}