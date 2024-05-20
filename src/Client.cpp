#include "Client.hpp"
#include "utils.hpp"

Client::Client(){}

Client::Client(Client const &inst) {
	*this = inst;
}

Client::Client(int new_sd, int listen_sd, std::vector<ServerBlock>  serverBlocks){
	this->_socket = new_sd;
	this->sever_socket = listen_sd;
	this->server_blocks = serverBlocks;
	this->request.sever_socket = listen_sd;
}

Client &Client::operator=(Client const &client){
	if (this != &client)
	{
		this->_socket = client._socket;
	}
	return (*this);
}

Client::~Client(){}

bool Client::receiveData(const char *buffer, int rc){
	// (void) buffer;
	// (void) rc;
	// _data = createMockData();

	request.data.write(buffer, rc);
	if (httpStage()) //true = already response , false = not response yet --> request incomplete
		return (true);
	return (false);
}

bool Client::httpStage() {
	// Cgi cgi(request.method, request.path, request.header, request.body); --> set request data when init

	switch (_stage) {
		case FIRST_LINE: {
			_stage = request.parseFirstLine(_stage);
		}
		case HEADER: {
			_stage = request.parseHeader(_stage); //method not allowed, client_max_body_size
		}
		case BODY: {
			_stage = request.parseBody(_stage);
		}
		case ROUTER: {
			//print
			// _stage = cgi.router(_stage);
			// std::string errLocation = "./docs/curl/" + request.setDefaultErrorPage(); // test response
			std::string errLocation = request.setDefaultErrorPage(); // test response
			_response.error404(_socket, errLocation);
			// _response.byFile(_socket, 404, errLocation, "text/html; charset=UTF-8"); // test response
			// _response.byFile(_socket, 200, "./page-copy.html", "text/html; charset=UTF-8"); // test response
			// _response.byStatus(_socket, 307); // test response
			// std::stringstream mockStringstream = createMockData();// test response
			// _response.byStringstream(_socket, 200, mockStringstream,"text/html");// test response
			_stage = RESPONSED;
		}
		case RESPONSED: {
			request.clear();
			_stage = FIRST_LINE;
			return (true); // already response // if not comment now will infinite loop
		}
		default:
			break;
	}
	return (false); // not response yet --> request incomplete
}