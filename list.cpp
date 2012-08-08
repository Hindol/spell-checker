#include "list.h"


std::ostream & operator <<(std::ostream &os, const WordList &wordList)
{
    pthread_rwlock_rdlock(&wordList.rwlock);

    WordList::Node *iter = wordList.head;
    while (iter != 0L)
    {
        os << iter->word << '\n';
        iter = iter->next;
    }

    pthread_rwlock_unlock(&wordList.rwlock);

    return os;
}


WordList::WordList()
    : head(0L), tail(0L)
{
    // Note: http://stackoverflow.com/questions/2190090/\
    //  how-to-prevent-writer-starvation-in-a-read-write-lock-in-pthreads
    pthread_rwlockattr_t rwattr;

    // Commenting this out gives better timings, so...
    // pthread_rwlockattr_setkind_np(&rwattr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);

    pthread_rwlock_init(&rwlock, &rwattr);
}


/**
 * Note: http://stackoverflow.com/questions/11652748/\
 *  locking-shared-resources-in-constructor-and-destructor
 */
WordList::~WordList()
{
    while (head != 0L)
    {
        // Re-using tail here since it is no longer required
        tail = head->next;
        delete head;
        head = tail;
    }

    pthread_rwlock_destroy(&rwlock);
}


bool WordList::Contains(const std::string &word) const
{
    pthread_rwlock_rdlock(&rwlock);

    Node *iter = head;
    bool found = false;
    while (iter != 0L)
    {
        if (iter->word == word)
        {
            found = true;
            break;
        }
        iter = iter->next;
    }

    pthread_rwlock_unlock(&rwlock);
    return found;
}


void WordList::Append(const std::string &word)
{
    pthread_rwlock_wrlock(&rwlock);

    if (tail != 0L)
    {
        tail->next = new Node();
        tail->next->word = word;
        tail->next->next = 0L;
        tail = tail->next;
    }
    else
    {
        head = tail = new Node();
        tail->word = word;
    }

    pthread_rwlock_unlock(&rwlock);
}


void WordList::AppendIfUnique(const std::string &word)
{
    pthread_rwlock_rdlock(&rwlock);

    if (tail != 0L) // List contains at least one element
    {
        Node *iter = head;
        bool notFound = true;

        while (iter != tail) // Do not move past tail
        {
            if (iter->word == word)
            {
                notFound = false;
                break;
            }
            iter = iter->next;
        }

        // Now iter is equal to tail
        if (iter->word == word) notFound = false;

        if (notFound)
        {
            // Release read lock and acquire write lock
            // Note: Some other thread might write in between
            pthread_rwlock_unlock(&rwlock);
            pthread_rwlock_wrlock(&rwlock);

            // Again check if the word is already inserted
            iter = iter->next;
            while (iter != 0L)
            {
                if (iter->word == word)
                {
                    notFound = false;
                    break;
                }
                iter = iter->next;
            }

            if (notFound)
            {
                tail->next = new Node();
                tail->next->word = word;
                tail->next->next = 0L;
                tail = tail->next;
            }
        }
    }
    else
    {
        // Release read lock and acquire write lock
        // Note: Some other thread might write in between
        pthread_rwlock_unlock(&rwlock);
        pthread_rwlock_wrlock(&rwlock);

        if (tail == 0L) // If the tail is still null, insert
        {
            head = tail = new Node();
            tail->word = word;
        }
        else
        {
            Node *iter = head;
            bool notFound = true;
            while (iter != 0L)
            {
                if (iter->word == word)
                {
                    notFound = false;
                    break;
                }
                iter = iter->next;
            }

            if (notFound)
            {
                tail->next = new Node();
                tail->next->word = word;
                tail->next->next = 0L;
                tail = tail->next;
            }
        }
    }

    pthread_rwlock_unlock(&rwlock);
}
