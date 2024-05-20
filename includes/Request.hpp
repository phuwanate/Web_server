#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include "mock.hpp"
#include "HttpStatus.hpp"
#include "Response.hpp"

#define	DEFAULT	"\033[0m"
#define	RED		"\033[31m"
#define	GREEN	"\033[32m"
#define	YELLOW	"\033[33m"
#define	WHITE	"\x1B[37m"
#define	BLUE	"\x1B[94m"
#define	CYAN	"\x1B[36m"
#define	MAGENTA	"\x1B[95m"

class ServerBlock;
class Response;

class Request {
	public:
		Request();
		Request(std::vector<ServerBlock>*	server_blocks);
		~Request();

		std::string							method;
		std::string							path;
		std::map<std::string, std::string>	header; 
		std::stringstream					body; //use by first
		int									sever_socket; //set by client
		std::vector<ServerBlock>*			server_blocks; // set by serverhandler

		HttpStage							parseFirstLine(HttpStage stage); //use by client
		HttpStage							parseHeader(HttpStage stage); //use by client
		HttpStage							parseBody(HttpStage stage); //use by client
		void								clear();
		std::stringstream					data; //set by client
		int									socket; //set by client

	private:
		HttpStage							_stage;
		std::string							_boundary;
		Response 							_response;

		bool								isMultipart();
		bool								validBodyLength();
};

#endif