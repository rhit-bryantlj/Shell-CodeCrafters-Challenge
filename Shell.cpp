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
  if(directory[0] == '.'){ // relative path or home directory path search
    std::vector<std::string> split_dir = split_string(directory, '/');
    int dir_it = 0;
    if(split_dir[0] == ".") // should handle for starting at current ex ./dir/dir2
      dir_it == 1;
    std::filesystem::path cur = std::filesystem::current_path();
    while(dir_it < split_dir.size()){
      if(split_dir[dir_it] == ".."){
        std::filesystem::current_path(cur.parent_path());
        cur = std::filesystem::current_path();
      } else {
        cur.append(split_dir[dir_it]);
        if(!std::filesystem::exists(cur)){
          std::cout << "cd: " << directory << ": No such file or directory\n";
          break;
        }
        std::filesystem::current_path(cur);
      }
      dir_it++;
    }
  } else if(directory[0] == '~'){ // HOME directory
    std::string home = getenv("HOME");
    std::filesystem::current_path(home);
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
      for(int i = 1; i < arguments.size(); i++){
        if(i == arguments.size() -1)
          std::cout << arguments[i] << "\n";
        else
          std::cout << arguments[i] << " ";
      }
    }else if(arguments[0] == "type"){
      handle_type_command(arguments, path);
    } else if(arguments[0] == "pwd"){
      std::string cwd = std::filesystem::current_path();
      std::string print_cwd = cwd.substr(0, cwd.length());
      std::cout << print_cwd << "\n";
    } else if(arguments[0] == "cd") {
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
