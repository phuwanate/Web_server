#include "Response.hpp"

Response::Response(){
	_status = initStatusrMsg();
}

// "default_err_page" file must pass access function and can be read
void Response::byStatus(int socket, int status)
{
	// std::cout << _default_err_page << std::endl;
	// if (status == 404 && _default_err_page.length())
	// {
	// 	return response.byFile(socket, 404, _default_err_page, "text/html");
	// }
	
	std::string firstLine = createFirstLine(status);
	std::string body = createBodyByStatus(status);
	std::string response = createResponse(firstLine, body, "text/html");

	send(socket, response.c_str(), response.size(), 0);
}

void Response::byStringstream(int socket, int status, std::stringstream &file, const std::string &content_type) {
	std::string firstLine = createFirstLine(status);
	std::string response = createResponse(firstLine, file.str(), content_type);

	send(socket, response.c_str(), response.size(), 0);
}

void Response::byFile(int socket, int status, std::string const &location,  std::string const &content_type){
	std::string firstLine = createFirstLine(status);
	Response res;

	std::string body;
	if (!readFile(location, body)) {
		res.byStatus(socket, 404);
	}
	std::string response = createResponse(firstLine, body, content_type);
	send(socket, response.c_str(), response.size(), 0);
}

void Response::byRedirect(int socket, int status, std::string const &location) {
	std::stringstream ss;

	ss << createFirstLine(status) << "\r\n"; 
	ss << "Location: " << location << "\r\n\r\n";

	send(socket, ss.str().c_str(), ss.str().size(), 0);
}

std::string Response::createFirstLine(int status) {
	std::string firstLine = "HTTP/1.1 " + std::to_string(status) + " " + _status[status];
	return (firstLine);
}

std::string Response::createBodyByStatus(int status) {
	std::stringstream body;

	body << "<!DOCTYPE html>" << std::endl;
	body << "<html>" << std::endl;
	body << "<head>" << std::endl;
	body << "<title>" << status << " " << _status[status] << "</title>" << std::endl;
	body << "</head>" << std::endl;
	body << "<body>" << std::endl;
	body << "<h1>" << status << " " << _status[status] << "</h1>" << std::endl;
	body << "</body>" << std::endl;
	body << "</html>" << std::endl;
	return(body.str());
}

std::string Response::createResponse(std::string firstLine, std::string body, std::string contentType) {
	std::stringstream ss;
	
	ss << firstLine << "\r\n";
	ss << "Connection: Keep-Alive\r\n";
	ss << "Keep-Alive: timeout=5, max=20\r\n";
	ss << "Content-Type: " << contentType << "\r\n";
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "\r\n";  // end of headers
	ss << body;
	return(ss.str());
}

bool	Response::readFile(std::string file_location, std::string& buffer){
	std::ifstream file(file_location.c_str());
	std::string    temp_string;

	buffer = "";
	if (!file.is_open())
		return(false);
	while (file)
	{
		std::getline(file, temp_string);
		buffer.append(temp_string);
		buffer.append("\n");
		temp_string = "";
	}
	file.close();
	return (true);
}