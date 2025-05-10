#include "CmdParser.h"


CommandParser::CommandParser(CommandEntry *commands, int numCommands) 
    : commandTable(commands), numCommands(numCommands), bufferIndex(0) {
        memset(inputBuffer, 0, sizeof(inputBuffer));
}

void CommandParser::processInput() {
    //check and see if the Serial port is connected initialized before looking for something
    if(Serial) {  
        if (Serial.available() > 0) {
            char incomingChar = Serial.read();
            // Handle backspace
            if (incomingChar == 8 || incomingChar == 127) { // ASCII 8: Backspace, ASCII 127: DEL
                if (bufferIndex > 0) {
                    bufferIndex--;
                    Serial.print("\b \b");  // Move cursor back, print space, and move cursor back again
                }
            } else if (incomingChar == '\n' || incomingChar == '\r') {
            // Check for end of command
                inputBuffer[bufferIndex] = '\0'; // Null-terminate the string
                //Serial.println();  // Move to the next line on the terminal   
                if (bufferIndex > 0) {
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
    //*cmd = (char)"\0"; // make it an empty string when done. 

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
