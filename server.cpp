#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <strings.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <sstream>

using namespace std;

void fireman(int)
{
   while (waitpid(-1, NULL, WNOHANG) > 0)
      ;
}

string GetEncodedMessage(vector <tuple<char, int, string>>& alphabet, string unencodedMessage)
{
    string encodedMessage = "";

    for (char q : unencodedMessage)
    {
        vector<tuple<char, int, string>>::iterator it = find_if(alphabet.begin(), alphabet.end(), [=](const std::tuple<char, int, string>& c) {return get<0>(c) == q; });

        encodedMessage += get<2>(*it);

    }

    return encodedMessage;
}

string DecimalToBinary(double num, int k_prec)
{
    string binary = "";

    // Fetch the integral part of decimal number 
    int Integral = num;

    // Fetch the fractional part decimal number 
    double fractional = num - Integral;

    // Conversion of integral part to 
    // binary equivalent 
    while (Integral)
    {
        int rem = Integral % 2;

        // Append 0 in binary 
        binary.push_back(rem + '0');

        Integral /= 2;
    }

    // Reverse string to get original binary 
    // equivalent 
    reverse(binary.begin(), binary.end());

    // Append point before conversion of 
    // fractional part 
    //binary.push_back('.'); 

    // Conversion of fractional part to 
    // binary equivalent 
    while (k_prec--)
    {
        // Find next bit in fraction 
        fractional *= 2;
        int fract_bit = fractional;

        if (fract_bit == 1)
        {
            fractional -= fract_bit;
            binary.push_back(1 + '0');
        }
        else
            binary.push_back(0 + '0');
    }

    return binary;
}

double DetermineNumberOfBits(double probability)
{
    double result = -log2(probability);

    return result;
}

vector<string> GetConvertedBinaryNums(vector<double> probabilities, int alphabetSize, vector<double> unconvertedBinaryNums)
{
    vector<string> convertedBinaryNums;

    for (int j = 0; j < alphabetSize; j++)
    {
        double numOfBits;

        numOfBits = ceil(fabs(DetermineNumberOfBits(probabilities[j])));

        string conv = DecimalToBinary(unconvertedBinaryNums[j], numOfBits);

        convertedBinaryNums.push_back(conv);
    }

    return convertedBinaryNums;
}

vector<double> GetUnconvertedBinaryNums(vector<double> probabilities, int alphabetSize)
{
    vector<double> unconvertedBinaryNums;

    unconvertedBinaryNums.push_back(0.0);

    double accumulator = 0.0;

    for (int i = 0; i < alphabetSize - 1; i++)
    {
        accumulator += probabilities[i];

        unconvertedBinaryNums.push_back(accumulator);
    }

    return unconvertedBinaryNums;
}

vector<double> GetProbabilities(vector <tuple<char, int, string>> alphabet, int messageSize)
{
    vector<double> probabilities;

    for (int i = 0; i < alphabet.size(); i++)
    {
        probabilities.push_back((double)get<1>(alphabet[i]) / messageSize);
    }

    return  probabilities;
}

void GenerateCodes(vector <tuple<char, int, string>>& alphabet, int messageSize)
{
    vector<double> probabilities = GetProbabilities(alphabet, messageSize);

    vector<double> unconvertedBinaryNums = GetUnconvertedBinaryNums(probabilities, alphabet.size());

    vector<string> convertedBinaryNums = GetConvertedBinaryNums(probabilities, alphabet.size(), unconvertedBinaryNums);

    for (int k = 0; k < alphabet.size(); k++)
    {
        get<2>(alphabet[k]) = convertedBinaryNums[k];
    }
}

void CountFrequency(string unencodedMessage, vector<tuple<char, int, string>>& alphabet)
{
    vector<char> alreadyCounted;
    vector<int> frequencyCounts;

    for (auto i : unencodedMessage)
    {
        if (alreadyCounted.empty())
            alreadyCounted.push_back(i);
        else if (find(alreadyCounted.begin(), alreadyCounted.end(), i) == alreadyCounted.end())
            alreadyCounted.push_back(i);
        else  if (find(alreadyCounted.begin(), alreadyCounted.end(), i) != alreadyCounted.end())
            continue;

        int frequency = count(unencodedMessage.begin(), unencodedMessage.end(), i);
        alphabet.push_back(make_tuple(i, frequency, ""));
    }

    sort(alphabet.begin(), alphabet.end(),
        [](const tuple<char, int, string>& p1, const tuple<char, int, string>& p2)
        {
            if (get<1>(p1) != get<1>(p2))
                return get<1>(p1) > get<1>(p2);
            // Then sort by symbol in alphabetical order
            if (get<0>(p1) != get<0>(p2))
                return get<0>(p1) > get<0>(p2);
            return false;
        });
}

