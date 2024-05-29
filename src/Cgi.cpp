#include "Cgi.hpp"
#include "utils.hpp"

Cgi::Cgi():_errnum(0) {

}

Cgi::Cgi(Cgi const &inst) {

    *this = inst;
}

Cgi &Cgi::operator=(Cgi const &inst) {

    if (this != &inst) {
        this->_errnum = inst._errnum;
        this->_req = inst._req;
        this->_resp = inst._resp;
    }
    return *this;
}

Cgi::~Cgi() {

}

int     Cgi::getErrnum() {

    return this->_errnum;
}

void     Cgi::setErrnum(int val) {

    this->_errnum = val;
}

void     Cgi::setSocket(int val) {

    this->_socket = val;
}

void    Cgi::setContentTypes() {

    this->_contentTypes["html"] = "text/html; charset=UTF-8";
    this->_contentTypes["htm"] = "text/html; charset=UTF-8";
    this->_contentTypes["css"] = "text/css";
    this->_contentTypes["jpg"] = "image/jpeg";
    this->_contentTypes["jpeg"] = "image/jpeg";
    this->_contentTypes["png"] = "image/png";
    this->_contentTypes["webp"] = "image/webp";
    this->_contentTypes["js"] = "application/x-javascript";
}

std::string Cgi::getType(std::string key) {

    if (this->_contentTypes.count(key) > 0) {
        return this->_contentTypes[key];
    } else {
        return "application/octet-stream"; // Default content type
    }
}

void    Cgi::initCgi(int errnum, int socket, std::vector<ServerBlock>* instptr, Request &reqinst) {
 
    this->_errnum = errnum;
    this->_socket = socket;
    this->server_blocks = instptr;
    this->_req = reqinst;
    setContentTypes();
}

HttpStage Cgi::apiRouter() {
    LocationBlock location = searchLocation(_req.header["Host"], _req.path, *server_blocks);
    ServerBlock server = searchServer(_req.header["Host"], *server_blocks);

    std::cout << RED << "cgi errnum: " << _errnum << DEFAULT << std::endl;
    if (server.getServerName().length() == 0) {
        _resp.byStatus(_socket, 400);
        return RESPONSED;
    }
    if (_errnum != 0 && _errnum != 404) {
        _resp.byStatus(_socket, _errnum);
        return RESPONSED;
    }
    if (location.getDirectoryPath().length() != 0) {
        bool method_allow = location.getAllowMethods()[_req.method];
        if (method_allow == false) {
            _resp.byStatus(_socket, 405);
        }
        else if (location.getDirectoryPath() == "/cgi") {
            //Generate page with cgi
            if (generatePage(location, server) == false)
                _resp.errorDefault(_socket, _req.setDefaultErrorPage(404), 404);
        }
        else if (location.getDirectoryPath() == "/upload") {
            //Upload file
            if (upload(location, server) == false)
                _resp.errorDefault(_socket, _req.setDefaultErrorPage(404), 404);
        }
        else if (location.getDirectoryPath() == "/delete") {
            //Delete file
            if (Delete(location, server) == false)
                _resp.errorDefault(_socket, _req.setDefaultErrorPage(404), 404);
        }
        else if (location.getReturn().length() != 0) {
            //Redirect
            std::cout << YELLOW << "Redirect to :" << location.getReturn() << DEFAULT << std::endl;
            _resp.byRedirect(_socket, 307, location.getReturn());
        }
        else
            if (serveFile(server, location) == false)
                _resp.errorDefault(_socket, _req.setDefaultErrorPage(404), 404);
        return RESPONSED;
    }
    //location wasn't found
    if (serveFile(server, location) == false)
        _resp.errorDefault(_socket, _req.setDefaultErrorPage(404), 404);
    return RESPONSED;
    // serveFile();
    // _resp.byFile(_socket, 200, "./page-copy.html", "text/html; charset=UTF-8"); // test response
}

