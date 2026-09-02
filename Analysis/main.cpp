#include <cctype>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

void AnalyzeMissE(const char* inputFileName,
                  const char* outputFileName,
                  double promptMin,
                  double promptMax,
                  double randomMin,
                  double randomMax,
                  double random2Min,
                  double random2Max);

namespace
{
std::string Trim(const std::string& value)
{
  const std::string whitespace = " \t\r\n";
  const std::string::size_type first = value.find_first_not_of(whitespace);
  if(first == std::string::npos)
    return "";
  const std::string::size_type last = value.find_last_not_of(whitespace);
  return value.substr(first, last - first + 1);
}

std::string InputNumber(const std::string& path)
{
  const std::string::size_type slash = path.find_last_of("/\\");
  std::string name = path.substr(slash == std::string::npos ? 0 : slash + 1);
  const std::string::size_type dot = name.find_last_of('.');
  if(dot != std::string::npos)
    name.erase(dot);

  std::string::size_type end = name.size();
  std::string::size_type begin = end;
  while(begin > 0 && std::isdigit(static_cast<unsigned char>(name[begin - 1])))
    --begin;
  if(begin == end)
    return "";
  return name.substr(begin, end - begin);
}

bool ReadParameter(const std::string& line, std::string& key, double& value)
{
  std::string normalized = line;
  for(std::string::size_type i = 0; i < normalized.size(); ++i)
    if(normalized[i] == '=' || normalized[i] == ':')
      normalized[i] = ' ';

  std::istringstream input(normalized);
  std::string extra;
  if(!(input >> key >> value) || (input >> extra))
    return false;
  return key == "prompt_min" || key == "prompt_max" ||
         key == "random_min" || key == "random_max" ||
         key == "random2_min" || key == "random2_max";
}
}

int main(int argc, char** argv)
{
  const std::string listName = argc > 1 ? argv[1] : "input.dat";
  std::ifstream inputList(listName.c_str());
  if(!inputList)
    {
      std::cerr << "Cannot open input list: " << listName << std::endl;
      return 1;
    }

  std::set<std::string> usedNumbers;
  std::vector<std::string> inputNames;
  double promptMin = 0.0;
  double promptMax = 0.0;
  double randomMin = 0.0;
  double randomMax = 0.0;
  double random2Min = 0.0;
  double random2Max = 0.0;
  bool havePromptMin = false;
  bool havePromptMax = false;
  bool haveRandomMin = false;
  bool haveRandomMax = false;
  bool haveRandom2Min = false;
  bool haveRandom2Max = false;
  std::string line;
  unsigned int lineNumber = 0;
  while(std::getline(inputList, line))
    {
      ++lineNumber;
      const std::string item = Trim(line);
      if(item.empty() || item[0] == '#')
        continue;

      std::string key;
      double value = 0.0;
      if(ReadParameter(item, key, value))
        {
          if(key == "prompt_min") { promptMin = value; havePromptMin = true; }
          else if(key == "prompt_max") { promptMax = value; havePromptMax = true; }
          else if(key == "random_min") { randomMin = value; haveRandomMin = true; }
          else if(key == "random_max") { randomMax = value; haveRandomMax = true; }
          else if(key == "random2_min") { random2Min = value; haveRandom2Min = true; }
          else if(key == "random2_max") { random2Max = value; haveRandom2Max = true; }
          continue;
        }

      const std::string number = InputNumber(item);
      if(number.empty())
        {
          std::cerr << listName << ':' << lineNumber
                    << ": invalid parameter or no trailing input number in '"
                    << item << "'"
                    << std::endl;
          return 1;
        }
      if(!usedNumbers.insert(number).second)
        {
          std::cerr << listName << ':' << lineNumber
                    << ": duplicate input number " << number << std::endl;
          return 1;
        }
      inputNames.push_back(item);
    }

  if(!havePromptMin || !havePromptMax || !haveRandomMin || !haveRandomMax ||
     !haveRandom2Min || !haveRandom2Max)
    {
      std::cerr << listName << ": prompt_min, prompt_max, random_min and "
                   "random_max, random2_min and random2_max must all be "
                   "specified" << std::endl;
      return 1;
    }
  if(promptMin >= promptMax || randomMin >= randomMax ||
     random2Min >= random2Max)
    {
      std::cerr << listName << ": each time window must satisfy min < max"
                << std::endl;
      return 1;
    }
  if((promptMin < randomMax && randomMin < promptMax) ||
     (promptMin < random2Max && random2Min < promptMax) ||
     (randomMin < random2Max && random2Min < randomMax))
    {
      std::cerr << listName << ": time windows overlap"
                << std::endl;
      return 1;
    }
  if(inputNames.empty())
    {
      std::cerr << "No input ROOT files found in " << listName << std::endl;
      return 1;
    }

  for(std::vector<std::string>::const_iterator inputName = inputNames.begin();
      inputName != inputNames.end(); ++inputName)
    {
      const std::string number = InputNumber(*inputName);
      const std::string outputName = "Analyse_" + number + ".root";
      std::cout << "Analyzing " << *inputName << " -> " << outputName
                << std::endl;
      AnalyzeMissE(inputName->c_str(), outputName.c_str(),
                   promptMin, promptMax, randomMin, randomMax,
                   random2Min, random2Max);
    }

  std::cout << "Processed " << inputNames.size() << " input file(s)"
            << std::endl;
  return 0;
}
