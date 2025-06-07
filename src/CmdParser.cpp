#include "CmdParser.h"

CommandParser::CommandParser(CommandEntry *commands, int numCommands) 
    : commandTable(commands), numCommands(numCommands), bufferIndex(0), historyCount(0), historyIndex(-1) {
        memset(inputBuffer, 0, sizeof(inputBuffer));
        memset(commandHistory, 0, sizeof(commandHistory));
}
 
void CommandParser::processInput() {
    if(Serial) {  
        while (Serial.available() > 0) {
            char incomingChar = Serial.read();

            // Remove ANSI escape sequence handling

            // Handle Windows up/down arrow keys (0x1E = up, 0x1F = down)
            if (incomingChar == 0x1E) { // Up arrow
                if (historyCount > 0) {
                    // Erase current input from terminal
                    while (bufferIndex > 0) {
                        Serial.print("\b \b");
                        bufferIndex--;
                    }
                    if (historyIndex == -1) historyIndex = historyCount - 1;
                    else if (historyIndex > 0) historyIndex--;
                    strncpy(inputBuffer, commandHistory[historyIndex], bufferSize - 1);
                    inputBuffer[bufferSize - 1] = '\0';
                    bufferIndex = strlen(inputBuffer);
                    Serial.print(inputBuffer);
                }
                continue;
            }
            if (incomingChar == 0x1F) { // Down arrow
                if (historyCount > 0 && historyIndex != -1) {
                    // Erase current input from terminal
                    while (bufferIndex > 0) {
                        Serial.print("\b \b");
                        bufferIndex--;
                    }
                    if (historyIndex < historyCount - 1) {
                        historyIndex++;
                        strncpy(inputBuffer, commandHistory[historyIndex], bufferSize - 1);
                        inputBuffer[bufferSize - 1] = '\0';
                    } else {
                        historyIndex = -1;
                        inputBuffer[0] = '\0';
                    }
                    bufferIndex = strlen(inputBuffer);
                    Serial.print(inputBuffer);
                }
                continue;
            }

            // Handle backspace
            if (incomingChar == 8 || incomingChar == 127) { // ASCII 8: Backspace, ASCII 127: DEL
                if (bufferIndex > 0) {
                    bufferIndex--;
                    Serial.print("\b \b");  // Move cursor back, print space, and move cursor back again
                }
            } else if (incomingChar == '\n' || incomingChar == '\r') {
                inputBuffer[bufferIndex] = '\0'; // Null-terminate the string
                if (bufferIndex > 0) {
                    // Add to history if not duplicate
                    if (historyCount == 0 || strcmp(inputBuffer, commandHistory[(historyCount - 1) % historySize]) != 0) {
                        strncpy(commandHistory[historyCount % historySize], inputBuffer, bufferSize - 1);
                        commandHistory[historyCount % historySize][bufferSize - 1] = '\0';
                        if (historyCount < historySize) historyCount++;
                    }
                    historyIndex = -1; // Reset navigation
                    parseCommand(inputBuffer);
                }
                bufferIndex = 0; // Reset buffer index for the next command
                showPrompt(); // Show the prompt again after processing
            } else {
                // Add character to buffer if space permits
                if (bufferIndex < bufferSize - 1) {
                    inputBuffer[bufferIndex++] = incomingChar;
                    Serial.print(incomingChar); // provide local echo
                }
            }
        }
    }
}

void CommandParser::processInput(const char* cmd)
{
    strncpy(inputBuffer,cmd, sizeof(inputBuffer)-1);
    parseCommand(inputBuffer);  // process a null terminated string as a command
}

void CommandParser::parseCommand(char* command)
{
    // Convert the command to lowercase for case-insensitive comparison
    toLowerCase(command);

    char* commandName = strtok(command, ":");
    char* commandParam = strtok(NULL, ":"); // If no parameter, this will be NULL

    for (int i = 0; i < numCommands; i++) {
        if (strcmp(commandTable[i].commandName, command) == 0) {
            commandTable[i].handler(commandParam); // Call the handler with the parameter (if any)
            return;
        }
    }
    Serial.printf(" <--Unknown command\n"); // if we fall thru, its unknown
}

void CommandParser::toLowerCase(char *str) {
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}

long CommandParser::parseParameter(char *param) {
    if (param == NULL)
        return -1;

    if (strlen(param) > 2 && param[0] == '0' && param[1] == 'x') {
        return strtol(param, NULL, 16); // Convert hex string to long
    } else {
        char* endptr;
        long num;
        num = strtol(param, &endptr, 10); // Convert decimal string to long
        return *endptr != '\0' ? -1 : num;
    }
}

void CommandParser::showPrompt() {
    Serial.print("\nCMD> ");
}