long Cgi::creatFileStream() {
    FILE *file = tmpfile();
    if (file == NULL) {
        std::cerr << "Failed to create temporary file" << std::endl;
        return -1;
    }

    const std::size_t bufferSize = 4096;
    std::vector<char> buffer(bufferSize);

    _req.body.seekg(0, std::ios::beg); // Rewind the stringstream to the beginning

    while (_req.body) {
        _req.body.read(buffer.data(), bufferSize);
        std::streamsize bytesRead =  _req.body.gcount();

        if (bytesRead > 0) {
            // Write the read bytes to the file
            std::fwrite(buffer.data(), sizeof(char), bytesRead, file);
        }
    }

    std::rewind(file);
    long filetmp = fileno(file);
    return filetmp;
}

bool Cgi::upload(LocationBlock &location, ServerBlock &server) {
    std::string root = server.getRoot();
    std::string filepath = "./" + root + "/cgi";
    if (_req.path[_req.path.length() - 1] != '/')
        filepath += "/";
    if(isIndexExists(filepath, location.getIndex()) == false)
        return false;

    std::string extension = filepath.substr(filepath.find_last_of("."));
    std::string compiler = location.getCgiMap()[extension];

    pid_t p_id = fork();
    if(p_id == 0)
	{
		pid_t coc_id = fork();
		if (coc_id == 0) {
			dup2(creatFileStream(), STDIN_FILENO);
            _req.method = "POST";
			execute(filepath, compiler, root); 
			_exit(0);
		} else {
			int status;
			pid_t exited_pid = waitcoc(coc_id, &status);
			cgiResponse(exited_pid, status, "/upload", _req.body);
		}
		_exit(0);
    }
    return true;   
}

bool Cgi::Delete(LocationBlock &location, ServerBlock &server) {

    std::string root = server.getRoot();
    std::string filepath = "./" + root + "/cgi";
    if (_req.path[_req.path.length() - 1] != '/')
        filepath += "/";
    if(isIndexExists(filepath, location.getIndex()) == false)
        return false;

    // std::cerr << "Cgi File: " << filepath << std::endl;
    std::string extension = filepath.substr(filepath.find_last_of("."));
    std::string compiler = location.getCgiMap()[extension];
    // std::cerr << "Cgi compiler: " << compiler << std::endl;

    pid_t p_id = fork();
    if(p_id == 0)
	{
		pid_t coc_id = fork();
		if (coc_id == 0) {
			dup2(creatFileStream(), STDIN_FILENO);
            _req.method = "DELETE";
			execute(filepath, compiler, root); 
		} else {
			int status;
			pid_t exited_pid = waitcoc(coc_id, &status);
			cgiResponse(exited_pid, status, "/delete", _req.body);
		}
		exit(0);
    }
    return true;   
}

bool Cgi::generatePage(LocationBlock &location, ServerBlock &server) {
    
    std::string root = server.getRoot();
    std::string filepath = "./" + root + _req.path;
    if (_req.path[_req.path.length() - 1] != '/')
        filepath += "/";
    if(isIndexExists(filepath, location.getIndex()) == false)
        return false;

    // std::cout << "Cgi File: " << filepath << std::endl;
    std::string extension = filepath.substr(filepath.find_last_of("."));
    std::string compiler = location.getCgiMap()[extension];
    // std::cout << "Cgi compiler: " << compiler << std::endl;

    pid_t p_id = fork();
    if (p_id == 0) {

        int pipes[2];
        if (pipe(pipes) == -1) {
            std::cerr << RED << "Error: cannot open pipe." << DEFAULT << std::endl;
            exit(1);
        }
        pid_t coc_id = fork();
        if (coc_id == 0) {
            dup2(pipes[1], STDOUT_FILENO);
            close(pipes[0]);
            close(pipes[1]);
            execute(filepath, compiler, root); 
        }else {
			int                 exit_status;
			std::stringstream   stream;
			close(pipes[1]);
			pid_t exitPID = waitcoc(coc_id, &exit_status);
			char buffer[1024];
			while(true) {
				int rd = read(pipes[0], &buffer, 1023);
				if (rd == 0) break;
				if (rd < 0) {
                    _resp.byStatus(_socket, 502);
                }
                buffer[rd] = '\0';
				stream << buffer;
                // std::cerr << YELLOW << "Stream: " << buffer << std::endl;
			}
			close(pipes[0]);
			cgiResponse(exitPID, exit_status, "/generate", stream);
		}
		exit(0);
    }
    return true;
}

