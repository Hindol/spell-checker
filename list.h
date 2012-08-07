#ifndef LIST_H
#define LIST_H


#include <pthread.h>
#include <string>
#include <ostream>


/**
 * An overly simplified thread-safe linked list for storing words.
 */
class WordList
{
    friend std::ostream& operator<<(std::ostream& os, const WordList &wordList);

public:
    WordList();
    ~WordList();
    bool Contains(const std::string &word) const;
    void Append(const std::string &word);
    void AppendIfUnique(const std::string &word);

private:
    struct Node
    {
        Node *next;
        std::string word;
    };

    Node *head;
    Node* tail;
    mutable pthread_rwlock_t rwlock;
};

#endif // LIST_H
