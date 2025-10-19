
#include "../webserv.hpp"


class CSVData
{
private:
	DataMap		dataMap;
	std::string	filename;

	Data* strToData(const std::string& line)
	{
		std::stringstream	ss(line);
		std::string			field;
		Data*				d = new Data();

		// Read the name
		std::getline(ss, d->name, ',');

		// Read the age
		std::getline(ss, d->age, ',');

		// Read the comment
		std::getline(ss, d->comment, ',');

		return d;
	}

	std::string dataToStr(const Data& d)
	{
		std::stringstream	ss;
		ss << d.name << "," << d.age << "," << d.comment;
		return ss.str();
	}

	bool CreateFile()
	{
		std::ofstream file(filename.c_str());
		if (!file.is_open())
		{
			std::cerr << "Failed to create the file!" << std::endl;
			return false;
		}
		// Write header
		file << "Id,Name,Age,Comment\n";

		std::cout << "File created with default data." << std::endl;
		file.close();
		return true;
	}

	std::string dataToJSON(const Data& d)
	{
		std::stringstream	ss;

		ss << "{";
		ss << "\"name\": \"" << d.name << "\",";
		ss << "\"age\": " << d.age << ",";
		ss << "\"comment\": " << d.comment;
		ss << "}";
		return ss.str();
	}

public:
	CSVData(const std::string& _filename) : filename(_filename)
	{
		std::ifstream file(_filename.c_str());

		// If the file doesn't exist, create it with some default data
		if (!file.is_open())
		{
			std::cout << "File does not exist. Creating file..." << std::endl;
			CreateFile();
			file.open(_filename.c_str());
		}

		// If the file exists, read the data into the vector
		std::string line;
		bool        HeaderLine = true; // Skip the header
		while (std::getline(file, line))
		{
			if (HeaderLine)
			{
				HeaderLine = false; // Skip the first line (header)
				continue;
			}

			Data* d = strToData(line);
			if (d)
				dataMap[dataMap.size()] = d;
		}

		file.close();
		std::cout << "File read successfully." << std::endl;
	}

	~CSVData()
	{
		for (DataMap::iterator it = dataMap.begin(); it != dataMap.end(); ++it)
			delete it->second;
	}

	std::string getJSONData()
	{
		std::stringstream		ss;
		DataMap::const_iterator	it;

		ss << "[";
		for (it = dataMap.begin(); it != dataMap.end(); ++it)
		{
			ss << dataToJSON(*it->second);
			ss << ",";
		}
		if (!dataMap.empty())
			ss.seekp(-1, ss.cur); // Remove last comma
		ss << "]";
		return ss.str();
	}

	bool addData(const Data& d)
	{
		std::ofstream file(filename.c_str(), std::ios::app);
		if (!file.is_open())
		{
			std::cerr << "Failed to open the file for appending!" << std::endl;
			return false;
		}

		file << d.name << "," << d.age << "," << d.comment << "\n";
		file.close();
		return true;
	}

	bool addData(const std::string& name, const std::string& age, const std::string& comment)
	{
		Data d;
		d.name = name;
		d.age = age;
		d.comment = comment;
		return addData(d);
	}

	Data makeData(const std::string& name, const std::string& age, const std::string& comment)
	{
		Data d;
		d.name = name;
		d.age = age;
		d.comment = comment;
		return d;
	}

	bool removeData(int id)
	{
		std::ifstream fileIn(filename.c_str());
		if (!fileIn.is_open())
		{
			std::cerr << "Failed to open the file for reading!" << std::endl;
			return false;
		}

		std::ofstream fileOut("temp.csv");
		if (!fileOut.is_open())
		{
			std::cerr << "Failed to open the temporary file for writing!" << std::endl;
			fileIn.close();
			return false;
		}

		std::string line;
		int			currentId = 0;
		while (std::getline(fileIn, line))
		{
			if (currentId != id)
				fileOut << line << "\n";
			currentId++;
		}

		fileIn.close();
		fileOut.close();

		// Replace original file with the temporary file
		std::remove(filename.c_str());
		std::rename("temp.csv", filename.c_str());
		return true;
	}

	void printDataFile()
	{
		std::ifstream file(filename.c_str());

		// If the file doesn't exist, create it with some default data
		if (!file.is_open())
		{
			std::cout << "File does not exist. Creating file..." << std::endl;
			return;
		}

		// If the file exists, read the data into the vector
		std::string line;
		int 		counterId = 0;

		while (std::getline(file, line))
		{
			std::stringstream	ss(line);
			Data				d;

			// Read the name
			std::getline(ss, d.name, ',');

			// Read the age
			std::getline(ss, d.age, ',');

			// Read the comment
			std::getline(ss, d.comment, ',');

			std::cout << "ID: " << counterId++
					  << ", Name: " << d.name
					  << ", Age: " << d.age
					  << ", Comment: " << d.comment << std::endl;
		}
		std::cout << std::endl;

		file.close();
	}

	void printDataMap(const DataMap& dataMap)
	{
		for (DataMap::const_iterator it = dataMap.begin(); it != dataMap.end(); ++it)
		{
			const Data& d = *it->second;
			std::cout << "ID: " << it->first
					  << ", Name: " << d.name
					  << ", Age: " << d.age
					  << ", Comment: " << d.comment << std::endl;
		}
	}

};

/*
int main()
{
	CSVData data("data.csv");
	data.printDataFile();

	std::cout << data.getJSONData() << std::endl;

	return 0;
}
*/