vector<tuple<char, int, string>> GetAlphabet(string unencodedMessage)
{
    vector<tuple<char, int, string>> alphabet;

    CountFrequency(unencodedMessage, alphabet);
    GenerateCodes(alphabet, unencodedMessage.size());

    return alphabet;
}

string PerformShannonCoding(string unencodedMessage)
{
    auto alphabet = GetAlphabet(unencodedMessage);
    return GetEncodedMessage(alphabet, unencodedMessage);
}

int main(int argc, char *argv[])
{
   int sockfd, newsockfd, portno, clilen;
   struct sockaddr_in serv_addr, cli_addr;

   // Check the commandline arguments
   if (argc != 2)
   {
      std::cerr << "Port not provided" << std::endl;
      exit(0);
   }

   // Create the socket
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
   {
      std::cerr << "Error opening socket" << std::endl;
      exit(0);
   }

   // Populate the sockaddr_in structure
   bzero((char *)&serv_addr, sizeof(serv_addr));
   portno = atoi(argv[1]);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);

   // Bind the socket with the sockaddr_in structure
   if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
   {
      std::cerr << "Error binding" << std::endl;
      exit(0);
   }

   // Set the max number of concurrent connections
   listen(sockfd, 5);
   clilen = sizeof(cli_addr);

   // Accept a new connection
   signal(SIGCHLD, fireman);

   while (true)
   {
      newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
      if (newsockfd < 0)
         {
            std::cerr << "Error accepting new connections" << std::endl;
            exit(0);
         }

      if (fork() == 0)
      {
         
         int n, msgSize = 0;
         n = read(newsockfd, &msgSize, sizeof(int));
         if (n < 0)
         {
            std::cerr << "Error reading from socket" << std::endl;
            exit(0);
         }
         char *tempBuffer = new char[msgSize + 1];
         bzero(tempBuffer, msgSize + 1);
         n = read(newsockfd, tempBuffer, msgSize + 1);
         if (n < 0)
         {
            std::cerr << "Error reading from socket" << std::endl;
            exit(0);
         }
         
         std::string buffer = tempBuffer;
         delete[] tempBuffer;
         auto alphabet = GetAlphabet(buffer);
         buffer  = PerformShannonCoding(buffer);

         //buffer = "I got your message";
         msgSize = buffer.size();
         n = write(newsockfd, &msgSize, sizeof(int));

         if (n < 0)
         {
            std::cerr << "Error writing to socket" << std::endl;
            exit(0);
         }
         n = write(newsockfd, buffer.c_str(), msgSize);
         if (n < 0)
         {
            std::cerr << "Error writing to socket" << std::endl;
            exit(0);
         }

        stringstream ss;

        ss << alphabet.size() << '\n';

        for (const auto& item : alphabet)
        {

            char symbol = get<0>(item);
            int frequency = get<1>(item);
            const string& code = get<2>(item);

            // we mark whitespace with '|' so stringstream properly captures ' ' as input
            if (get<0>(item) == ' ')
                symbol = '|';

            ss << symbol << ' '
               << frequency << ' '
               << code << '\n';
        }

        string serializedAlphabet = ss.str();

        int dataSize = serializedAlphabet.size();

        n = write(newsockfd, &dataSize, sizeof(int));
        if (n < 0)
        {
            std::cerr << "Error writing data size to socket" << std::endl;
            exit(1);
        }

        // Send the serialized string
        n = write(newsockfd, serializedAlphabet.c_str(), dataSize);
        if (n < 0)
        {
            std::cerr << "Error writing serialized data to socket" << std::endl;
            exit(1);
        }

         close(newsockfd);
         _exit(0);
      }
      wait(nullptr);
   }

   close(newsockfd);
   close(sockfd);
   return 0;
}
