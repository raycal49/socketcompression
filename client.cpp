#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <strings.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <sstream>

using namespace std;

struct threadData
{
    string unencodedMessage;
    vector<tuple<char, int, string>> alphabet;
    string encodedMessage;
    char* serverIP;
    int portno;

    threadData(string m, char* IP, int portNumber)
    {
        unencodedMessage = m;
        serverIP = IP;
        portno = portNumber;
    }
};

struct dataToPass
{
    string unencodedMessage;
    string encodedMessage;
    vector<tuple<char, int, string>> alphabet;
};

vector<string> GetUnencodedMessages()
{
    vector<string> unencodedMessages;
    string input;

    while (getline(cin, input))
    {
        if (input.empty())
            break;

        unencodedMessages.push_back(input);
    }

    return unencodedMessages;
}

void PrintAlphabet(vector <tuple<char, int, string>> alphabet)
{
    cout << "Alphabet: " << endl;

    for (int m = 0; m < alphabet.size(); m++)
    {
        cout << "Symbol: " << get<0>(alphabet[m]) << ", ";

        cout << "Frequency: " << get<1>(alphabet[m]) << ", ";

        cout << "Shannon code: " << get<2>(alphabet[m]) << endl;

    }

    cout << endl;
}

void PrintThreadInfo(threadData* d)
{
    cout << "Message: " << d->unencodedMessage << endl << endl;
    PrintAlphabet(d->alphabet);
    cout << "Encoded message: " << d->encodedMessage << endl;
}

void* threadInstructions(void* castedThreadData)
    {
        threadData* data = (threadData*) castedThreadData;
    
        int sockfd, portno, n;
        struct sockaddr_in serv_addr;
        struct hostent *server;
    
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
        {
            std::cerr << "ERROR opening socket" << std::endl;
            exit(0);
        }
        server = gethostbyname(data->serverIP);
        if (server == NULL) {
            std::cerr << "ERROR, no such host" << std::endl;
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
        serv_addr.sin_port = htons(data->portno);
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        {
            std::cerr << "ERROR connecting" << std::endl;
            exit(0);
        }
        
        // pass unencoded message to server for processing
        std::string buffer = data->unencodedMessage;
        
        int msgSize = sizeof(buffer);
        n = write(sockfd,&msgSize,sizeof(int));
        if (n < 0) 
        {
            std::cerr << "ERROR writing to socket" << std::endl;
            exit(0);
        }
        n = write(sockfd,buffer.c_str(),msgSize);
        if (n < 0) 
        {
            std::cerr << "ERROR writing to socket" << std::endl;
            exit(0);
        }
        n = read(sockfd,&msgSize,sizeof(int));
        if (n < 0) 
        {
            std::cerr << "ERROR reading from socket" << std::endl;
            exit(0);
        }
        
        // retrieve encoded message from server and place into our ThreadData struct
        char *tempBuffer = new char[msgSize+1];
        bzero(tempBuffer,msgSize+1);
        n = read(sockfd,tempBuffer,msgSize);
        if (n < 0) 
        {
            std::cerr << "ERROR reading from socket" << std::endl;
            exit(0);
        }
        buffer = tempBuffer;
        data->encodedMessage = buffer;
        delete [] tempBuffer;

        // retrieve alphabet data from server as a string
        n = read(sockfd, &msgSize, sizeof(int));
        if (n < 0)
        {
            std:cerr << "ERROR reading from socket" << std::endl;
            exit(0);
        }
        tempBuffer = new char[msgSize+1];
        bzero(tempBuffer,msgSize+1);
        n = read(sockfd,tempBuffer,msgSize);
        if (n < 0) 
        {
            std::cerr << "ERROR reading from socket" << std::endl;
            exit(0);
        }
        buffer = tempBuffer;
        delete [] tempBuffer;

        stringstream ss(buffer);

        int alphabetSize;
        
        // process alphabet string and format into vector<char, int, string>  form
        ss >> alphabetSize;

        for (int i = 0; i < alphabetSize; i++)
        {
            char symbol;
            int frequency;
            string code;

            ss >> symbol >> frequency >> code;

            // convert all '|' to ' ' so we properly display ' ' symbol and its frequency
            if (symbol == '|')
                symbol = ' ';

            data->alphabet.push_back(make_tuple(symbol, frequency, code));
        }

        close(sockfd);

        return NULL;
    }

int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
       std::cerr << "usage " << argv[0] << " hostname port" << std::endl;
       exit(0);
    }

    vector<string> unencodedMessages = GetUnencodedMessages();

    vector<pthread_t> threadVector;
    vector<threadData*> threadDataVector;

    for (int i = 0; i < unencodedMessages.size(); i++)
    {
        pthread_t newThread;
        threadData* newThreadData = new threadData(unencodedMessages[i], argv[1], atoi(argv[2]));

        if (pthread_create(&newThread, NULL, threadInstructions, static_cast<void*> (newThreadData)))
        {
            cout << "Error creating thread";
            return -1;
        }

        threadVector.push_back(newThread);
        threadDataVector.push_back(newThreadData);
    }

    int inputAmount = threadDataVector.size();

    for (int j = 0; j < inputAmount; j++)
    {
        pthread_join(threadVector[j], NULL);
    }

    for (int k = 0; k < inputAmount; k++)
    {
        PrintThreadInfo(threadDataVector[k]);
        if (k != inputAmount - 1)
            cout << endl;
    }
    
    return 0;
}