void Cgi::cgiResponse(pid_t exitPID, int status, std::string cgi_route, std::stringstream &stream) {

    if(exitPID < 0) {
		std::cerr << "Error: CGI not worked properly." << std::endl;
        _resp.byStatus(_socket, 500);
	}

	if (!WIFEXITED(status)) {
		std::cerr << "Error: CGI not worked properly." << std::endl;
		_resp.byStatus(_socket, 500);
	} 
	int exit_status = WEXITSTATUS(status);
	//execve failed;
	if (exit_status != 0) {
		std::cerr << "Error: CGI not worked properly." << std::endl;
		_resp.byStatus(_socket, 500);
	} 

    if (cgi_route == "/generate") {
        std::cout << YELLOW << "Generate Page..." << std::endl;
        _resp.byStringstream(_socket, 200, stream, "text/html; charset=UTF-8");
    } else if (cgi_route == "/upload") {
        std::cout << YELLOW << "Upload File..." << std::endl;
        _resp.byStatus(_socket, 201);
    } else if (cgi_route == "/delete") {
        std::cout << YELLOW << "Delete File..." << std::endl;
        _resp.byStatus(_socket, 200);
    }
}

time_t Cgi::startTime() {

    time_t timer;
    return time(&timer);
}

pid_t Cgi::waitcoc(pid_t child, int *status) {
    
    pid_t exit_status;
    time_t start_time = startTime();
    time_t post_time = start_time + 3;

    while ((exit_status = waitpid(child, status, WNOHANG)) == 0) {
    //   printf("Child process (pid: %d) hasn't exited yet...\n", child_pid);
      start_time = startTime();      
      if (post_time - start_time == 0)
        kill(child, SIGKILL);
      sleep(1);
    }
    return exit_status;
}

void Cgi::execute(std::string &file, std::string &compiler, std::string &root){

    char *env[] = {NULL};
	char *av[6];
    if (_req.method == "DELETE")
        root =  "./" + root + "/upload";
    else
        root = "./" + root + _req.path;
    // std::cerr << "method: " << _req.method << std::endl;
    std::cerr << "content-type: " << _req.header["Content-Type"] << std::endl;
    // std::cerr << "root: " << root.c_str() << std::endl;

	av[0] = strdup(compiler.c_str());
	av[1] = strdup(file.c_str());
	av[2] = strdup(_req.method.c_str());//Cgi upload need method
	av[3] = strdup(_req.header["Content-Type"].c_str());//Cgi upload need content-type
	av[4] = strdup(root.c_str());
	av[5] = NULL;
	if(execve(av[0], av, env) == -1)
		_exit(1);
    
}

bool    Cgi::serveFile(ServerBlock &server, LocationBlock &location){

    std::string root;
    std::string endpoint;
    std::vector<std::string> index;
    std::string filepath;

    if (prepareFilePath(server, location, root, endpoint, filepath) == false)
        return false;

    if (isFileExists(filepath) == true) {
        if (!(hasPermission(filepath, R_OK))){
            _resp.byStatus(_socket, 403);
            return (true);
        }
        if (isDir(filepath)) {
            // std::cout << "This filepath:" << filepath << " does exists." << std::endl;
            //Request directory need to get truePath for redirection.
            if (location.getIndex().size() != 0) {
                index = location.getIndex();
            }
            if (isIndexExists(filepath, index) == false) {
                if (useServerparameter(filepath, server, location) == true)
                    return true;
                else
                    return false;
            } else {
                 std::cout << YELLOW << "ServeFile" << DEFAULT << std::endl;

                _resp.byRedirect(_socket, 307, _truePath);
                return true;
            }
        }
        //Request a file.
        std::string contentTypes = checkContentType(filepath);
        _resp.byFile(_socket, 200, filepath, contentTypes);
        return true;
    }//end of if (isFileExists(filepath))
    return false;
}

