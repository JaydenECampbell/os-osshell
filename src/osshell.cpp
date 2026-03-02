#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>

bool fileExecutableExists(std::string file_path);
void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    // Create list to store history
    std::vector<std::string> history = *readHistory();

    // Create variables for storing command user types
    std::string user_command;               // to store command user types in
    std::vector<std::string> command_list;  // to store `user_command` split into its variour parameters
    char **command_list_exec;               // to store `command_list` converted to an array of character arrays

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    /*
    Provide a prompt for a user to enter a command (osshell> )

    Search the directories in the PATH environment variable and find the first executable with the name of the entered command
        If one is found, spawn a new process to run that executable, and wait for its completion (i.e. fork() and exec())
        If not found, print the following error message:
        "<command>: Error command not found"
    Just hitting enter without typing a command should not print an error
    After a command executes (or no command is found and error printed, or user simply hit enter), the shell interface should prompt the user for the next command

    */
    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    while (true)
    {
        std::cout << "osshell> ";
        std::getline(std::cin, user_command);
        if (user_command == "")
        {
            continue;
        }

        history.push_back(user_command);  // Store command in history

        if (user_command == "exit")
        {
            break;
        }
        
        else if (user_command == "history")
        {
            int i;
            for (i = 0; i < history.size(); i++)
            {
                printf("%2d: %s\n", i, history[i].c_str());
            }
            printf("------\n");
        }

        else if (user_command != "")
        {
            splitString(user_command, ' ', command_list);
            if (command_list.size() == 0)
            {
                continue;
            }

            std::string executable_path = "";   // Searches path for executable with name of command user entered, and stores full path to that executable here if found
            std::string command_name = command_list[0];

            if (command_name[0] == '.' || command_name[0] == '/')
            {
                if (fileExecutableExists(command_name))
                {
                    executable_path = command_name;
                }
            }

            else
            {
                for (int i = 0; i < os_path_list.size(); i++)
                {
                    std::string full_path = os_path_list[i] + "/" + command_name;

                    if (fileExecutableExists(full_path))
                    {
                        executable_path = full_path;
                        break;
                    }
                }
            }

            // If not found → error
            if (executable_path == "")
            {
                std::cout << command_name << "<command>: Error command not found" << std::endl;
                continue;
            }

            command_list[0] = executable_path;  // Replaces command name with full path to executable for execv() function
            vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);   // Converts to char** for execv() function
            pid_t pid = fork(); // Forks process
            if (pid == 0)
            {
                execv(executable_path.c_str(), command_list_exec);  // Executes command in child process
                perror("execv failed"); // If execv returns, there was an error
                exit(1);
            }

            else if (pid > 0)
            {
                int status;
                waitpid(pid, &status, 0); // Waits for child process to finish
            }
        }
    }
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)

    

    /************************************************************************************
     *   Example code - remove in actual program                                        *
     ************************************************************************************/
     
    /*
    // Shows how to loop over the directories in the PATH environment variable
    int i;
    for (i = 0; i < os_path_list.size(); i++)
    {
        printf("PATH[%2d]: %s\n", i, os_path_list[i].c_str());
    }
    printf("------\n");
    
    // Shows how to split a command and prepare for the execv() function
    std::string example_command = "ls -lh";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");

    // Second example command - reuse the `command_list` and `command_list_exec` variables
    example_command = "echo \"Hello world\" I am alive!";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");

    
    /************************************************************************************
     *   End example code                                                               *
     ************************************************************************************/


    return 0;
}

/*
    history: pointer to history vector from main
*/
void printHistory(std::vector<std::string> *history) {
    for (int i = 0; i < history->size(); i++) {
        printf("  %d: %s\n", i, history->at(i).c_str());
    }
}

/*
    history: pointer to history vector from main
    cmd: the user command to add to history
*/
void pushToHistory(std::vector<std::string> *history, std::string cmd) {
    history->push_back(cmd);
    if (history->size() > 128) history->erase(history->begin());
    /* TODO: Write to file */
}

std::vector<std::string> *readHistory() {
    /* TODO: Read from file */
    return new std::vector<std::string>();
}

/*
   file_path: path to a file
   RETURN: true/false - whether or not that file exists and is executable
*/
bool fileExecutableExists(std::string file_path)
{
    bool exists = false;
    if (access(file_path.c_str(), F_OK) == 0 && access(file_path.c_str(), X_OK) == 0)
    {
        exists = true;
    }

    return exists;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}


