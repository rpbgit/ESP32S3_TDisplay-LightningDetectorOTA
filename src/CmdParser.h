#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <Arduino.h>

// Define a type for command handler functions
typedef void (*CommandHandler)(char *param);

// Define a structure for command entries
struct CommandEntry {
    const char *commandName;
    CommandHandler handler;
};

// CommandParser class for parsing and executing commands
class CommandParser {
public:
    CommandParser(CommandEntry *commands, int numCommands);
    void processInput();
    void processInput(const char* cmd);
    void parseCommand(char* command);
    void showPrompt();
    long parseParameter(char *param);
    void toLowerCase(char *str);

private:
    static const int historySize = 16;      // Max number of commands in history
    static const int bufferSize = 128;      // Max length of a command
    char inputBuffer[bufferSize];           // Current input buffer
    char commandHistory[historySize][bufferSize]; // Command history buffer
    int historyCount;                       // Number of commands in history
    int historyIndex;                       // Current index for history navigation

    CommandEntry *commandTable;
    int numCommands;
    int bufferIndex;                        // Current position in inputBuffer
};
#endif