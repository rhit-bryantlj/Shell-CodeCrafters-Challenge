#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdbool.h>
#include <unistd.h>

std::vector<std::string> split_string(const std::string &s, char delimiter){
    std::stringstream ss(s);
    std::vector<std::string> return_vect;
    std::string token;
    while(getline(ss, token, delimiter)){
      return_vect.push_back(token);
    }
    return return_vect;
}

void handle_type_command(std::vector<std::string> arguments, std::vector<std::string> path){
  if(arguments[1] == "echo" || arguments[1] == "exit" || arguments[1] == "type" || arguments[1] == "pwd" || arguments[1] == "cd"){
        std::cout << arguments[1] << " is a shell builtin\n";
  }else{
    std::string filepath;
    for(int i = 0; i < path.size(); i++){
      filepath = path[i] + '/' + arguments[1];
      std::ifstream file(filepath);
      if(file.good()){
        std::cout << arguments[1] << " is " << filepath << "\n";
        return;
      }
    }
    std::cout << arguments[1] << ": not found\n";
  }
}

void handle_change_directory(std::string directory){ 
  if(directory[0] == '~'){ // HOME directory
    std::filesystem::current_path(getenv("HOME"));
  } else { // Absolue Filepath
    std::filesystem::path path(directory);
    if(std::filesystem::is_directory(path))
      std::filesystem::current_path(directory);
    else
      std::cout << "cd: " << directory << ": No such file or directory\n";
  }
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string path_string = getenv("PATH");
  std::vector<std::string> path = split_string(path_string, ':');

  std::string input;
  std::vector<std::string> arguments;

  while(true){
    std::cout << "$ ";
    std::getline(std::cin, input);
    if(input == "exit 0" || input == "exit"){
      return 0;
    }

    arguments = split_string(input, ' ');

    if(arguments[0] == "echo"){
      std::cout << input.substr(input.find(" ") + 1) << "\n";
    }else if(arguments[0] == "type"){
      handle_type_command(arguments, path);
    } else if(arguments[0] == "pwd"){
      std::string cwd = std::filesystem::current_path();
      std::string print_cwd = cwd.substr(0, cwd.length());
      std::cout << print_cwd << "\n";
    } else if(arguments[0] == "cd") {
      if(arguments.size() > 2)
        std::cout << "cd: too many arguments\n";
      else
        handle_change_directory(arguments[1]);
    } else {
      std::string filepath;
      for(int i = 0; i < path.size(); i++){
        filepath = path[i] + '/' + arguments[0];
        std::ifstream file(filepath);
        if(file.good()){
          std::string command = "exec " + path[i] + '/' + input;
          std::system(command.c_str());
          break;
        }
        else if(i == path.size() - 1){
          std::cout << arguments[0] << ": not found\n";
        }
      }
    }
  }
}