bool Cgi::isFileExists(const std::string& filepath) {
    struct stat buffer;
    return (stat(filepath.c_str(), &buffer) == 0);
}

bool Cgi::isDir(const std::string& filepath) {
    struct stat buffer;
    if (stat(filepath.c_str(), &buffer) == 0) {
        return S_ISDIR(buffer.st_mode);
    }
    return false;
}

bool Cgi::hasPermission(const std::string& filepath, int mode) {
    return (access(filepath.c_str(), mode) == 0);
}

bool Cgi::isIndexExists(std::string &filepath, std::vector<std::string> index) {

    std::vector<std::string>::iterator it = index.begin();
    for (; it != index.end(); it++) {
        if (isFileExists(filepath + *it))
        {
            filepath += *it;
            if (_req.path == "/")
                _truePath = _req.path + *it;
            else
                _truePath = _req.path + "/" + *it;
            return true;
        }
    }
    return false;
}

std::string Cgi::checkContentType(std::string file) {

    std::string type = "unkown";
    size_t      found = file.find_last_of('.');
    std::string file_ext;
    
    if (found == std::string::npos)
        return type;
    
    file_ext = file.substr(found + 1);
    type = getType(file_ext);
    return type;
}

bool Cgi::useServerparameter(std::string &filepath, ServerBlock &server, LocationBlock &location){

    //in location path cannot found index, go back to use server_index.
    std::vector<std::string> index;
    
    index = server.getIndex();
    if (isIndexExists(filepath, index) == false) {
        //there is no server's index exists in root+location_path check if autoindex = on
    
        if (location.getAutoIndex() == true) {
            _resp.byAutoIndex(_socket, 200, filepath);
            return true;
        }
        else {
            //autoindex = off go to response error default
            return false;
        }
    }
    std::cout << "True path: " << _truePath << std::endl;
    _resp.byRedirect(_socket, 307, _truePath);
    return true;
}

bool Cgi::prepareFilePath(ServerBlock &server, LocationBlock &location, std::string &root, std::string &endpoint, std::string &filepath) {

    //need to know is it a full path with file_name or it just a directory.
    if (location.getRoot().length() != 0) {
        root = location.getRoot();
    } else {
        root = server.getRoot();
    }
    // if (root[root.size() - 1] != '/' && location.getDirectoryPath()[0] != '/') {
    //     root += "/";
    // }
    std::cout << "request path: " << _req.path << std::endl;
    //file or directory
    if (checkContentType(_req.path) == "unkown") {
        //directory
        // if (location.getDirectoryPath().length() != 0) {
        filepath = "./" + root + _req.path;
        if (isFileExists(filepath) == true) {
            if (isDir(filepath)) {
                filepath += "/";
            }
            if (!(hasPermission(filepath, R_OK))){
                _resp.byStatus(_socket, 403);
                return (true);
            }
        }
        else {
            // not exists directory return 404.
            return (false);
        }
        // filepath = "./" + root + endpoint + "/";
    }
    else {
        //full path with root.
        if (_req.path.find(root) != std::string::npos) {
            filepath = "." + _req.path;
        } else {
            // redirect exactly path (without root.)
            filepath = "./" + root + _req.path;
        }
    }
    std::cout << "Prepare: " << filepath << std::endl;
    return true;
}

