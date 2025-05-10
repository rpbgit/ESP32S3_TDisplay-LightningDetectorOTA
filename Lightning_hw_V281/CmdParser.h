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

class CommandParser {
public:
    CommandParser(CommandEntry *commands, int numCommands);
    void processInput();
    void processInput(const char* cmd);
    void showPrompt();
    long parseParameter(char *param);

private:
    CommandEntry *commandTable;
//    CommandType *commandType;
    int numCommands;
    static const int bufferSize = 50;  // Adjust the buffer size as needed
    char inputBuffer[bufferSize];
    int bufferIndex;
    
    void parseCommand(char *command);
    //CommandType getCommandType(const char *commandName);
    void toLowerCase(char *str);
};

#endif
