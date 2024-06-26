#include "utils.hpp"

std::string	getBlock(std::string const &content, std::string const &needle, bool skipKeyword) {
	int bracesCount = -1;
	size_t index, start;
	std::string targetBlock;

	index = content.find(needle);
	if (index == std::string::npos)
		return targetBlock;
	
	index += needle.length();
	for (; index < content.length(); index++) {
		if (content[index] == '{') {
			skipKeyword == true ? start = index : start = 0;
			bracesCount = 1;
			index++;
			break;
		}
	}

	for (; index < content.length() && bracesCount > 0; index++) {
		if (content[index] == '{')
			bracesCount++;
		else if (content[index] == '}')
			bracesCount--;
	}
	if (bracesCount == 0)
		targetBlock = content.substr(start, index - start);
	return targetBlock;
}

size_t	findFirstBrace(std::string const &content, std::string const &needle) {
	size_t index = content.find(needle);
	if (index == std::string::npos)
		return 0;
	index += needle.length();
	while (index < content.length() && content[index] != '{')
		index++;
	return index;
}

bool	isServerConflict(ServerBlock newInstance, ServerBlock oldInstance) {
	if ( (newInstance.getHostIP() == oldInstance.getHostIP()) && \
	 (newInstance.getServerName() == oldInstance.getServerName()) &&
	 (duplicate_port(newInstance.getPortNumb(), oldInstance.getPortNumb()))) {
	 	return true;
	}
	return false;
}

bool duplicate_port(std::vector<size_t> newPort, std::vector<size_t> oldPort) {
	std::vector<size_t>::iterator n_it = newPort.begin();
	for(; n_it != newPort.end(); n_it++) {
		for (std::vector<size_t>::iterator o_it = oldPort.begin();o_it != oldPort.end(); o_it++) {
			if (*n_it == *o_it) {
				std::cout << RED << "Error: found duplicating servers block [" << *n_it << "]";
				std::cout <<  ", [ingnore]" << DEFAULT<< std::endl;
				return true;
			}
		}
	}
	return false;
}

void	isLocationDuplicate(LocationBlock newInstance, LocationBlock oldInstance) {
	if (newInstance.getDirectoryPath() == oldInstance.getDirectoryPath())
		throw std::string ("Error: duplicate location \"" + newInstance.getDirectoryPath() + "\" in location block.");
}

std::string searchTarget(std::string const &content) {
	size_t      index = 0;
	std::string target;

	if (content[0] == '{' || content[0] == '}' || content[0] == ';')
		return content.substr(0, 1);
	for (; index < content.length(); index++) {

		if (isWhiteSpace(content[index]) == true)
			return content.substr(0, index); //for retrive directive keywords (listen, index, etc.)
		switch (content[index]){

			case '{':
				return content.substr(0, index);
			case '}':
				return content.substr(0, index);
			case ';':
				return content.substr(0, index);
		}
	}
	return content.substr(0, index);
}

bool	isWhiteSpace(char target) {
	if (target == ' ' || target == '\t' || target == '\n' || target == '\r' || target == '\v')
		return true;
	return false;
}

bool	isDigit(std::string values) {
	for (std::string::iterator it = values.begin(); it != values.end(); it++) {
		if (std::isdigit(*it) == false)
			return false;
	}
	return true;
}

void	validateHostIP(std::string value) {
	int validateRange;
	std::string tmp = value;
	int count = 0;
	while (true) {
		// std::cout << "here" << std::endl;
		size_t pos = value.find(".");
		if (pos == std::string::npos){
			if (value == "" || isDigit(value) == false)
				throw std::string ("Error: invalid host number \"" + value + "\" in host directive.");
			validateRange = convertString<int>(value);
			if (validateRange < 0 || validateRange > 255)
				throw std::string ("Error: host number \"" + value + "\" is out of bound.");
			break;
		}
		std::string target = value.substr(0, pos);
		if (target == "" || isDigit(target) == false)
			throw std::string ("Error: invalid host number \"" + value + "\" in host directive.");
		value = value.substr(pos + 1, (value.length()) - (pos + 1));
		validateRange = convertString<int>(target);
		if (validateRange < 0 || validateRange > 255)
			throw std::string ("Error: host number \"" + value + "\" is out of bound.");
		count++;
	}
	if (count != 3)
		throw std::string ("Error: invalid host number \"" + tmp + "\" in host directive.");
}

std::string   splitString(std::string &needToSplit, std::string const &delimeter) {
	std::size_t pos = needToSplit.find(delimeter);
	std::string result;

	if (pos == std::string::npos)
		return "";
	result = needToSplit.substr(0, pos);
	needToSplit = needToSplit.substr(pos + delimeter.length(), needToSplit.length());
	return result;
}

