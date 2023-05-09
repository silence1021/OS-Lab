#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>

enum class UserClass
{
    owner,
    group,
    other
};

struct Permission
{
    bool read;
    bool write;
    bool execute;
};

// Define a structure for File
struct File
{
    std::string name;
    std::map<UserClass, Permission> permissions;
};

// Define a structure for Directory
struct Directory
{
    std::string name;
    std::map<std::string, Directory> subdirectories;
    std::map<std::string, File> files;
    std::map<UserClass, Permission> permissions;
};

// Declare global variables for the current directory and root directory
Directory current_directory;
Directory root;

// Function to print the current directory's content
void ls()
{
    for (const auto &subdir : current_directory.subdirectories)
    {
        std::cout << subdir.first << "/\n";
    }
    for (const auto &file : current_directory.files)
    {
        std::cout << file.first << "\n";
    }
}

// Function to print the working directory
void pwd(const Directory &dir, const std::string &path)
{
    if (dir.name == current_directory.name)
    {
        std::cout << path << "\n";
        return;
    }

    for (const auto &subdir : dir.subdirectories)
    {
        pwd(subdir.second, path + "/" + subdir.first);
    }
}

// Function to change directory
bool cd(const std::string &dir_name)
{
    if (dir_name == "..")
    {
        if (current_directory.name != root.name)
        {
            pwd(root, "/");
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (current_directory.subdirectories.count(dir_name))
    {
        current_directory = current_directory.subdirectories[dir_name];
        return true;
    }

    return false;
}

// Function to create a new directory
void mkdir(const std::string &dir_name)
{
    Permission default_permission = {true, true, true};
    current_directory.subdirectories[dir_name] = {dir_name, {}, {}, {{UserClass::owner, default_permission}, {UserClass::group, default_permission}, {UserClass::other, default_permission}}};
}

// Function to remove a directory
void rmdir(const std::string &dir_name)
{
    current_directory.subdirectories.erase(dir_name);
}

// Function to create a new file
void touch(const std::string &file_name)
{
    Permission default_permission = {true, true, false};
    current_directory.files[file_name] = {file_name, {{UserClass::owner, default_permission}, {UserClass::group, default_permission}, {UserClass::other, default_permission}}};
}

// Function to remove a file
void rm(const std::string &file_name)
{
    current_directory.files.erase(file_name);
}

int main()
{
    root = {"root"};
    current_directory = root;

    std::string command;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, command);

        std::istringstream iss(command);
        std::string operation;
        std::string arg;

        iss >> operation >> arg;

        if (operation == "ls")
        {
            ls();
        }
        else if (operation == "pwd")
        {
            pwd(root, "/");
        }
        else if (operation == "cd")
        {
            if (!cd(arg))
            {
                std::cout << "Error: Directory not found.\n";
            }
        }
        else if (operation == "mkdir")
        {
            mkdir(arg);
        }
        else if (operation == "rmdir")
        {
            rmdir(arg);
        }
        else if (operation == "touch")
        {
            touch(arg);
        }
        else if (operation == "rm")
        {
            rm(arg);
        }
        else if (operation == "exit")
        {
            break;
        }
        else
        {
            std::cout << "Error: Invalid command.\n";
        }
    }
    return 0;
}