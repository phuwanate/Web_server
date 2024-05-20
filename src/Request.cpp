#include "Request.hpp"
#include "ServerBlock.hpp"

Request::Request():_stage(FIRST_LINE) {
}

Request::Request(std::vector<ServerBlock>*	server_blocks):_stage(FIRST_LINE) {
	this->server_blocks = server_blocks;
}

Request::~Request() {
}

HttpStage Request::parseFirstLine(HttpStage stage) {
	_stage = stage;
	std::string buffer;
	std::getline(data, buffer);
	std::istringstream line(buffer);

	line >> buffer;
	if (line.fail()) {
		_stage = RESPONSED;
		_response.byStatus(socket, 400);
		return (_stage);
	}
	method = buffer;
	//TO DO: validate_buffer with allowed buffer //

	line >> buffer;
	if (line.fail()) {
		_stage = RESPONSED;
		_response.byStatus(socket, 400);
		return (_stage);
	}
	path = buffer;

	_stage = HEADER;
	return (_stage);
}

bool Request::isMultipart() {
	std::string value = header["content-type"];

	if (value.empty()) {
		_stage = ROUTER;
		return (false);
	}
	if(value.substr(0, 19) != "multipart/form-data"){
		_stage = RESPONSED;
		_response.byStatus(socket, 422);
		return (false);
	}
	std::size_t has_boundary = value.find("boundary=");
	if (has_boundary != std::string::npos)
		_boundary = "--" + value.substr(has_boundary + 9) + "--\r";
	return (true);
}

bool Request::validBodyLength(){
	ServerBlock server = searchServer(header["Host"], *server_blocks);
	size_t max = server.getClientMaxBodySize();

	if (header["content-length"].length()) {
		size_t length = std::atoi(header["content-length"].c_str());
		if (length > max) {
			_stage = RESPONSED;
			_response.byStatus(socket, 413);
			return (false);
		}
	}
	return (true);
}

HttpStage Request::parseHeader(HttpStage stage) {
	_stage = stage;
	std::string buffer;
	std::getline(data, buffer);

	while(buffer.length() && buffer.c_str()[0] != '\r')
	{
		std::size_t colon = buffer.find_first_of(": ");
		if (colon == std::string::npos) {
			_stage = RESPONSED;
			_response.byStatus(socket, 400);
			return (_stage);
		}
		std::string key = buffer.substr(0, colon);
		std::string value = buffer.substr(colon + 2, buffer.size() - (colon + 2) -1);//skip ": " and trim "\r"
		if (!key.length() || !value.length()) {
			_stage = RESPONSED;
			_response.byStatus(socket, 400);
			return (_stage);
		}
		header[key] = value;
		std::getline(data, buffer);
	}
	if (isMultipart() && validBodyLength())
		_stage = BODY;
	return (_stage);
}

HttpStage Request::parseBody(HttpStage stage) {
	_stage = stage;
	std::string buffer;
	std::stringstream remain_data;
	remain_data << data.rdbuf();
	body << remain_data.str();
	while (std::getline(remain_data, buffer))
	{
		if (buffer == _boundary) {
			_stage = ROUTER;
		}
	}
	return (_stage);
}

void	Request::clear() {
	method = "";
	path = "";
	header.clear();
	body.str(std::string());
	body.clear();
	data.str(std::string());
	data.clear();
	_boundary = "";
}