unsigned long hostIPToNetworkByteOrder(std::string const &hostIP) {
	std::string needToConvert = hostIP;
	int ipStorage[3];//IPV4
	unsigned long result;

	for (int index = 0; index < 3; index++)
		ipStorage[index] = convertString<unsigned long>(splitString(needToConvert, "."));
	
	result = convertString<unsigned long>(needToConvert); //for the last bit in IP address.
	for (int index = 2; index >= 0; index --)
		result = (result * 256) + ipStorage[index];
	return result;
}

bool	checkFileExists(std::string File) {
	std::ifstream inputStream(File.c_str());

	return  inputStream.good();
}

std::string	errNumberOfParameters(std::string const &directive, std::string const &block) {
	return "Error: [" + block + "] invalid numbers of parameters \"" + directive + "\" directive";
}

std::string	ftToupper(std::string src) {
	std::string result;
	
	for (size_t index = 0; index < src.length(); index++) {
		result += toupper(src[index]);
	}
	return result;
}

template	<typename T>
T   convertString(std::string const &needToConvert) {

	std::stringstream   stream(needToConvert);
	T                   result;

	if (isDigit(needToConvert) == false)
		return false;

	stream >> result;
	return (result);
}

ServerBlock	searchServer(std::string serverName, std::vector<ServerBlock> s_vec) {
	std::vector<ServerBlock>::iterator s_it;
	std::vector<std::string>::iterator f_it;
	ServerBlock s_block;

	// std::cout << "ServerName: " << s_vec[0].getFullName()[0] << std::endl;


	for (s_it = s_vec.begin(); s_it != s_vec.end(); s_it++) {
		std::vector<std::string> f_vec = s_it->getFullName();
		f_it = f_vec.begin();
		for (; f_it != f_vec.end(); f_it++) {
			// std::cout << "FullName: " << *f_it << std::endl;
			if (*f_it == serverName)
				return *s_it;
		}
	}
	// if (s_it == s_vec.end())
	// 	std::cerr << "Error: Invalid server name: " << serverName << std::endl;
	// std::cout << "Client use invalid servername." << std::endl;
	return s_block;
}

ServerBlock	searchServerByPort(size_t serverPort, std::vector<ServerBlock> s_vec) {
	std::vector<ServerBlock>::iterator s_it;
	std::vector<size_t>::iterator p_it;
	ServerBlock s_block;

	for (s_it = s_vec.begin(); s_it != s_vec.end(); s_it++) {
		for (p_it = s_it->getPortNumb().begin(); p_it != s_it->getPortNumb().end(); p_it++)
			if (*p_it == serverPort)
				return *s_it;
	}
	if (s_it == s_vec.end())
		std::cerr << "Error: Invalid server port" << std::endl;
	return s_block;
}

LocationBlock	searchLocation(std::string serverName, std::string directoryPath, std::vector<ServerBlock> s_vec) {
	std::vector<LocationBlock> l_vec;
	std::vector<LocationBlock>::iterator l_it;
	std::vector<ServerBlock>::iterator s_it;
	std::vector<std::string>::iterator f_it;
	LocationBlock  l_block;

	for (s_it = s_vec.begin(); s_it != s_vec.end(); s_it++) {
		std::vector<std::string> f_vec = s_it->getFullName();
		f_it = f_vec.begin();
		for (; f_it != f_vec.end(); f_it++) {
			if (*f_it == serverName) {
				l_vec = s_it->getLocationBlocks();
				
				for (l_it = l_vec.begin(); l_it != l_vec.end(); l_it++) {
					if (l_it->getDirectoryPath() == directoryPath) {
						return *l_it;
					}
				}
			}
		}
	}
	// if (s_it == s_vec.end())
	// 	std::cerr << "Error: Invalid server name: " << serverName << std::endl;
	return l_block;
}

void	*ft_memset(void *s, int src, size_t n){
	unsigned char	*dest;

	dest = (unsigned char *)s;
	while (n--)
		*dest++ = (unsigned char)src;
	return (s);
}

void	*ft_memcpy(void *dst, const void *src, size_t n){
	char	*s;
	char	*d;

	s = (char *)src;
	d = (char *)dst;
	if (src != NULL || dst != NULL)
	{	
		while (n--)
		{
			*d++ = *s++;
		}
	}
	return (dst);
}

bool isDirective(const std::string &key) {

	if (key == "listen" || key == "host" || key == "server_name" || key == "root" || key == "client_max_body_size" \
	|| key == "index" || key == "autoindex" || key == "error_page" || key == "location" || key == "server" \
	|| key == "alias" || key == "allow_http_methods" || key == "return" || key == "cgi_extensions" \
	|| key == "cgi_compiler_path") {
		return true;	
	}
	return false;
}

// template int ftConvert<int>(std::string const &needToConvert);
// template unsigned long ftConvert<unsigned long>(std::string const &needToConvert);
