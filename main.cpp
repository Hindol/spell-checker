#include "list.h"
#include <algorithm> // Used only to covert all strings to lower-case
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/time.h>


// Anonymous namespace limits the scope of variables to the file (like static)
namespace {

WordList dictWords;
WordList misspeltWords;
std::ifstream dictionary("DICTIONARY");
std::ifstream input("ARTICLE");
pthread_mutex_t inputMutex;

struct ThreadArgs
{
    int id;
};

void * SpellCheck(void * arg)
{
    ThreadArgs threadArgs = *static_cast<ThreadArgs *>( arg );
    const std::string ID = static_cast<std::ostringstream*>( &(std::ostringstream() << threadArgs.id) )->str();
    const int CHUNK_SIZE = 25;
    std::string word[CHUNK_SIZE];

    while (true)
    {
        pthread_mutex_lock(&inputMutex);
        if (!input.good())
        {
            pthread_mutex_unlock(&inputMutex);
            break;
        }

        for (int i = 0; (i < CHUNK_SIZE) && input.good(); ++i)
        {
            input >> word[i];
        }
        pthread_mutex_unlock(&inputMutex);

        for (int i = 0; i < CHUNK_SIZE; ++i)
        {
            // Change to lower case, remove punctuation etc.
            std::string canonicalWord;
            transform(word[i].begin(), word[i].end(), back_inserter(canonicalWord), ::tolower);
            // TODO: Remove punctuation

            if (!dictWords.Contains(canonicalWord))
            {
                misspeltWords.AppendIfUnique(word[i]);
            }
        }
    }

    return 0L;
}

void SpawnSpellCheckerThreads(const int NUM_THREADS, pthread_t threads[], ThreadArgs threadArgs[])
{
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadArgs[i].id = i;
        pthread_create(&threads[i], NULL, SpellCheck, static_cast<void *>(&threadArgs[i]));
    }
}

void JoinSpellCheckerThreads(const int NUM_THREADS, pthread_t threads[])
{
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], NULL);
    }
}

timespec timediff(const timespec &begin, const timespec &end)
{
    timespec diff;
    if ((end.tv_nsec - begin.tv_nsec) < 0)
    {
        diff.tv_sec = end.tv_sec - begin.tv_sec - 1;
        diff.tv_nsec = 1000000000 + end.tv_nsec - begin.tv_nsec;
    } else {
        diff.tv_sec = end.tv_sec - begin.tv_sec;
        diff.tv_nsec = end.tv_nsec - begin.tv_nsec;
    }
    return diff;
}

}


int main()
{
    using namespace std;

    // How many worker threads?
    const int NUM_THREADS = 4;

    timespec begin;
    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);

    // Read list of correctly spelt words
    string word;
    while (dictionary.good())
    {
        dictionary >> word;
        dictWords.Append(word);
    }

    pthread_t threads[NUM_THREADS];
    ThreadArgs threadArgs[NUM_THREADS];
    pthread_mutex_init(&inputMutex, 0L);

    SpawnSpellCheckerThreads(NUM_THREADS - 1, threads, threadArgs);

    // Main thread also takes part in spell-checking
    threadArgs[NUM_THREADS - 1].id = NUM_THREADS - 1;
    SpellCheck(&threadArgs[NUM_THREADS - 1]);

    JoinSpellCheckerThreads(NUM_THREADS - 1, threads);

    ofstream output("MISSPELT_WORDS");
    output << misspeltWords;

    timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    cout << "Using " << NUM_THREADS << " thread(s)." << endl;
    cout << "Total time: " << timediff(begin, end).tv_sec << " s " <<
            timediff(begin, end).tv_nsec << " ns" << endl;

    pthread_mutex_destroy(&inputMutex);
    return 0;
}

